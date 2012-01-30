#ifndef SPRITE_RENDER_INCLUDED
#define SPRITE_RENDER_INCLUDED

#include "core\array.h"
#include "framework\graphics\device_types.h"

class Spritemap;

class SpriteRender
{
public:
	struct Parameters
	{
		Parameters()
			: mMaxSprites(0)
			, spritemap(NULL)
		{
		}
		int mMaxSprites;
		Effect shader;
		// use EITHER a sprite map or texture, not both
		Spritemap* spritemap;
		Texture2D texture;
	};

	void RemoveSprites();

	void AddSprite( int spriteID, D3DXVECTOR2 position, D3DXVECTOR2 scale=D3DXVECTOR2(1.0f,1.0f) );
	void Draw( Device& device, D3DXVECTOR2 startPosition, D3DXVECTOR2 scale, const char* technique );

	bool Create( Device& d, Parameters& p );
	void Release( Device& d );

	inline Texture2D& GetTexture()
	{
		return m_texture;
	}

private:

	struct Sprite
	{
		D3DXVECTOR2 position;
		D3DXVECTOR2 scale;
		int spriteID;
	};

	void updateSpriteMesh( Device& d );
	bool initGraphics(Device& d);

	bool mDirty;
	Array<Sprite> mSprites;

	Spritemap* m_spritemap;
	Texture2D m_texture;
	Effect m_shader;
	VertexDescriptor m_vd;
	VertexBuffer m_spriteVb;
	IndexBuffer m_spriteIb;
	ShaderInputLayout m_inputLayout;
};

#endif