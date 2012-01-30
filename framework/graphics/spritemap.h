#ifndef SPRITE_MAP_INCLUDED
#define SPRITE_MAP_INCLUDED

#include "core\array.h"
#include "framework\graphics\device_types.h"

class Spritemap
{
public:
	void Init(int maxSprites, Texture2D texture);
	void Release();
	void AddSprite(unsigned int id, D3DXVECTOR2 uv0, D3DXVECTOR2 uv1);
	bool GetSprite(unsigned int id, D3DXVECTOR2& uv0, D3DXVECTOR2& uv1);

	inline Texture2D GetTexture()
	{
		return mTexture;
	}
private:
	class Sprite
	{
	public:
		D3DXVECTOR2 mUV0;
		D3DXVECTOR2 mUV1;
		unsigned int mID;
	};

	Array<Sprite> mSprites;
	Texture2D mTexture;
};

#endif