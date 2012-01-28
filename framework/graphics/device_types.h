#ifndef DEVICE_TYPES_INCLUDED
#define DEVICE_TYPES_INCLUDED

#include <D3DX10.h>
#include "vertex_descriptor.h"
#include "core\vector2.h"
#include "core\string_hashing.h"

// Forward declared D3D stuff here
// Do it so this file can be kept completely clean of implementation
struct ID3D10Effect;
struct ID3D10EffectTechnique;
struct ID3D10Buffer;
struct ID3D10Texture2D;
struct ID3D10DepthStencilView;
struct ID3D10RenderTargetView;
struct ID3D10InputLayout;
struct ID3DX10Font;

enum PrimitiveTopology
{
	PRIMITIVE_POINTS			= D3D10_PRIMITIVE_TOPOLOGY_POINTLIST,
	PRIMITIVE_LINES				= D3D10_PRIMITIVE_TOPOLOGY_LINELIST,
	PRIMITIVE_LINESTRIP			= D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP,
	PRIMITIVE_TRIANGLES			= D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	PRIMITIVE_TRIANGLESTRIP		= D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	PRIMITIVE_NA				= 0xbad1fff
};

class Texture2D;
class EffectTechnique;

struct DrawIndexedParameters
{
	DrawIndexedParameters()
		: m_startIndex(0)
		, m_indexCount(0) 
	{
	}
	int m_pass;
	unsigned int m_startIndex;
	unsigned int m_indexCount;
};

class Font
{
friend class Device;
public:
	enum Justification
	{
		DRAW_LEFT = DT_LEFT,
		DRAW_RIGHT = DT_RIGHT,
		DRAW_CENTER = DT_CENTER,
	};

	struct DrawParameters
	{
		DrawParameters()
			: mJustification(DRAW_LEFT)
		{
			mColour[0] = 1.0f;
			mColour[1] = 1.0f;
			mColour[2] = 1.0f;
			mColour[3] = 1.0f;
		}

		Justification mJustification;
		float mColour[4];
	};

	Font()
	{
		Invalidate();
	}

	enum FontFlags
	{
		ITALIC
	};

	struct Parameters
	{
		void SetName( char *name )
		{
			strcpy_s( mTypeface, name );
		}
		int mSize;
		int mWeight;
		int mFlags;
		char mTypeface[64];
	};

	inline bool IsValid()
	{
		return mFont != NULL && mSprites != NULL;
	}

	inline void Invalidate()
	{
		mFont = NULL;
		mSprites = NULL;
	}

private:
	ID3DX10Font* mFont;
	ID3DX10Sprite* mSprites;
	Parameters mParams;
};

class ShaderInputLayout
{
friend class Device;
public:
	ShaderInputLayout()
		: m_layout(NULL)
	{
	}

	inline bool IsValid() const
	{
		return m_layout != NULL;
	}

	inline void Invalidate()
	{
		m_layout = NULL;
	}

	inline bool operator!=(const ShaderInputLayout& rt) const
	{
		return (rt.m_layout != m_layout);
	}
private:
	ID3D10InputLayout* m_layout;
};

class TextureSampler
{
friend class EffectTechnique;
friend class ShadowedDevice;
public:
	TextureSampler(EffectTechnique* parent=NULL)
		: m_sampler(NULL)
		, m_effect(parent)
	{
	}
	void Set(Texture2D& t);
private:
	ID3D10EffectShaderResourceVariable* m_sampler;
	EffectTechnique* m_effect;
};

class MatrixConstant
{
friend class EffectTechnique;
friend class ShadowedDevice;
public:
	MatrixConstant(EffectTechnique* parent=NULL)
		: m_variable(NULL)
	{
	}
	void Set(D3DXMATRIX mat);
	void Apply();
private:
	ID3D10EffectMatrixVariable* m_variable;
	D3DXMATRIX m_value;
};

class VectorConstant
{
friend class EffectTechnique;
friend class ShadowedDevice;
public:
	VectorConstant(EffectTechnique* parent=NULL)
		: m_variable(NULL)
	{
	}
	void Set(D3DXVECTOR4 vec);
	void Apply();
private:
	ID3D10EffectVectorVariable* m_variable;
	D3DXVECTOR4 m_value;
};

