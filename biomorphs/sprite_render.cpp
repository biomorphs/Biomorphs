#include "sprite_render.h"
#include "spritemap.h"
#include "framework\graphics\shadowed_device.h"

struct SpriteVertex
{
	D3DXVECTOR2 mPosition;
	D3DXVECTOR2 mUV;	
};

void SpriteRender::AddSprite( int spriteID, D3DXVECTOR2 position )
{
	Sprite sp;
	sp.position = position;
	sp.spriteID = spriteID;
	mSprites.push_back(sp);
	mDirty = true;
}

void SpriteRender::Draw( ShadowedDevice& device, D3DXVECTOR2 startPosition, D3DXVECTOR2 scale )
{
	if( mDirty )
	{
		updateSpriteMesh( device );
		mDirty = false;
	}

	EffectTechnique t = m_shader.GetTechniqueByName("Render");

	VectorConstant posScale = t.GetVectorConstant("PositionScale");
	posScale.Set(D3DXVECTOR4(startPosition.x, startPosition.y, scale.x, scale.y));
	posScale.Apply();

	TextureSampler sampler = t.GetSamplerByName("BlitTexture");
	sampler.Set( m_spritemap->GetTexture() );

	device.SetTechnique(t, 0);
	device.SetInputLayout(m_inputLayout);
	device.SetPrimitiveTopology(PRIMITIVE_TRIANGLES);

	device.SetIndexBuffer(m_spriteIb);
	device.SetVertexBuffer(0, m_spriteVb);

	DrawIndexedParameters dp;
	dp.m_indexCount = mSprites.size() * 6;
	dp.m_pass = 0;
	dp.m_startIndex = 0;
	device.DrawIndexed(dp);
}

void SpriteRender::Release( Device& d )
{
	mSprites.destroy();
	d.Release( m_spriteVb );
	d.Release( m_spriteIb );
	d.Release( m_inputLayout );
}

bool SpriteRender::Create( Device& d, Parameters& p )
{
	if( !mSprites.init(p.mMaxSprites) )
	{
		return false;
	}

	m_shader = p.shader;
	m_spritemap = p.spritemap;

	return initGraphics(d);
}

void SpriteRender::updateSpriteMesh( ShadowedDevice& d )
{
	Device dd = d.GetDevice();

	// lock the vertex buffer
	SpriteVertex* vertices = (SpriteVertex*)dd.LockVB(m_spriteVb);

	const float kXScale = 1.0f;
	const float kYScale = 1.0f;
	int tilesDrawn=0;
	for(unsigned int a=0;a<mSprites.size();++a)
	{
		int sid = mSprites[a]->spriteID;
		D3DXVECTOR2 uv0; 
		D3DXVECTOR2 uv1;
		if( m_spritemap->GetSprite(sid,uv0,uv1) )
		{
			SpriteVertex v;
			// push a quad for each tile
			D3DXVECTOR2 pos = mSprites[a]->position;

			v.mPosition = pos + D3DXVECTOR2(0.0f, 0.0f);		v.mUV = D3DXVECTOR2(uv0.x,uv1.y);
			*vertices = v;	++vertices;

			v.mPosition = pos + D3DXVECTOR2(kXScale, 0.0f);		v.mUV = uv1;
			*vertices = v;	++vertices;
			
			v.mPosition = pos + D3DXVECTOR2(kXScale, kYScale);	v.mUV = D3DXVECTOR2(uv1.x,uv0.y);
			*vertices = v;	++vertices;			

			v.mPosition = pos + D3DXVECTOR2(0.0f, kYScale);		v.mUV = uv0;	
			*vertices = v;	++vertices;
			++tilesDrawn;
		}
	}

	dd.UnlockVB(m_spriteVb);

	unsigned short *indices = (unsigned short*)dd.LockIB(m_spriteIb);
	for(int tile=0;tile<tilesDrawn;++tile)
	{
		int i=tile*4;
		unsigned short *ind = indices;	
		
		*(ind + 0) = i + 0;	
		*(ind + 1) = i + 2;	
		*(ind + 2) = i + 1;	
		*(ind + 3) = i + 0;	
		*(ind + 4) = i + 3;	
		*(ind + 5) = i + 2;	

		indices = indices + 6;
	}

	dd.UnlockIB(m_spriteIb);
}

bool SpriteRender::initGraphics(Device& d)
{
	VertexElement e;
	e.byteOffset=0;
	e.elementType = VertexElement::PerVertex;
	e.format = VertexElement::VTX_FLOAT2;
	e.SetSemanticName("POSITION");
	m_vd.AddElement(e);
	e.byteOffset += sizeof(D3DXVECTOR2);
	e.SetSemanticName("TEXCOORD");
	m_vd.AddElement(e);

	// allocate the tile vertex buffer memory
	size_t vertexSize = m_vd.GetVertexSize(0);
	size_t numVerts = mSprites.maxSize() * 4;
	size_t vertexBufferSize = vertexSize * numVerts;

	VertexBuffer::Parameters vbParams;
	vbParams.sourceBuffer = NULL;
	vbParams.access = VertexBuffer::CpuWrite;	// we want to write to the buffer
	vbParams.stride = vertexSize;
	vbParams.vertexCount = numVerts;
	vbParams.vertexSize = vertexSize;
	m_spriteVb = d.CreateVB( vbParams );

	IndexBuffer::Parameters ibParams;
	ibParams.format = IndexBuffer::IB_16BIT;
	ibParams.indexCount = mSprites.maxSize() * 6;
	ibParams.sourceBuffer = NULL;
	ibParams.access = IndexBuffer::CpuWrite;	// we want to write to the buffer
	m_spriteIb = d.CreateIB( ibParams );

	// Create the input layout
	m_inputLayout = d.CreateVertexInputLayout( m_shader, m_vd );	

	return true;
}

void SpriteRender::RemoveSprites()
{
	mSprites.deleteAll();
	mDirty = true;
}