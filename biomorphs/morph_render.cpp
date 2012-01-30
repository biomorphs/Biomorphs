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

int MorphRender::_drawRecursive( MorphDNA& dna, RecursionParams& params, MorphVertex*& vertices, unsigned int*& indices )
{
	if( params.branchDepth <= 0 )
	{
		return 0;
	}

	const float kBranchWidth = 0.15f;	// line width
	D3DXVECTOR2 branchDir;
	D3DXVECTOR2 branchPerp;
	GetGeometryVectors( params.Angle, params.Length, branchDir, branchPerp );	// calculate vectors
	int vCount = WriteVertices( vertices, kBranchWidth, params.Colour, params.Origin, branchDir, branchPerp );	// write verts
	int iCount = WriteQuadIndices( indices, params.vertexOffset );	// write indices

	// calculate child parameters
	int newDepth = params.branchDepth-1;
	float branchLength = BRANCHLENGTH(dna, newDepth);
	float branchAngle = BRANCHANGLE(dna, newDepth);
	D3DXVECTOR4 branchColour = BRANCHCOLOUR(dna, newDepth);
		
	// now draw 2 child branches
	RecursionParams childParams;
	childParams.branchDepth = newDepth;
	childParams.Length = branchLength;
	childParams.Origin = params.Origin + branchDir;
	childParams.Colour = branchColour;
	childParams.Angle = params.Angle + branchAngle;
	childParams.vertexOffset = params.vertexOffset + vCount;

	int ch0Indices = _drawRecursive( dna, childParams, vertices, indices );
	childParams.vertexOffset += ((ch0Indices / 6) * 4);

	childParams.Angle = params.Angle - branchAngle;
	int ch1Indices = _drawRecursive( dna, childParams, vertices, indices );

	iCount += ch0Indices + ch1Indices;

	return iCount;
}

void MorphRender::DrawBiomorph( MorphDNA& dna )
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
	baseParams.Origin = D3DXVECTOR2(0.0f,0.0f);
	baseParams.Colour = BASECOLOUR(dna);

	MorphVertex* v = m_lockedVBData + m_verticesWritten;
	unsigned int* i = m_lockedIBData + m_indicesWritten;
	int indexCount = _drawRecursive( dna, baseParams, v, i );
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
