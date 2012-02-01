#ifndef MORPH_DNA_INCLUDED
#define MORPH_DNA_INCLUDED

#include "core/angles.h"
#include "core/minmax.h"
#include "core/random.h"

typedef unsigned long long uint_64;

#define MORPH_DNA_INLINE __forceinline

// biomorph dna structure
struct MorphDNA
{
	MorphDNA()
		: mFullSequence0(0)
		, mFullSequence1(0)
	{
	}
	union
	{
		struct
		{
			unsigned int mBranchDepth : 4;				// 1 to 16 branches (minimum of 1)
			unsigned int mBranchInitialAngle : 7;		// 0 to 127 quantised to 0 - 180 degrees
			unsigned int mBranchInitialLength : 6;		// 0 to 63 quantised to 0 - 1
			unsigned int mBranchLengthModifier : 8;		// 0 to 255 quantised to 0.0 to 2.0
			unsigned int mBranchAngleModifier : 8;		// 0 to 255 quantised to 0.0 to 2.0
			unsigned int mBaseColourRed : 5;			// 0 to 31 quantised to 0.0 to 1.0
			unsigned int mBaseColourGreen : 5;			// 0 to 31 quantised to 0.0 to 1.0
			unsigned int mBaseColourBlue : 5;			// 0 to 31 quantised to 0.0 to 1.0
			unsigned int mBranchRedModifier : 8;		// 0 to 255 quantised to 0.0 - 2.0
			unsigned int mBranchGreenModifier : 8;		// 0 to 255 quantised to 0.0 - 2.0
			unsigned int mBranchBlueModifier : 8;		// 0 to 255 quantised to 0.0 - 2.0
		};
		struct
		{
			unsigned int mFullSequenceHigh0;
			unsigned int mFullSequenceLow0;
			unsigned int mFullSequenceHigh1;
			unsigned int mFullSequenceLow1;
		};
		struct
		{
			uint_64 mFullSequence0; 
			uint_64 mFullSequence1; 
		};
	};
};

// dna builder helper
inline MorphDNA MAKEDNA(unsigned int branches, 
						float initAngle, 
						float initLength,
						float lengthMod,
						float angleMod,
						D3DXVECTOR3 baseColour,
						D3DXVECTOR3 colourMod)
{
	MorphDNA dna;
	dna.mBranchDepth = branches;
	dna.mBranchInitialLength = (unsigned int)(initLength * 63.0f);
	dna.mBranchLengthModifier = (unsigned int)(lengthMod * 0.5f * 255.0f);
	dna.mBranchInitialAngle = (unsigned int)((Angles::ToRadians(initAngle) / Angles::PI) * 127.0f);
	dna.mBranchAngleModifier = (unsigned int)(angleMod * 0.5f * 255.0f);

	dna.mBaseColourRed = (unsigned int)(baseColour.x * 31.0f);
	dna.mBaseColourGreen = (unsigned int)(baseColour.y * 31.0f);
	dna.mBaseColourBlue = (unsigned int)(baseColour.z * 31.0f);

	dna.mBranchRedModifier = (unsigned int)(colourMod.x * 0.5f * 255.0f);
	dna.mBranchGreenModifier = (unsigned int)(colourMod.y * 0.5f * 255.0f);
	dna.mBranchBlueModifier = (unsigned int)(colourMod.z * 0.5f * 255.0f);

	return dna;
}

// branch depth (num recursions)
MORPH_DNA_INLINE int BASEDEPTH(const MorphDNA& dna)
{
	return 1 + dna.mBranchDepth;
}

// branch base angle
MORPH_DNA_INLINE float BASEANGLE(const MorphDNA& dna)
{
	return ((float)(dna.mBranchInitialAngle / 127.0f) * Angles::PI);
}

// branch base length 
MORPH_DNA_INLINE float BASELENGTH(const MorphDNA& dna)
{
	return (float)(dna.mBranchInitialLength / 63.0f);
}

// base colour
MORPH_DNA_INLINE D3DXVECTOR4 BASECOLOUR(const MorphDNA& dna)
{
	float red = (float)dna.mBaseColourRed / 31.0f;
	float green = (float)dna.mBaseColourGreen / 31.0f;
	float blue = (float)dna.mBaseColourBlue / 31.0f;
	const float alpha = 1.0f;

	return D3DXVECTOR4( red, green, blue, alpha );
}

