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
		int mMaxSprites;
		Effect shader;
		Spritemap* spritemap;
	};

	void RemoveSprites();

	void AddSprite( int spriteID, D3DXVECTOR2 position );
	void Draw( ShadowedDevice& device, D3DXVECTOR2 startPosition, D3DXVECTOR2 scale );

	bool Create( Device& d, Parameters& p );
	void Release( Device& d );

private:

	struct Sprite
	{
		D3DXVECTOR2 position;
		int spriteID;
	};

	void updateSpriteMesh( ShadowedDevice& d );
	bool initGraphics(Device& d);

	bool mDirty;
	Array<Sprite> mSprites;

	Spritemap* m_spritemap;
	Effect m_shader;
	VertexDescriptor m_vd;
	VertexBuffer m_spriteVb;
	IndexBuffer m_spriteIb;
	ShaderInputLayout m_inputLayout;
};

#endif