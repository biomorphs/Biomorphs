#ifndef D3D_DEVICE_INCLUDED
#define D3D_DEVICE_INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <D3D10.h>
#include <D3DX10.h>

#include "device_types.h"

class Device;

class Device
{
public:
	Device();
	~Device();

	struct InitParameters
	{
		int windowWidth;
		int windowHeight;
		bool enableDebugD3d;
		int msaaQuality;
		int msaaCount;
	};

	// used for saving textures
	enum TextureFileType
	{
		TextureTypeBMP = D3DX10_IFF_BMP,
		TextureTypeJPG = D3DX10_IFF_JPG,
		TextureTypePNG = D3DX10_IFF_PNG,
		TextureTypeDDS = D3DX10_IFF_DDS
	};

	// Main Init/Shutdown
	bool Initialise(InitParameters& params);
	bool Shutdown();
	void Flush();	// Clear the device state

	// Backbuffer / Depthstencil access
	Rendertarget& GetBackBuffer();
	Texture2D GetBackBufferTexture();
	DepthStencilBuffer& GetDepthStencilBuffer();

	// Swap chain / VSync stuff
	void PresentBackbuffer();

	// Render State
	void SetViewport( Viewport& vp );
	void SetRenderTargets( Rendertarget& colourTarget, DepthStencilBuffer& depthStencilTarget );
	void SetPrimitiveTopology(PrimitiveTopology t);
	void SetInputLayout(ShaderInputLayout& l);
	void SetVertexBuffer(int streamIndex, VertexBuffer& vb);
	void SetIndexBuffer(IndexBuffer& ib);
	void SetTechnique(EffectTechnique& technique, int pass);

	// Draw/clear calls
	void DrawIndexed(DrawIndexedParameters& params); 
	bool ClearTarget( const DepthStencilBuffer& rt, float depth, unsigned int stencil );
	bool ClearTarget( const Rendertarget& rt, float clearColour[4] );

	// Rendertarget stuff
	Rendertarget CreateRendertarget( const Rendertarget::Parameters params );
	void Release( Rendertarget& r );

	// DepthStencil buffers
	DepthStencilBuffer CreateDepthStencil( const DepthStencilBuffer::Parameters& params );
	void Release( DepthStencilBuffer& d );

	// Texture read/write
	LockedTexture2D LockTexture(Texture2D& t, Texture2D::CPUAccess);
	void UnlockTexture(LockedTexture2D& t);
	void CopyTextureToTexture(Texture2D& src, Texture2D& dst);
	bool SaveTextureToFile(Texture2D& t, const char* fileName, TextureFileType type);

	// VB read/write
	void* LockVB(VertexBuffer& vb);
	void UnlockVB(VertexBuffer& vb);

	// IB read/write
	void* LockIB(IndexBuffer& ib);
	void UnlockIB(IndexBuffer& ib);

	// Texture stuff
	Texture2D CreateTexture( Texture2D::Parameters params );
	Texture2D LoadTextureFromFile( const char* fileName );
	void Release( Texture2D& t );

	// Vertex buffers
	VertexBuffer CreateVB( VertexBuffer::Parameters params );
	void Release(VertexBuffer& vb);

	// Index buffers
	IndexBuffer CreateIB( IndexBuffer::Parameters params );
	void Release(IndexBuffer& ib);

	// Vertex input layout
	ShaderInputLayout CreateVertexInputLayout( Effect& effect, VertexDescriptor& vd );
	void Release(ShaderInputLayout& l);

	// Shaders/Effects
	Effect CreateEffect(Effect::Parameters params);
	void Release(Effect& e);

	// Text Rendering
	Font CreateFont(Font::Parameters params);
	void Release(Font &f);
	void DrawText(const char* text, Font& f, Font::DrawParameters& p, Vector2 position, Vector2 rectSize = Vector2(0,0));

	static HWND s_appWindow;
private:

	bool initDriver(InitParameters& params);
	bool createRenderBuffers(InitParameters& params);

	D3D10_DRIVER_TYPE m_driverType;
	ID3D10Device*     m_d3dDevice;
	IDXGISwapChain*   m_swapChain;

	InitParameters m_params;
	Rendertarget m_backBufferRT;
	DepthStencilBuffer m_mainDepthStencil;
};

#endif