// branch length modifier
MORPH_DNA_INLINE float BRANCHLENGTHMOD(const MorphDNA& dna) 
{
	return (float)(dna.mBranchLengthModifier / 255.0f) * 2.0f;
}

// branch angle modifier
MORPH_DNA_INLINE float BRANCHANGLEMOD(const MorphDNA& dna) 
{
	return ((float)(dna.mBranchAngleModifier / 255.0f) * 2.0f);
}

// branch red modifier
MORPH_DNA_INLINE float BRANCHREDMOD(const MorphDNA& dna) 
{
	return ((float)(dna.mBranchRedModifier / 255.0f) * 2.0f);
}

// branch green modifier
MORPH_DNA_INLINE float BRANCHGREENMOD(const MorphDNA& dna) 
{
	return ((float)(dna.mBranchGreenModifier / 255.0f) * 2.0f);
}

// branch blue modifier
MORPH_DNA_INLINE float BRANCHBLUEMOD(const MorphDNA& dna) 
{
	return ((float)(dna.mBranchBlueModifier / 255.0f) * 2.0f);
}

// branch length for a specific branch
MORPH_DNA_INLINE float BRANCHLENGTH(const MorphDNA& dna, int depth )
{
	float mod = pow(BRANCHLENGTHMOD(dna), BASEDEPTH(dna) - depth);
	return mod * BASELENGTH(dna);
}

// branch angle for a specific branch 
MORPH_DNA_INLINE float BRANCHANGLE(const MorphDNA&dna, int depth)	
{
	float mod = pow(BRANCHANGLEMOD(dna), BASEDEPTH(dna) - depth);
	return mod * BASEANGLE(dna);
}

// branch colour for a specific branch
MORPH_DNA_INLINE D3DXVECTOR4 BRANCHCOLOUR(const MorphDNA&dna, int depth)	
{
	int d = BASEDEPTH(dna) - depth;

	D3DXVECTOR4 baseColour = BASECOLOUR(dna);

	float mod = pow(BRANCHREDMOD(dna), d);
	float r = mod * baseColour.x;

	mod = pow(BRANCHGREENMOD(dna), d);
	float g = mod * baseColour.y;

	mod = pow(BRANCHBLUEMOD(dna), d);
	float b = mod * baseColour.z;

	return D3DXVECTOR4( r, g, b, 1.0f );
}

MORPH_DNA_INLINE int MutateGene(  int originalValue,
						int modification,
						int valueMin,
						int valueMax )
{
	int bd = originalValue + modification;
	bd = Bounds::Max( valueMin, bd );
	return Bounds::Min( valueMax, bd );
}

inline void MutateDNA( MorphDNA& dna )
{
	int gene = Random::getInt(0,10);
	int direction = Random::getInt(0,100);
	int dir = direction > 50 ? 1 : -1;

	switch(gene)
	{
	case 0:
		dna.mBranchDepth = MutateGene( dna.mBranchDepth, dir, 15, 15 );
		break;
	case 1:
		dna.mBranchInitialAngle = MutateGene( dna.mBranchInitialAngle, dir * 2, 1, 127 );
		break;
	case 2:
		dna.mBranchInitialLength = MutateGene( dna.mBranchInitialLength, dir * 1, 1, 63 );
		break;
	case 3:
		dna.mBranchLengthModifier = MutateGene( dna.mBranchLengthModifier, dir * 4, 1, 255 );
		break;
	case 4:
		dna.mBranchAngleModifier = MutateGene( dna.mBranchAngleModifier, dir * 4, 1, 255 );
		break;
	case 5:
		dna.mBaseColourRed = MutateGene( dna.mBaseColourRed, dir * 1, 31, 31 );
		break;
	case 6:
		dna.mBaseColourGreen = MutateGene( dna.mBaseColourGreen, dir * 1, 31, 31 );
		break;
	case 7:
		dna.mBaseColourBlue = MutateGene( dna.mBaseColourBlue, dir * 1, 31, 31 );
		break;
	case 8:
		dna.mBranchRedModifier = MutateGene( dna.mBranchRedModifier, dir * 3, 1, 255 );
		break;
	case 9:
		dna.mBranchGreenModifier = MutateGene( dna.mBranchGreenModifier, dir * 3, 1, 255 );
		break;
	case 10:
		dna.mBranchBlueModifier = MutateGene( dna.mBranchBlueModifier, dir * 3, 1, 255 );
		break;
	}
}

#endif