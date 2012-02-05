#ifndef BIOMORPH_MANAGER_H_INCLUDED
#define BIOMORPH_MANAGER_H_INCLUDED

#include "biomorph.h"
#include "morph_render.h"
#include <map>

class Device;
class BiomorphManager
{
public:
	BiomorphManager();
	~BiomorphManager();

	struct Parameters
	{
		int TextureSize;
	};

	bool Initialise( Device* d, Parameters& p );
	void Release();

	bool GenerateBiomorph( MorphDNA& dna );
	void CleanupDatabase();	// removes unreferenced biomorphs

	// instance creation / destruction
	BiomorphInstance CreateInstance( MorphDNA& dna );
	void DestroyInstance( BiomorphInstance& instance );

private:	
	typedef std::map<StringHashing::StringHash, BiomorphBase*> BiomorphMap;
	typedef std::pair<StringHashing::StringHash, BiomorphBase*> BiomorphMapPair;

	Device* mDevice;
	MorphRender mMorphRenderer;
	BiomorphMap mBiomorphs;
};

#endif