class EffectTechnique
{
friend class Device;
friend class Effect;
friend class VectorConstant;
friend class MatrixConstant;
friend class TextureSampler;
friend class ShadowedDevice;
public:
	EffectTechnique()
		: m_technique(NULL)
		, m_passCount(0)
		, m_parent(NULL)
	{
	}

	struct Parameters
	{
		char techniqueName;
	};

	inline bool IsValid() const
	{
		return m_technique != NULL && m_passCount != 0;
	}

	void Invalidate()
	{
		m_technique = NULL;
	}

	inline bool operator!=(const EffectTechnique& rt) const
	{
		return (rt.m_technique != m_technique);
	}

	MatrixConstant GetMatrixConstant(const char* name);
	VectorConstant GetVectorConstant(const char* name);
	TextureSampler GetSamplerByName(const char* name);
private:
	ID3D10EffectTechnique* m_technique;
	Effect* m_parent;
	int m_passCount;
	bool m_dirty;
};

class Effect
{
friend class Device;
friend class EffectTechnique;
public:
	Effect()
		: m_effect(NULL)
	{
	}

	struct Parameters
	{
		Parameters( const char* effectFile )
		{
			strcpy_s( effectName, effectFile );
		}

		char effectName[64];
	};

	inline bool IsValid() const
	{
		return m_effect != NULL;
	}

	inline void Invalidate()
	{
		m_effect = NULL;
	}

	inline bool operator!=(const Effect& rt) const
	{
		return (rt.m_effect != m_effect);
	}

	EffectTechnique GetTechniqueByName(const char* name);
private:
	ID3D10Effect* m_effect;
};

class IndexBuffer
{
friend class Device;
public:
	enum CPUAccess
	{
		CpuNoAccess = 0,
		CpuWrite = D3D10_MAP_WRITE_DISCARD
	};

	enum Format
	{
		IB_16BIT=DXGI_FORMAT_R16_UINT,
		IB_32BIT=DXGI_FORMAT_R32_UINT
	};

	struct Parameters
	{
		Parameters()
			: sourceBuffer(NULL)
			, format(IB_16BIT)
			, access(CpuNoAccess)
		{
		}

		size_t indexCount;
		Format format;
		CPUAccess access;
		void* sourceBuffer;	// if this is NULL, the object creates its own buffer
	};

	IndexBuffer()
		: m_buffer(NULL)
		, m_indexCount(0)
		, m_ownedBuffer(NULL)
	{
	}

	inline bool IsValid() const
	{
		return m_buffer != NULL;
	}

	inline void Invalidate()
	{
		m_buffer = NULL;
		m_ownedBuffer = NULL;
	}

	inline bool operator!=(const IndexBuffer& rt) const
	{
		return (rt.m_buffer != m_buffer);
	}

private:
	ID3D10Buffer* m_buffer;
	size_t m_indexCount;
	void* m_ownedBuffer;
	Format m_format;
	CPUAccess m_lockType;
};

class VertexBuffer
{
friend class Device;
public:
	enum CPUAccess
	{
		CpuNoAccess = 0,
		CpuWrite = D3D10_MAP_WRITE_DISCARD
	};

	struct Parameters
	{
		Parameters()
			: sourceBuffer(NULL)
			, access(CpuNoAccess)
		{
		}
		size_t vertexSize;
		size_t vertexCount;
		size_t stride;
		CPUAccess access;

		void* sourceBuffer;	// if this is NULL, the object creates its own buffer
	};

	VertexBuffer()
		: m_buffer(NULL)
		, vertexSize(0)
		, vertexCount(0)
		, vertexStride(0)
	{
	}

	inline bool IsValid() const
	{
		return m_buffer != NULL;
	}

	inline void Invalidate()
	{
		m_buffer = NULL;
		m_ownedBuffer = NULL;
	}

	template<class VertexType>
	static Parameters BuildVertexParams(int vertexCount, void* sourceData=NULL)
	{
		Parameters p;
		p.vertexSize = sizeof(VertexType);
		p.stride = sizeof(VertexType);
		p.vertexCount = vertexCount;
		p.sourceBuffer = sourceData;

		return p;
	}

