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

	class SerialNode
	{
	friend class Serialiser;
	public:
		template< typename T >
		bool GetValue( T& result )
		{
			return false;
		}
	private:
		void* mPtr;
		size_t mSize;
		int mType;
	};

	template<typename T>
	inline void AddNode( const char* key, const T& value )
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

	// specialised for strings
	template<>
	inline void AddNode( const char* key, const char* const &value )
	{
		// add an unknown node
		SerialNode* nodePtr = (SerialNode*)mAllocator.Allocate( sizeof(SerialNode) );
		if( nodePtr )
		{
			size_t stringSize = (strlen(value) + 1) * sizeof(char);
			char* dataPtr = (char*)mAllocator.Allocate( stringSize );
			if( dataPtr != NULL )
			{
				memset( dataPtr, '\0', stringSize );
				memcpy( dataPtr, value, stringSize );
				nodePtr->mPtr = (void*)dataPtr;
				nodePtr->mSize = stringSize;
				nodePtr->mType = TYPE_STRING;

				StringHashing::StringHash keyHash = StringHashing::getHash( key );
				mNodes.push_back( SerialiserListNode( keyHash, nodePtr ) );
			}
		}
	}

	inline SerialNode* GetNode( const char* key )
	{
		StringHashing::StringHash hash = StringHashing::getHash( key );
		for( SerialiserList::iterator it = mNodes.begin();
			it != mNodes.end();
			++it )
		{
			if( (*it).first == hash )
			{
				return (*it).second;
			}
		}

		return NULL;
	}

private:
	enum SerialisedType
	{
		TYPE_INT,
		TYPE_UINT,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_UNKNOWN
	};

	template<typename T>
	inline SerialisedType _nodeType( const T& value )
	{
		return TYPE_UNKNOWN;
	}

	template<typename T>
	inline T* _nodeValue(SerialNode* node)
	{
		return NULL;
	}

	typedef std::pair<StringHashing::StringHash, SerialNode*> SerialiserListNode;
	typedef std::list<SerialiserListNode> SerialiserList;

	LinearAllocator mAllocator;
	SerialiserList mNodes;
};

// Serialiser node data types

template<>
inline Serialiser::SerialisedType Serialiser::_nodeType( const int& value )
{
	return TYPE_INT;
}

template<>
inline Serialiser::SerialisedType Serialiser::_nodeType( const unsigned int& value )
{
	return TYPE_UINT;
}

template<>
inline Serialiser::SerialisedType Serialiser::_nodeType( const float& value )
{
	return TYPE_FLOAT;
}

// serialiser node data fetchers

template<>
inline bool Serialiser::SerialNode::GetValue( int& result )
{
	if( mType == TYPE_INT )
	{
		int* iPtr = (int*)mPtr;
		result = *iPtr;

		return true;
	}

	return false;
}

template<>
inline bool Serialiser::SerialNode::GetValue( unsigned int& result )
{
	if( mType == TYPE_UINT )
	{
		unsigned int* iPtr = (unsigned int*)mPtr;
		result = *iPtr;

		return true;
	}

	return false;
}

template<>
inline bool Serialiser::SerialNode::GetValue( float& result )
{
	if( mType == TYPE_FLOAT )
	{
		float* iPtr = (float*)mPtr;
		result = *iPtr;

		return true;
	}

	return false;
}

template<>
inline bool Serialiser::SerialNode::GetValue( char* &result )
{
	if( mType == TYPE_STRING )
	{
		char* iPtr = (char*)mPtr;
		strcpy( result, iPtr );

		return true;
	}

	return false;
}



#endif