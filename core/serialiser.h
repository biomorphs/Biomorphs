#ifndef SERIALISER_H_INCLUDED
#define SERIALISER_H_INCLUDED

#include "string_hashing.h"
#include "linear_allocator.h"
#include <list>

class Serialiser
{
public:
	Serialiser(size_t maxSize = 8 * 1024)
	{
		mAllocator.Initialise( maxSize );
	}
	~Serialiser()
	{
		mAllocator.Release();
	}

	template<typename T>
	void AddNode( const char* key, const T& value )
	{
		// add an unknown node
		SerialNode* nodePtr = (SerialNode*)mAllocator.Allocate( sizeof(SerialNode) );
		if( nodePtr )
		{
			void* dataPtr = mAllocator.Allocate( sizeof(value) );
			if( dataPtr != NULL )
			{
				memcpy( dataPtr, &value, sizeof(value ) );
				nodePtr->mPtr = dataPtr;
				nodePtr->mSize = sizeof(value);
				nodePtr->mType = _nodeType( value );

				StringHashing::StringHash keyHash = StringHashing::getHash( key );
				mNodes.push_back( SerialiserListNode( keyHash, nodePtr ) );
			}
		}
	}

private:
	enum SerialisedType
	{
		TYPE_INT,
		TYPE_UINT,
		TYPE_UNKNOWN
	};

	struct SerialNode
	{
		void* mPtr;
		size_t mSize;
		SerialisedType mType;
	};

	template<typename T>
	SerialisedType _nodeType( const T& value )
	{
		return TYPE_UNKNOWN;
	}

	typedef std::pair<StringHashing::StringHash, SerialNode*> SerialiserListNode;
	typedef std::list<SerialiserListNode> SerialiserList;

	LinearAllocator mAllocator;
	SerialiserList mNodes;
};

#endif