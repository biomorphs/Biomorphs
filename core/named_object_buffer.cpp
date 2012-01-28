#include "core/named_object_buffer.h"

NamedObjectBuffer::NamedObjectBuffer()
	: m_tail(0)
	, m_head(0)
	, m_bufferSize(0)
{
}

NamedObjectBuffer::~NamedObjectBuffer()
{
}

bool NamedObjectBuffer::addObjectRaw( std::string name, NamedObjectType type, size_t size, void* value )
{
	StringHashing::StringHash hash = StringHashing::getHash( name.c_str() );
	size_t available = (size_t)m_head + m_bufferSize - (size_t)m_tail;
	if( size > available )
	{
		return false;	// out of space
	}

	if( m_objects.find( hash ) != m_objects.end() ) // already exists
	{
		return false;
	}

	NamedObject newObject;
	newObject.Offset = (size_t)m_tail - (size_t)m_head;
	newObject.Size = size;
	newObject.Type = type;

	// copy our data into the buffer
	memcpy( m_tail, value, newObject.Size );

	m_objects.insert(ObjectPair(hash, newObject));
	m_tail = (void*)((size_t)m_tail + newObject.Size);

	return true;
}

bool NamedObjectBuffer::initialise( size_t sizeRequired )
{
	m_head = malloc( sizeRequired );
	if( NULL==m_head )
	{
		return false;
	}

	m_tail = m_head;
	m_bufferSize = sizeRequired;

	return true;
}