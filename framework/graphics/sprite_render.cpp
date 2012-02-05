#include "sprite_render.h"
#include "spritemap.h"
#include "framework\graphics\shadowed_device.h"

struct SpriteVertex
{
	D3DXVECTOR2 mPosition;
	D3DXVECTOR2 mUV;	
};

void SpriteRender::AddSprite( int spriteID, D3DXVECTOR2 position, D3DXVECTOR2 scale )
{
	Sprite sp;
	sp.position = position;
	sp.scale = scale;
	sp.spriteID = spriteID;
	mSprites.push_back(sp);
	mDirty = true;
}

void SpriteRender::Draw( Device& device, D3DXVECTOR2 startPosition, D3DXVECTOR2 scale, const char* technique )
{
	if( mDirty )
	{
		updateSpriteMesh( device );
		mDirty = false;
	}

	EffectTechnique t = m_shader.GetTechniqueByName(technique);
	TextureSampler sampler = t.GetSamplerByName("BlitTexture");
	VectorConstant posScale = t.GetVectorConstant("PositionScale");
	if( m_texture.IsValid() )
	{
		sampler.Set( m_texture );
	}
	else
	{
		sampler.Set( m_spritemap->GetTexture() );
	}

	posScale.Set(D3DXVECTOR4(startPosition.x, startPosition.y, scale.x, scale.y));
	posScale.Apply();

	device.SetTechnique(t, 0);
	device.SetInputLayout(m_inputLayout);
	device.SetPrimitiveTopology(PRIMITIVE_TRIANGLES);

	device.SetIndexBuffer(m_spriteIb);
	device.SetVertexBuffer(0, m_spriteVb);

	DrawIndexedParameters dp;
	dp.m_indexCount = (unsigned int)mSprites.size() * 6;
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
	m_texture = p.texture;

	return initGraphics(d);
}

void SpriteRender::updateSpriteMesh( Device& dd )
{
	// lock the vertex buffer
	SpriteVertex* vertices = (SpriteVertex*)dd.LockVB(m_spriteVb);

	int tilesDrawn=0;
	for(unsigned int a=0;a<mSprites.size();++a)
	{
		int sid = mSprites[a]->spriteID;
		D3DXVECTOR2 uv0(0.0f,0.0f); 
		D3DXVECTOR2 uv1(1.0f,1.0f);
		
		if( m_spritemap != NULL )
		{
			m_spritemap->GetSprite(sid,uv0,uv1);
		}

		SpriteVertex v;
		// push a quad for each tile
		D3DXVECTOR2 pos = mSprites[a]->position;
		D3DXVECTOR2 scale = mSprites[a]->scale;

		v.mPosition = pos + D3DXVECTOR2(0.0f, 0.0f);		v.mUV = D3DXVECTOR2(uv0.x,uv1.y);
		*vertices = v;	++vertices;

		v.mPosition = pos + D3DXVECTOR2(scale.x, 0.0f);		v.mUV = uv1;
		*vertices = v;	++vertices;
			
		v.mPosition = pos + scale;	v.mUV = D3DXVECTOR2(uv1.x,uv0.y);
		*vertices = v;	++vertices;			

		v.mPosition = pos + D3DXVECTOR2(0.0f, scale.y);		v.mUV = uv0;	
		*vertices = v;	++vertices;
		++tilesDrawn;
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
	unsigned int vertexSize = m_vd.GetVertexSize(0);
	unsigned int numVerts = (unsigned int)mSprites.maxSize() * 4;
	unsigned int vertexBufferSize = vertexSize * numVerts;

	VertexBuffer::Parameters vbParams;
	vbParams.sourceBuffer = NULL;
	vbParams.access = VertexBuffer::CpuWrite;	// we want to write to the buffer
	vbParams.stride = vertexSize;
	vbParams.vertexCount = numVerts;
	vbParams.vertexSize = vertexSize;
	m_spriteVb = d.CreateVB( vbParams );

	IndexBuffer::Parameters ibParams;
	ibParams.format = IndexBuffer::IB_16BIT;
	ibParams.indexCount = (unsigned int)mSprites.maxSize() * 6;
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