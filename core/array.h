#ifndef ARRAY
#define ARRAY

#include <memory>

template<class T>
class Array
{
public:
	Array()
		: m_buffer(0)
		, m_tail(0)
		, m_head(0)
		, m_grow(false)
	{
	}

	~Array()
	{
		destroy();
	}

	void destroy()
	{
		if( m_buffer != 0 )
		{
			free(m_buffer);
			m_buffer = 0;
		}
		m_tail = 0;
		m_head = 0;
	}

	void deleteAll()
	{
		m_head = m_buffer;
	}

	inline T* operator[](unsigned int idx)
	{
		if( idx < (unsigned int)(m_head-m_buffer) )
		{
			return &(m_buffer[idx]);
		}

		return 0;
	}

	inline const size_t size() const
	{
		return (m_head-m_buffer);
	}

	inline const size_t maxSize() const
	{
		return (m_tail-m_buffer);
	}

	bool init(unsigned int size, bool grow=false)
	{
		if( size==0 || m_buffer != 0 )
		{
			return false;
		}

		T* buf = (T*)malloc(sizeof(T) * size);
		if( buf )
		{
			m_buffer = buf;
			m_head = buf;
			m_tail = m_buffer + size;
			m_grow=grow;

			return true;
		}

		return false;
	}

	void fastDelete(unsigned int index)
	{
		if( m_head > m_buffer && index < (unsigned int)(m_head-m_buffer) )
		{
			// call destructor on the object
			m_buffer[index].~T();

			// swap last living with this
			m_buffer[index] = *(m_head-1);
			m_head = m_head-1;
		}
	}

	// Useful for arrays of objects - no extra constructors called
	T* push_back()
	{
		if( m_head>=m_tail )
		{
			if( m_grow )
			{
				size_t newSize = (m_tail - m_buffer) * 2;
				T* newBuf = (T*)malloc(sizeof(T) * newSize);
				if( newBuf )
				{
					memcpy(newBuf,m_buffer,(m_tail-m_buffer) * sizeof(T));
					m_head = newBuf + (m_head - m_buffer);
					m_tail = newBuf + newSize;
					m_buffer = newBuf;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}

		if( m_buffer )
		{
			// Instantiate the object
			T* newVal = m_head;
			new(newVal) T;

			m_head = m_head+1;

			return newVal;
		}

		return 0;
	}

	T* push_back(const T& value)
	{
		T* newPtr = push_back();
		{
			*newPtr = value;
			return newPtr;
		}

		return 0;
	}

	void sort(int(*sortFn)(const void *, const void *))
	{
		qsort(m_buffer,m_head-m_buffer,sizeof(T),sortFn);
	}

private:
	T* m_buffer;
	T* m_tail;
	T* m_head;
	bool m_grow;
};

#endif