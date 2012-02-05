#include "biomorph_manager.h"
#include "framework\graphics\device.h"

BiomorphManager::BiomorphManager()
	: mDevice(NULL)
{
}

BiomorphManager::~BiomorphManager()
{
	Release();
}

bool BiomorphManager::Initialise( Device* d, Parameters& p )
{
	MorphRender::Parameters mp;
	mp.mTextureHeight = p.TextureSize;
	mp.mTextureWidth = p.TextureSize;

	if( !mMorphRenderer.Initialise( d, mp ) )
	{
		return false;
	}

	mDevice = d;

	return true;
}

void BiomorphManager::Release()
{
	mMorphRenderer.Release();

	for( BiomorphMap::iterator it = mBiomorphs.begin();
		it != mBiomorphs.end();
		++it )
	{
		if( (*it).second->mRefcount > 0 )
		{
			printf("Biomorph still has references!");
		}
		mDevice->Release( (*it).second->mTexture );
	}

	mBiomorphs.erase( mBiomorphs.begin(), mBiomorphs.end() );
}

bool BiomorphManager::GenerateBiomorph( MorphDNA& dna )
{
	StringHashing::StringHash morphHash = dna.GetHash();
	BiomorphMap::iterator it = mBiomorphs.find( morphHash );
	if( it != mBiomorphs.end() )
	{
		// test the values. if they dont match, we have a hash collision
		if( (*it).second->mDNA != dna )
		{
			printf("Hash collision! This is very bad!\n");
		}
	}
	else
	{
		// generate the biomorph texture
		mMorphRenderer.StartRendering();
		mMorphRenderer.DrawBiomorph( dna );
		mMorphRenderer.EndRendering();

		BiomorphBase* newBase = new BiomorphBase;
		if( newBase )
		{
			newBase->mDNA = dna;
			newBase->mRefcount = 0;
			newBase->mTexture = mMorphRenderer.CopyOutputTexture( newBase->mTexture );
			mBiomorphs.insert( BiomorphMapPair( morphHash, newBase ) );

			return true;
		}
	}

	return false;
}

void BiomorphManager::CleanupDatabase()
{
	for( BiomorphMap::iterator it = mBiomorphs.begin();
		it != mBiomorphs.end();
		++it )
	{
		if( (*it).second->mRefcount <= 0 )
		{
			mDevice->Release( (*it).second->mTexture );
			mBiomorphs.erase( it );
			it = mBiomorphs.begin();
		}
	}
}

BiomorphInstance BiomorphManager::CreateInstance( MorphDNA& dna )
{
	BiomorphInstance newInstance;

	BiomorphMap::iterator it = mBiomorphs.find( dna.GetHash() );
	if( it != mBiomorphs.end() )
	{
		(*it).second->mRefcount++;
		newInstance.mBase = (*it).second;
	}

	return newInstance;
}

void BiomorphManager::DestroyInstance( BiomorphInstance& instance )
{
	if( instance.IsValid() )
	{
		instance.mBase->mRefcount--;
	}
}