#ifndef VERTEX_DESCRIPTOR_INCLUDED
#define VERTEX_DESCRIPTOR_INCLUDED

#include <list>
#include <D3D10.h>

class Device;

struct VertexElement
{
	enum Format
	{
		VTX_FLOAT2 = DXGI_FORMAT_R32G32_FLOAT,
		VTX_FLOAT3 = DXGI_FORMAT_R32G32B32_FLOAT,
		VTX_FLOAT4 = DXGI_FORMAT_R32G32B32A32_FLOAT,
	};

	static inline unsigned int GetVertexFormatSize(unsigned int f)
	{
		switch(f)
		{
		case VTX_FLOAT2:
			return sizeof(D3DXVECTOR2);
			break;
		case VTX_FLOAT3:
			return sizeof(D3DXVECTOR3);
			break;
		case VTX_FLOAT4:
			return sizeof(D3DXVECTOR4);
			break;
		default:
			return 0;
		}
	}

	VertexElement()
	{
		semanticIndex = 0;
		format = 0;
		streamIndex = 0;
		byteOffset = 0;
		elementType = PerVertex;
		instanceDrawStep = 0;
	}

	inline void SetSemanticName(const char* name)
	{
		strcpy_s(semanticName, name);
	}

	VertexElement& operator=(const VertexElement& e)
	{
		SetSemanticName(e.semanticName);
		semanticIndex = semanticIndex;
		format = e.format;
		streamIndex = e.streamIndex;
		byteOffset = e.byteOffset;
		elementType = e.elementType;
		instanceDrawStep = e.instanceDrawStep;

		return *this;
	}

	char semanticName[32];
	unsigned int semanticIndex;
	unsigned int format;
	unsigned int streamIndex;
	unsigned int byteOffset;
	
	enum ElementType
	{
		PerVertex = D3D10_INPUT_PER_VERTEX_DATA,
		PerInstance = D3D10_INPUT_PER_INSTANCE_DATA
	};

	ElementType elementType;
	unsigned int instanceDrawStep;
};

class VertexDescriptor
{
friend class Device;
public:
	inline void AddElement( const VertexElement& e )
	{
		m_elements.insert(m_elements.end(),e);
	}

	inline void Invalidate()
	{
		m_elements.clear();
	}

	inline unsigned int GetVertexSize(int streamIndex)
	{
		// Find the element with the highest offset, take its stride + size
		ElementIterator largestOffsetVert = m_elements.end();
		unsigned int largestOffset = 0;
		for(ElementIterator it = m_elements.begin(); 
			it != m_elements.end();
			++it)
		{
			if( it->streamIndex == streamIndex && it->byteOffset>=largestOffset )
			{
				largestOffset = it->byteOffset;
				largestOffsetVert = it;
			}
		}

		if( largestOffsetVert != m_elements.end() )
		{
			return largestOffset + VertexElement::GetVertexFormatSize(largestOffsetVert->format);
		}
		
		return 0;
	}

private:
	typedef std::list<VertexElement> ElementList;
	typedef std::list<VertexElement>::iterator ElementIterator;

	ElementList m_elements;
};

#endif