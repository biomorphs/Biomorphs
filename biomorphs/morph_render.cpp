#include "biomorphs/morph_render.h"
#include "framework/graphics/device.h"

MorphRender::MorphRender()
{
}

MorphRender::~MorphRender()
{
}

Texture2D MorphRender::CopyOutputTexture(Texture2D& texture)
{
	Texture2D resultTexture = texture;;
	if( !resultTexture.IsValid() )
	{
		// create a texture the same size as the output texture
		Texture2D::Parameters tp;
		tp.access = Texture2D::CpuNoAccess;
		tp.bindFlags = Texture2D::BindAsShaderResource | Texture2D::BindAsRenderTarget;
		tp.format = Texture2D::TypeFloat32;
		tp.height = m_params.mTextureHeight;
		tp.width = m_params.mTextureWidth;
		tp.msaaCount = 1;
		tp.msaaQuality = 0;
		tp.numMips = 1;
		resultTexture = m_device->CreateTexture( tp );
	}

	if( resultTexture.IsValid() )
	{
		// render to the texture
		m_device->CopyTextureToTexture( m_texture, resultTexture );
	}

	return resultTexture;
}

int MorphRender::_drawRecursive( MorphDNA& dna, RecursionParams& params, MorphVertex* vertices, unsigned int* indices )
{
	if( params.branchDepth <= 0 )
	{
		return 0;
	}

	const float kBranchWidth = 0.09f;	// line width
	const D3DXVECTOR2 baseDirection(0.0f, 1.0f);

	// rotate the direction, based on the params angle
	D3DXMATRIX branchRotationMat;
	D3DXMatrixRotationZ( &branchRotationMat, params.Angle );

	// rotate the vector
	D3DXVECTOR4 branchDirection4;
	D3DXVec2Transform( &branchDirection4, &baseDirection, &branchRotationMat );
	
	// resolve to normalised 2d vector
	D3DXVECTOR2 branchDirection2( branchDirection4.x, branchDirection4.y );
	D3DXVec2Normalize( &branchDirection2, &branchDirection2 );

	// now get the normalised perpendicular
	D3DXVECTOR3 branchPerp3;
	const D3DXVECTOR3 v0( 0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 v1( branchDirection2.x, branchDirection2.y, 0.0f );
	D3DXVec3Cross( &branchPerp3, &v0, &v1 );
	D3DXVECTOR2 branchPerpendicular( branchPerp3.x, branchPerp3.y );
	D3DXVec2Normalize( &branchPerpendicular, &branchPerpendicular );
	branchPerpendicular = branchPerpendicular * 0.5f * kBranchWidth;

	// apply length to direction
	branchDirection2 = branchDirection2 * params.Length;

	// draw a single quad representing this branch
	D3DXVECTOR4 branchColour = params.Colour;

	vertices[0].mPosition = params.Origin - branchPerpendicular;
	vertices[0].mColour = branchColour;

	vertices[1].mPosition = params.Origin + branchPerpendicular;
	vertices[1].mColour = branchColour;

	vertices[2].mPosition = params.Origin + branchDirection2 + branchPerpendicular;
	vertices[2].mColour = branchColour;

	vertices[3].mPosition = params.Origin + branchDirection2 - branchPerpendicular;
	vertices[3].mColour = branchColour;
	vertices += 4;

	// add some indices for the 2 triangles
	indices[0] = params.vertexOffset + 0;	
	indices[1] = params.vertexOffset + 2;		
	indices[2] = params.vertexOffset + 1;

	indices[3] = params.vertexOffset + 0;	
	indices[4] = params.vertexOffset + 3;		
	indices[5] = params.vertexOffset + 2;
	indices += 6;

	int indexCount = 6;

	// add 2 children if required
	if( params.branchDepth > 0 )
	{
		int newDepth = params.branchDepth-1;
		float branchLength = BRANCHLENGTH(dna, newDepth);
		float branchAngle = BRANCHANGLE(dna, newDepth);
		D3DXVECTOR4 branchColour = BRANCHCOLOUR(dna, newDepth);
		
		// now draw 2 child branches
		RecursionParams childParams;
		childParams.branchDepth = params.branchDepth - 1;
		childParams.Length = branchLength;
		childParams.Origin = params.Origin + branchDirection2;
		childParams.Colour = branchColour;

		childParams.Angle = params.Angle + branchAngle;
		childParams.vertexOffset = params.vertexOffset + 4;
		int ch0Indices = _drawRecursive( dna, childParams, vertices, indices );

		// move vb + ib pointers along
		indices += ch0Indices;
		vertices += ((ch0Indices / 6) * 4);
		childParams.vertexOffset += ((ch0Indices / 6) * 4);

		childParams.Angle = params.Angle - branchAngle;

		int ch1Indices = _drawRecursive( dna, childParams, vertices, indices );
		indices += ch1Indices;
		vertices += ((ch1Indices / 6) * 4);

		indexCount += ch0Indices + ch1Indices;
	}

	return indexCount;
}

void MorphRender::DrawBiomorph( D3DXVECTOR2 origin, MorphDNA& dna )
{
	if( m_verticesWritten > kMaxVertices || m_indicesWritten > kMaxIndices )
	{
		printf("Drawing too many verts/indices\n");
		return ;
	}

	RecursionParams baseParams;
	baseParams.vertexOffset = m_verticesWritten;
	baseParams.branchDepth = BASEDEPTH(dna);
	baseParams.Angle = 0;						// Start straight up
	baseParams.Length = BASELENGTH(dna);
	baseParams.Origin = origin;
	baseParams.Colour = BASECOLOUR(dna);

	int indexCount = _drawRecursive( dna, baseParams, m_lockedVBData + m_verticesWritten, m_lockedIBData + m_indicesWritten );
	m_verticesWritten += (indexCount / 6) * 4;
	m_indicesWritten += indexCount;
}

void MorphRender::StartRendering()
{
	// Lock the VB and IB for writing
	m_lockedVBData = (MorphVertex*)m_device->LockVB(m_vb);
	m_lockedIBData = (unsigned int*)m_device->LockIB(m_ib);

	m_verticesWritten = m_indicesWritten = 0;
}

void MorphRender::EndRendering(D3DXVECTOR4 posScale)
{
	// Unlock the vb and ib ready to draw
	m_device->UnlockVB(m_vb);
	m_device->UnlockIB(m_ib);

	// reset the shader state (unbinds the render target)
	m_device->ResetShaderState();
	m_device->SetRenderTargets( &m_rt, &m_depthStencil );

	// Set the viewport
	Viewport vp;
	vp.topLeft = Vector2(0,0);
	vp.depthRange = Vector2f(0.0f,1.0f);
	vp.dimensions = Vector2(m_params.mTextureWidth, m_params.mTextureHeight);
	m_device->SetViewport( vp );

	// clear all colour to 0
	static float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_device->ClearTarget( m_rt, clearColour );

	// Clear depth/stencil
	m_device->ClearTarget(m_depthStencil, 1.0f, 0 );

	EffectTechnique t = m_shader.GetTechniqueByName("Render");

	m_device->SetTechnique(t, 0);
	m_device->SetInputLayout(m_inputLayout);
	m_device->SetPrimitiveTopology(PRIMITIVE_TRIANGLES);

	m_device->SetIndexBuffer(m_ib);
	m_device->SetVertexBuffer(0, m_vb);

	VectorConstant posScaleConstant = t.GetVectorConstant("PositionScale");
	posScaleConstant.Set(posScale);
	posScaleConstant.Apply();

	DrawIndexedParameters dp;
	dp.m_indexCount = m_indicesWritten;
	dp.m_pass = 0;
	dp.m_startIndex = 0;
	m_device->DrawIndexed(dp);
}

bool MorphRender::Initialise( Device* d, const Parameters& p )
{
	m_device = d;

	// load the effect
	Effect::Parameters ep("shaders/simple_blit.fx");
	m_shader = m_device->CreateEffect( ep );

	// create vertex descriptor
	VertexElement e;
	e.byteOffset=0;
	e.elementType = VertexElement::PerVertex;
	e.format = VertexElement::VTX_FLOAT2;
	e.SetSemanticName("POSITION");
	m_vd.AddElement(e);
	e.byteOffset += sizeof(D3DXVECTOR2);
	e.format = VertexElement::VTX_FLOAT4;
	e.SetSemanticName("COLOR");
	m_vd.AddElement(e);

	// allocate the vertex buffer memory
	size_t vertexSize = m_vd.GetVertexSize(0);
	size_t numVerts = kMaxVertices;
	size_t vertexBufferSize = vertexSize * numVerts;

	VertexBuffer::Parameters vbParams;
	vbParams.sourceBuffer = NULL;
	vbParams.access = VertexBuffer::CpuWrite;	// we want to write to the buffer
	vbParams.stride = vertexSize;
	vbParams.vertexCount = numVerts;
	vbParams.vertexSize = vertexSize;
	m_vb = m_device->CreateVB( vbParams );

	// allocate index buffer
	IndexBuffer::Parameters ibParams;
	ibParams.format = IndexBuffer::IB_32BIT;
	ibParams.indexCount = kMaxIndices;
	ibParams.sourceBuffer = NULL;
	ibParams.access = IndexBuffer::CpuWrite;	// we want to write to the buffer
	m_ib = m_device->CreateIB( ibParams );

	// Create the input layout
	m_inputLayout = m_device->CreateVertexInputLayout( m_shader, m_vd );

	// create a texture to render to
	Texture2D::Parameters tp;
	tp.access = Texture2D::CpuNoAccess;
	tp.bindFlags = Texture2D::BindAsShaderResource | Texture2D::BindAsRenderTarget;
	tp.format = Texture2D::TypeFloat32;
	tp.height = p.mTextureHeight;
	tp.width = p.mTextureWidth;
	tp.msaaCount = 1;
	tp.msaaQuality = 0;
	tp.numMips = 1;
	m_texture = m_device->CreateTexture( tp );

	// create the render target
	Rendertarget::Parameters rtp;
	rtp.target = m_texture;
	m_rt = m_device->CreateRendertarget( rtp );

	// create a depth-stencil buffer
	DepthStencilBuffer::Parameters dbp;
	dbp.m_format = DepthStencilBuffer::TypeDepthStencil32;
	dbp.m_width = p.mTextureWidth;
	dbp.m_height = p.mTextureHeight;
	dbp.m_msaaCount = 1;
	dbp.m_msaaQuality = 0;
	m_depthStencil = m_device->CreateDepthStencil( dbp );

	m_params = p;

	return m_shader.IsValid() && m_vb.IsValid() && m_ib.IsValid() && m_rt.IsValid() && m_depthStencil.IsValid();
}

bool MorphRender::Release()
{
	m_device->Release( m_rt );
	m_device->Release( m_texture );
	m_device->Release( m_inputLayout );
	m_device->Release( m_vb );
	m_device->Release( m_ib );
	m_device->Release( m_shader );

	return true;
}
