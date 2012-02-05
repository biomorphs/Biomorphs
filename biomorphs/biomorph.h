#ifndef BIOMORPH_H_INCLUDED
#define BIOMORPH_H_INCLUDED

#include "morph_dna.h"
#include "framework\graphics\device_types.h"

class BiomorphBase
{
friend class BiomorphManager;
friend class BiomorphInstance;
public:
	BiomorphBase()
		: mRefcount(0)
	{
		mTexture.Invalidate();
	}

	inline bool IsValid()
	{
		return mTexture.IsValid();
	}

private:
	Texture2D mTexture;
	MorphDNA mDNA;
	int mRefcount;
};

class BiomorphInstance
{
friend class BiomorphManager;
public:
	BiomorphInstance()
		: mBase(NULL)
	{
	}

	Texture2D* GetTexture()
	{
		if( mBase )
		{
			return &mBase->mTexture;
		}

		return NULL;
	}

	inline bool IsValid() const
	{
		return mBase != NULL;
	}

private:
	BiomorphBase* mBase;
};

#endif