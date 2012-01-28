#ifndef STRING_HASH_INCLUDED
#define STRING_HASH_INCLUDED

class StringHashing
{
public:
	typedef unsigned long StringHash;

	// djb2 hash
	static inline unsigned long getHash( const char* str )
	{
		unsigned long hash = 5381;
		int c=0;

		while( c = *str++ )
		{
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}

		return hash;
	}
};

#endif