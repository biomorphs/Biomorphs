#ifndef LINEAR_ALLOCATOR_H_INCLUDED
#define LINEAR_ALLOCATOR_H_INCLUDED

#include <stdlib.h>

// this needs some work. allocs too much on properly aligned ptrs
#define ALIGN_UP(ptr,align) ((size_t)ptr + (align - ((size_t)ptr % align)))

class LinearAllocator
{
public:
	LinearAllocator()
		: mBuffer(NULL)
		, mHead(NULL)
		, mTail(NULL)
	{
	}

	~LinearAllocator()
	{
		Release();
	}

	inline bool Initialise(size_t maxSize)
	{
		if( mBuffer )		// dont alloc twice
		{
			return false;
		}

		void* buffer = malloc( maxSize );
		if( !buffer )
		{
			return false;
		}

		mBuffer = buffer;
		mHead = buffer;
		mTail = (void*)((size_t)buffer + maxSize);

		memset( mBuffer, 0xcdcdcdcd, maxSize );

		return true;
	}

	inline void* Allocate( size_t size, size_t align=4 )
	{
		// align up the head ptr
		size_t headPtr = ALIGN_UP(mHead, align);

		if( headPtr + size < (size_t)mTail )
		{
			mHead = (void*)((size_t)mHead + size);
			return (void*)headPtr;
		}

		return NULL;
	}

	inline void Release()
	{
		if( mBuffer )
		{
			free( mBuffer );
			mBuffer = mHead = mTail = NULL;
		}
	}

private:
	void* mBuffer;
	void* mHead;
	void* mTail;
};

#endif