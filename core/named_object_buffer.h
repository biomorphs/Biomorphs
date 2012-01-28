#ifndef NAMED_OBJECT_BUFFER_INCLUDED
#define NAMED_OBJECT_BUFFER_INCLUDED

#include "core/string_hashing.h"
#include <map>

class NamedObjectBuffer
{
public:
	typedef unsigned int NamedObjectType;

	NamedObjectBuffer();
	~NamedObjectBuffer();
private:
	class NamedObject
	{
	public:
		size_t Offset;
		size_t Size;
		NamedObjectType Type;
	};

	typedef std::map< StringHashing::StringHash, NamedObject > ObjectMap;
	typedef std::pair< StringHashing::StringHash, NamedObject > ObjectPair;
public:

	class NamedObjectIterator
	{
	friend class NamedObjectBuffer;
	public:
		inline bool HasData()
		{
			return m_it != m_buffer->m_objects.end();
		}

		inline bool Next()
		{
			++m_it;

			return HasData();
		}

		inline NamedObjectBuffer::NamedObjectType getType() const 
		{
			return( m_it->second.Type );
		}

		inline size_t getSize() const 
		{
			return( m_it->second.Size );
		}

		void* getValue()
		{
			return (void*)((size_t)m_buffer->m_head + m_it->second.Offset);
		}
	private:

		NamedObjectIterator( NamedObjectBuffer* buffer )
			: m_buffer(buffer)
		{
			m_it = m_buffer->m_objects.begin();
		}

		NamedObjectBuffer* m_buffer;
		NamedObjectBuffer::ObjectMap::iterator m_it;
	};

	bool initialise( size_t sizeRequired );

	template< class TYPE >
	bool addObject( std::string name, NamedObjectType type, TYPE value );

	bool addObjectRaw( std::string name, NamedObjectType type, size_t size, void* value );

	NamedObjectIterator getIterator()
	{
		return NamedObjectIterator( this );
	}

protected:
private:

	void*   m_tail;
	void*	m_head;
	size_t	m_bufferSize;

	ObjectMap m_objects;
};

template< class TYPE >
bool NamedObjectBuffer::addObject( std::string name, NamedObjectType type, TYPE value )
{
	return addObjectRaw( name, type, sizeof(value), &value );
}

#endif