	inline bool operator!=(const VertexBuffer& rt) const
	{
		return (rt.m_buffer != m_buffer);
	}
private:
	ID3D10Buffer* m_buffer;
	size_t vertexSize;
	size_t vertexCount;
	size_t vertexStride;
	void* m_ownedBuffer;
	CPUAccess m_lockType;
};

class Texture2D
{
friend class Device;
friend class TextureSampler;
public:
	enum TextureFormat
	{
		TypeFloat32 = DXGI_FORMAT_R32_FLOAT,
		TypeDepthStencil32 = DXGI_FORMAT_D32_FLOAT,
		TypeInt8UnNormalised = DXGI_FORMAT_R8G8B8A8_UNORM
	};

	enum TextureBindType
	{
		BindAsShaderResource=1,
		BindAsRenderTarget=2,
		BindAsDepthStencil=4
	};

	enum CPUAccess
	{
		CpuNoAccess = 0,
		CpuRead = D3D10_MAP_READ,
		CpuWrite = D3D10_MAP_WRITE,
		CpuReadWrite = D3D10_MAP_READ_WRITE | D3D10_MAP_READ
	};

	struct Parameters
	{
		Parameters()
			: stagingTexture(false)
		{

		}
		int width;
		int height;
		TextureFormat format;
		int msaaQuality;
		int msaaCount;
		int numMips;
		CPUAccess access;
		unsigned int bindFlags;
		bool stagingTexture;
	};

	Texture2D()
		: m_texture(NULL)
		, m_shaderResource(NULL)
	{
	}

	inline bool IsValid() const
	{
		return (m_texture != NULL);
	}

	inline void Invalidate()
	{
		m_texture = NULL;
		m_shaderResource = NULL;
	}

	inline bool operator!=(const Texture2D& rt) const
	{
		return (rt.m_texture != m_texture) || (rt.m_shaderResource != m_shaderResource);
	}

	inline const Parameters& GetParameters() const
	{
		return m_params;
	}

private:
	ID3D10Texture2D* m_texture;
	ID3D10ShaderResourceView* m_shaderResource;
	Parameters m_params;
};

class LockedTexture2D
{
friend class Device;
public:
	inline bool IsValid()
	{
		return m_locked != false && m_stagingTexture.IsValid();
	}

	inline void* GetBuffer()
	{
		return m_lockedBuffer;
	}

private:
	LockedTexture2D()
		: m_lockedBuffer(NULL)
		, m_locked(false)
	{
	}

	bool m_locked;
	Texture2D m_stagingTexture;	// a copy of the texture, used for reading and writing
	Texture2D m_sourceTexture;
	void* m_lockedBuffer;
};

class DepthStencilBuffer
{
friend class Device;
public:
	struct Parameters
	{
		int m_width;
		int m_height;
		int m_msaaQuality;
		int m_msaaCount;
		int m_format;
	};

	DepthStencilBuffer()
		: m_renderTarget(NULL)
	{
	}

	inline bool IsValid() const
	{
		return (m_renderTarget != NULL) && m_surface.IsValid();
	}

	inline void Invalidate()
	{
		m_renderTarget = NULL;
		m_surface.Invalidate();
	}

	inline bool operator!=(const DepthStencilBuffer& rt) const
	{
		return rt.m_renderTarget != m_renderTarget;
	}

private:

	Texture2D m_surface;
	ID3D10DepthStencilView* m_renderTarget;
	Parameters m_params;
};

class Rendertarget
{
friend class Device;
public:
	Rendertarget()
		: m_rendertarget(NULL)
	{
	}

	struct Parameters
	{
		Texture2D target;
	};

	inline bool IsValid() const
	{
		return (m_rendertarget != NULL);
	}

	inline void Invalidate()
	{
		m_rendertarget = NULL;
	}

	inline bool operator!=(const Rendertarget& rt) const
	{
		return rt.m_rendertarget != m_rendertarget;
	}

private:
	ID3D10RenderTargetView* m_rendertarget;
};

class Viewport
{
public:
	Vector2 dimensions;
	Vector2 topLeft;
	Vector2f depthRange;

	inline bool operator!=(const Viewport& rt) const
	{
		return rt.dimensions != dimensions || topLeft != rt.topLeft || depthRange != rt.depthRange;
	}
};

#endif