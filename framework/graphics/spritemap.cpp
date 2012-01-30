#include "spritemap.h"

void Spritemap::Init(int maxSprites, Texture2D texture)
{
	mSprites.init(maxSprites);

	mTexture = texture;
}

void Spritemap::Release()
{
	mSprites.destroy();
}

void Spritemap::AddSprite(unsigned int id, D3DXVECTOR2 uv0, D3DXVECTOR2 uv1)
{
	for(unsigned int s=0;s<mSprites.size();++s)
	{
		if( mSprites[s]->mID==id )
		{
			return;
		}
	}

	Sprite spr;
	spr.mID = id;
	spr.mUV0 = uv0;
	spr.mUV1 = uv1;
	mSprites.push_back(spr);
}

bool Spritemap::GetSprite(unsigned int id, D3DXVECTOR2& uv0, D3DXVECTOR2& uv1)
{
	for(unsigned int s=0;s<mSprites.size();++s)
	{
		if( mSprites[s]->mID==id )
		{
			uv0 = mSprites[s]->mUV0;
			uv1 = mSprites[s]->mUV1;
			return true;
		}
	}
	return false;
}