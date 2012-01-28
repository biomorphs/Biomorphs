#ifndef RANDOM_INCLUDED
#define RANDOM_INCLUDED

namespace Random
{
	inline void seed(int seed)
	{
		srand(seed);
	}

	inline float getFloat( float min, float max )
	{
		float r = (float)rand() / RAND_MAX;
		return min + ((max-min) * r);
	}

	inline int getInt( int min, int max )
	{
		float r = (float)rand() / RAND_MAX;
		return min + (int)((max-min) * r);
	}

	inline int getUInt( unsigned int min, unsigned int max )
	{
		float r = (float)rand() / RAND_MAX;
		return min + (unsigned int)((max-min) * r);
	}
}

#endif