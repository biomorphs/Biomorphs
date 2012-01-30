#include "device.h"
#include "core\strings.h"

HWND Device::s_appWindow = 0;

Device::Device()
	: m_driverType(D3D10_DRIVER_TYPE_NULL)
	, m_d3dDevice(NULL)
	, m_swapChain(NULL)
{
}

Device::~Device()
{
}

void Device::SetTechnique(EffectTechnique& t, int pass)
{
	ID3D10EffectTechnique* technique = t.m_technique;
	technique->GetPassByIndex( pass )->Apply( 0 );
}

void Device::DrawIndexed(DrawIndexedParameters& params)
{
	m_d3dDevice->DrawIndexed( params.m_indexCount, params.m_startIndex, 0 );
}

void Device::SetInputLayout(ShaderInputLayout& l)
{
	// set the vertex shader input layout
	m_d3dDevice->IASetInputLayout( l.m_layout );
}

void Device::SetVertexBuffer(int streamIndex, VertexBuffer& vb)
{
	unsigned int vbOffset = 0;
	m_d3dDevice->IASetVertexBuffers( streamIndex, 1, &vb.m_buffer, &vb.vertexStride, &vbOffset );
}

void Device::SetIndexBuffer(IndexBuffer& ib)
{
	m_d3dDevice->IASetIndexBuffer( ib.m_buffer, (DXGI_FORMAT)ib.m_format, 0 );
}

void Device::SetPrimitiveTopology(PrimitiveTopology t)
{
	m_d3dDevice->IASetPrimitiveTopology( (D3D_PRIMITIVE_TOPOLOGY)t );
}

void Device::SetViewport( Viewport& vp )
{
	// Setup the viewport
    D3D10_VIEWPORT d3dvp;
	d3dvp.Width = vp.dimensions.x();
	d3dvp.Height = vp.dimensions.y();
	d3dvp.MinDepth = vp.depthRange.x();
	d3dvp.MaxDepth = vp.depthRange.y();
	d3dvp.TopLeftX = vp.topLeft.x();
	d3dvp.TopLeftY = vp.topLeft.y();
    m_d3dDevice->RSSetViewports( 1, &d3dvp );
}

bool Device::ClearTarget( const Rendertarget& rt, float clearColour[4] )
{
	if( rt.IsValid() )
	{
		m_d3dDevice->ClearRenderTargetView( rt.m_rendertarget, clearColour );
		return true;
	}

	return false;
}

bool Device::ClearTarget( const DepthStencilBuffer& rt, float depth, unsigned int stencil )
{
	if( rt.IsValid() )
	{
		m_d3dDevice->ClearDepthStencilView( rt.m_renderTarget, D3D10_CLEAR_DEPTH, depth, stencil );
		return true;
	}

	return false;
}

void Device::SetRenderTargets( Rendertarget* colourTarget, DepthStencilBuffer* depthStencilTarget )
{
	ID3D10RenderTargetView* c = colourTarget!=NULL ? colourTarget->m_rendertarget : NULL;
	ID3D10DepthStencilView* d = depthStencilTarget!=NULL ? depthStencilTarget->m_renderTarget : NULL;

	m_d3dDevice->OMSetRenderTargets(1, &c, d);
}

bool Device::SaveTextureToFile(Texture2D& t, const char* fileName, TextureFileType type)
{
	HRESULT hr = D3DX10SaveTextureToFileA( t.m_texture, (D3DX10_IMAGE_FILE_FORMAT)type, fileName );

	return !FAILED(hr);
}

void* Device::LockIB(IndexBuffer& ib)
{
	void* buffer = NULL;
	if( ib.IsValid() )
	{
		HRESULT hr = ib.m_buffer->Map( (D3D10_MAP)ib.m_lockType, 0, &buffer );
		if(FAILED(hr))
		{
			return NULL;
		}
	}

	return buffer;
}

void Device::UnlockIB(IndexBuffer& ib)
{
	if( ib.IsValid() )
	{
		ib.m_buffer->Unmap();	
	}
}

void* Device::LockVB(VertexBuffer& vb)
{
	void* buffer = NULL;
	if( vb.IsValid() )
	{
		HRESULT hr = vb.m_buffer->Map( (D3D10_MAP)vb.m_lockType, 0, &buffer );
		if(FAILED(hr))
		{
			return NULL;
		}
	}

	return buffer;
}

void Device::UnlockVB(VertexBuffer& vb)
{
	if( vb.IsValid() )
	{
		vb.m_buffer->Unmap();	
	}
}

void Device::PresentBackbuffer()
{
	m_swapChain->Present( 0, 0 );
}

void Device::ResetShaderState()
{
	ID3D10ShaderResourceView *const pSRV[1] = {NULL};
	m_d3dDevice->PSSetShaderResources( 0, 1, pSRV );
}

void Device::Flush()
{
	if( m_d3dDevice ) m_d3dDevice->ClearState();
}

bool Device::Shutdown()
{
	Flush();

	Release(m_mainDepthStencil);
	Release(m_backBufferRT);

    if( m_swapChain ) m_swapChain->Release();
    if( m_d3dDevice ) m_d3dDevice->Release();

	m_swapChain = NULL;
	m_d3dDevice = NULL;

	return true;
}

Rendertarget& Device::GetBackBuffer()
{
	return m_backBufferRT;
}

DepthStencilBuffer& Device::GetDepthStencilBuffer()
{
	return m_mainDepthStencil;
}

bool Device::Initialise(InitParameters& params)
{
	if(!initDriver(params))
	{
		Shutdown();
		return false;
	}

	if(!createRenderBuffers(params))
	{
		Shutdown();
		return false;
	}

	m_params = params;

	return true;
}

bool Device::createRenderBuffers(InitParameters& params)
{
	// Get the back buffer 
    ID3D10Texture2D* pBuffer;
    HRESULT hr = m_swapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer );
    if( FAILED( hr ) )
        return false;

	// Wrap it in our texture
	Texture2D backBufferSurface;
	backBufferSurface.m_texture = pBuffer;

	// Create the target
	Rendertarget::Parameters rtParams;
	rtParams.target = backBufferSurface;
	m_backBufferRT = CreateRendertarget( rtParams );

	// Release the texture buffer
	Release(backBufferSurface);

	// Create the default depth-stencil buffer
	DepthStencilBuffer::Parameters depthParams;
	depthParams.m_width = params.windowWidth;
	depthParams.m_height = params.windowHeight;
	depthParams.m_msaaQuality = params.msaaQuality;
	depthParams.m_msaaCount = params.msaaCount;
	depthParams.m_format = DepthStencilBuffer::TypeDepthStencil32;
	m_mainDepthStencil = CreateDepthStencil(depthParams);

	return true;
}

Texture2D Device::GetBackBufferTexture()
{
	Texture2D result;

	// get the render target surface
	ID3D10Texture2D* pBuffer;
    HRESULT hr = m_swapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer );
    if( FAILED( hr ) )
        return result;

	// Wrap it in our texture
	result.m_texture = pBuffer;
	result.m_params.access = Texture2D::CpuNoAccess;
	result.m_params.bindFlags = 0;
	result.m_params.format = Texture2D::TypeInt8UnNormalised;
	result.m_params.height = m_params.windowHeight;
	result.m_params.msaaCount = m_params.msaaCount;
	result.m_params.msaaQuality = m_params.msaaQuality;
	result.m_params.numMips = 1;
	result.m_params.width = m_params.windowWidth;

	return result;
}

bool Device::initDriver(InitParameters& params)
{
	HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( s_appWindow, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;

	if( params.enableDebugD3d )
	{
		createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
	}

    D3D10_DRIVER_TYPE driverTypes[] =
    {
        D3D10_DRIVER_TYPE_HARDWARE,
        D3D10_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
	sd.BufferDesc.Width = params.windowWidth;
	sd.BufferDesc.Height = params.windowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = s_appWindow;
	sd.SampleDesc.Count = params.msaaCount;
	sd.SampleDesc.Quality = params.msaaQuality;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D10CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags,
                                            D3D10_SDK_VERSION, &sd, &m_swapChain, &m_d3dDevice );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return false;

	return true;
}

Rendertarget Device::CreateRendertarget( const Rendertarget::Parameters params )
{
	// Aim the main target at it
	Rendertarget rt;
	ID3D10RenderTargetView* rtView = NULL;
	if( params.target.IsValid() )
	{
		HRESULT hr = m_d3dDevice->CreateRenderTargetView( params.target.m_texture, NULL, &rtView );
    
		if( !FAILED( hr ) )
		{
			rt.m_rendertarget = rtView;
		}
	}
	return rt;
}

DepthStencilBuffer Device::CreateDepthStencil( const DepthStencilBuffer::Parameters& params )
{
	DepthStencilBuffer resultBuffer;
	
	// Create the surface
	Texture2D::Parameters surfaceParams;
	surfaceParams.format = (Texture2D::TextureFormat)params.m_format;
	surfaceParams.width = params.m_width;
	surfaceParams.height = params.m_height;
	surfaceParams.msaaCount = params.m_msaaCount;
	surfaceParams.msaaQuality = params.m_msaaQuality;
	surfaceParams.bindFlags = Texture2D::BindAsDepthStencil;
	surfaceParams.numMips = 1;
	surfaceParams.access = Texture2D::CpuNoAccess;
	resultBuffer.m_surface = CreateTexture(surfaceParams);

	// Create the depth stencil target
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = (DXGI_FORMAT)params.m_format;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
	ID3D10DepthStencilView* rt = NULL;
	HRESULT hr = m_d3dDevice->CreateDepthStencilView( resultBuffer.m_surface.m_texture, &descDSV, &rt);
    if( !FAILED( hr ) )
	{
		resultBuffer.m_renderTarget= rt;
		resultBuffer.m_params = params;
	}

	return resultBuffer;
}

Texture2D Device::LoadTextureFromFile( const char* fileName )
{
	Texture2D resultTexture;

	std::wstring wideName = Strings::StringToWide(fileName);

	ID3D10ShaderResourceView* rview = NULL;
	HRESULT hr = D3DX10CreateShaderResourceViewFromFile( m_d3dDevice, wideName.c_str(), NULL, NULL, &rview, NULL );
    if( !FAILED( hr ) )
	{
		resultTexture.m_shaderResource = rview;
		rview->GetResource( (ID3D10Resource**)&resultTexture.m_texture );
	}

	return resultTexture;
}

void Device::CopyTextureToTexture(Texture2D& src, Texture2D& dst)
{
	if( src.IsValid() && dst.IsValid() )
	{
		m_d3dDevice->CopyResource(  dst.m_texture, src.m_texture );
	}
}

LockedTexture2D Device::LockTexture(Texture2D& t, Texture2D::CPUAccess bindParams)
{
	LockedTexture2D result;

	// First of all, create a 'staging' texture; a copy we use for read/write access
	Texture2D::Parameters stagingParams;
	stagingParams = t.m_params;
	stagingParams.stagingTexture = true;
	stagingParams.access = bindParams;
	Texture2D stagingTexture = CreateTexture( stagingParams );
	if( stagingTexture.IsValid() )
	{
		result.m_stagingTexture = stagingTexture;
		result.m_sourceTexture = t;

		// Now, copy the source texture to the staging texture using the GPU
		m_d3dDevice->CopyResource( result.m_stagingTexture.m_texture, result.m_sourceTexture.m_texture );

		// Finally, the staging texture is mapped
		D3D10_MAPPED_TEXTURE2D mappedTexture;
		int subResource = D3D10CalcSubresource(0,0,1);	// just map first mip level for now

		HRESULT hr = result.m_stagingTexture.m_texture->Map( subResource, (D3D10_MAP)bindParams, 0, &mappedTexture );
		if(!FAILED(hr))
		{
			result.m_locked = true;
			result.m_lockedBuffer = mappedTexture.pData;
		}
	}

	return result;
}

void Device::UnlockTexture(LockedTexture2D& t)
{
	if( t.m_locked )
	{
		unsigned int subResourceId = D3D10CalcSubresource(0,0,1);
		t.m_stagingTexture.m_texture->Unmap(subResourceId);
		t.m_lockedBuffer = NULL;
		t.m_locked= false;

		// Now, copy the staging texture back to the souyrce texture using the GPU
		m_d3dDevice->CopyResource( t.m_sourceTexture.m_texture, t.m_stagingTexture.m_texture );
	}	

	// release the staging texture
	if( t.m_stagingTexture.IsValid() )
	{
		Release( t.m_stagingTexture );
	}
}

Texture2D Device::CreateTexture( Texture2D::Parameters params )
{
	Texture2D resultTexture;

	// Create the texture
	D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = params.width;
    descDepth.Height = params.height;
	descDepth.MipLevels = params.msaaCount>1 ? 1 : params.numMips;
    descDepth.ArraySize = 1;
    descDepth.Format = (DXGI_FORMAT)params.format;
	descDepth.SampleDesc.Count = params.msaaCount;
	descDepth.SampleDesc.Quality = params.msaaQuality;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = 0;
	descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

	if( params.bindFlags & Texture2D::BindAsShaderResource )
	{
		descDepth.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	}
	if( params.bindFlags & Texture2D::BindAsRenderTarget )
	{
		descDepth.BindFlags |= D3D10_BIND_RENDER_TARGET;
	}
	if( params.bindFlags & Texture2D::BindAsDepthStencil )
	{
		descDepth.BindFlags |= D3D10_BIND_DEPTH_STENCIL;
	}

	if( params.access == Texture2D::CpuRead )
	{
		descDepth.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
	}
	if( params.access == Texture2D::CpuWrite )
	{
		descDepth.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		descDepth.Usage = D3D10_USAGE_DYNAMIC;
	}
	else if( params.access == Texture2D::CpuReadWrite )
	{
		descDepth.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
		descDepth.Usage = D3D10_USAGE_DYNAMIC;
	}

	if( params.stagingTexture )
	{
		// Staging textures are not bound to the device at all
		descDepth.BindFlags = 0;
		descDepth.Usage = D3D10_USAGE_STAGING;
	}

	ID3D10Texture2D* surface = NULL;
	HRESULT hr = m_d3dDevice->CreateTexture2D( &descDepth, NULL, &surface );
    if( !FAILED( hr ) )
	{
		resultTexture.m_texture = surface;
		resultTexture.m_params = params;
	}

	// create a shader resource, if required
	if( descDepth.BindFlags & D3D10_BIND_SHADER_RESOURCE )
	{
		hr = m_d3dDevice->CreateShaderResourceView( surface, NULL, &resultTexture.m_shaderResource );
	}

	return resultTexture;
}

VertexBuffer Device::CreateVB( VertexBuffer::Parameters params )
{
	VertexBuffer result;

	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = params.stride * params.vertexCount;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

	if( params.access != VertexBuffer::CpuNoAccess )
	{
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		bd.Usage = D3D10_USAGE_DYNAMIC;
	}
	
	void* sourceBuffer = NULL;
	if(!params.sourceBuffer)
	{
		sourceBuffer = new unsigned char[params.vertexCount * params.stride];
		result.m_ownedBuffer = sourceBuffer;
	}
	else 
	{
		sourceBuffer = params.sourceBuffer;
		result.m_ownedBuffer = NULL;
	}

    D3D10_SUBRESOURCE_DATA InitData;
	ID3D10Buffer* vb=NULL;
	InitData.pSysMem = sourceBuffer;
    HRESULT hr = m_d3dDevice->CreateBuffer( &bd, &InitData, &vb );
    if( !FAILED( hr ) )
	{
		result.m_buffer = vb;
		result.vertexCount = params.vertexCount;
		result.vertexSize = params.vertexSize;
		result.vertexStride = params.stride;
		result.m_lockType = params.access;
	}
	return result;
}

IndexBuffer Device::CreateIB( IndexBuffer::Parameters params )
{
	IndexBuffer result;

	size_t indexSize = params.format == IndexBuffer::IB_16BIT ? sizeof(unsigned short) : sizeof(unsigned int);

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = indexSize * params.indexCount;
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

	if( params.access != VertexBuffer::CpuNoAccess )
	{
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		bd.Usage = D3D10_USAGE_DYNAMIC;
	}

	void* sourceBuffer = NULL;
	if(!params.sourceBuffer)
	{
		sourceBuffer = new unsigned char[params.indexCount * indexSize];
		result.m_ownedBuffer = sourceBuffer;
	}
	else 
	{
		sourceBuffer = params.sourceBuffer;
		result.m_ownedBuffer = NULL;
	}

	D3D10_SUBRESOURCE_DATA InitData;
	ID3D10Buffer* ib = NULL;
    InitData.pSysMem = sourceBuffer;
    HRESULT hr = m_d3dDevice->CreateBuffer( &bd, &InitData, &ib );
    if( !FAILED( hr ) )
	{
		result.m_buffer = ib;
		result.m_indexCount = params.indexCount;
		result.m_format = params.format;
		result.m_lockType = params.access;
	}

	return result;
}

Effect Device::CreateEffect(Effect::Parameters params)
{
	Effect result;

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    dwShaderFlags |= D3D10_SHADER_DEBUG;

	ID3D10Blob* compileErrors = NULL;

	ID3D10Effect* e = NULL;
	std::wstring wideName = Strings::StringToWide(params.effectName);
	HRESULT hr = D3DX10CreateEffectFromFile( wideName.c_str(), NULL, NULL, "fx_4_0", dwShaderFlags, 0, m_d3dDevice, NULL,
											 NULL, &e, &compileErrors, NULL );
    if( !FAILED( hr ) )
    {
		result.m_effect = e;
    }
	else
	{
		void* errors = compileErrors->GetBufferPointer();
		char* errorTxt = (char*)errors;
		int numChars = compileErrors->GetBufferSize();
		printf_s("A shader failed to compile:\n%s\n", errorTxt);
	}

	return result;
}

ShaderInputLayout Device::CreateVertexInputLayout( Effect& effect, VertexDescriptor& vd )
{
	ShaderInputLayout result;

	if( effect.IsValid() )
	{
		// First, create a D3D model of the vertex descriptor in the effect
		// Define the input layout
		D3D10_INPUT_ELEMENT_DESC *layout = NULL;
		layout = new D3D10_INPUT_ELEMENT_DESC[ vd.m_elements.size() ];

		int i=0;
		for(VertexDescriptor::ElementIterator it=vd.m_elements.begin();
			it != vd.m_elements.end();
			++it)
		{
			layout[i].AlignedByteOffset = it->byteOffset;
			layout[i].Format = (DXGI_FORMAT)it->format;
			layout[i].InputSlot = it->streamIndex;
			layout[i].InputSlotClass = (D3D10_INPUT_CLASSIFICATION)it->elementType;
			layout[i].InstanceDataStepRate = it->instanceDrawStep;
			layout[i].SemanticIndex = it->semanticIndex;
			layout[i].SemanticName = it->semanticName;
			++i;
		}

		// Obtain the first technique
		ID3D10EffectTechnique* technique = effect.m_effect->GetTechniqueByIndex(0);

		// Create the input layout
		D3D10_PASS_DESC PassDesc;
		ID3D10InputLayout* inputLayout=NULL;
		technique->GetPassByIndex( 0 )->GetDesc( &PassDesc );	// Bind to the first pass
		HRESULT hr = m_d3dDevice->CreateInputLayout( layout, vd.m_elements.size(), PassDesc.pIAInputSignature,
														PassDesc.IAInputSignatureSize, &inputLayout );
		if(!FAILED(hr))
		{
			result.m_layout = inputLayout;
		}
		delete [] layout;
	}

	return result;
}

void Device::DrawText(const char* text, Font& f, Font::DrawParameters& p, Vector2 position, Vector2 rectSize)
{
	RECT fontRect = {position.x(), position.y(), rectSize.x(), rectSize.y()};

	// Save the render state so this can be safely called any time
    FLOAT OriginalBlendFactor[4];
    UINT OriginalSampleMask = 0;
    ID3D10BlendState* OriginalBlendState = NULL;
    ID3D10DepthStencilState* OriginalDepthStencil = NULL;
    UINT OriginalStencilRef;
    m_d3dDevice->OMGetBlendState( &OriginalBlendState, OriginalBlendFactor, &OriginalSampleMask );
    m_d3dDevice->OMGetDepthStencilState( &OriginalDepthStencil, &OriginalStencilRef );

	if( rectSize == Vector2(0,0) )
	{
		// Calculate the rect if needed
		f.mFont->DrawTextA(f.mSprites, text, -1, &fontRect, DT_CALCRECT, p.mColour);
	}

	// Render using the sprites
	f.mSprites->Begin(D3DX10_SPRITE_SORT_TEXTURE | D3DX10_SPRITE_SAVE_STATE);
	f.mFont->DrawTextA(f.mSprites, text, -1, &fontRect, p.mJustification | DT_WORDBREAK, p.mColour);
	f.mSprites->End();

	// Restore state
    m_d3dDevice->OMSetBlendState( OriginalBlendState, OriginalBlendFactor, OriginalSampleMask );
    m_d3dDevice->OMSetDepthStencilState( OriginalDepthStencil, OriginalStencilRef );
}

Font Device::CreateFont(Font::Parameters params)
{
	Font result;

	D3DX10_FONT_DESC fontDesc;
	fontDesc.Height = params.mSize;
	fontDesc.Width = 0;
	fontDesc.Weight = params.mWeight;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = params.mFlags & Font::ITALIC;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	// Convert font name to wide
	std::string fontName = params.mTypeface;
	std::wstring wideName = Strings::StringToWide( fontName );
	wcscpy_s( fontDesc.FaceName, wideName.c_str() );

	ID3DX10Font* font=NULL;
	D3DX10CreateFontIndirect(m_d3dDevice, &fontDesc, &font);

	// Use d3d sprite set to speed up render
	ID3DX10Sprite* sprite=NULL;
	if(font && !FAILED(D3DX10CreateSprite(m_d3dDevice, 512, &sprite)))
    {
		result.mParams = params;
		result.mFont = font;
		result.mSprites = sprite;
    }

	return result;
}

void Device::Release(Font &f)
{
	if( f.mSprites ) f.mSprites->Release();
	if( f.mFont ) f.mFont->Release();

	f.Invalidate();
}

void Device::Release(ShaderInputLayout& l)
{
	if( l.m_layout ) l.m_layout->Release();
	l.Invalidate();
}

void Device::Release(Effect& e)
{
	if( e.m_effect ) e.m_effect->Release();
	e.Invalidate();
}

void Device::Release( Texture2D& t )
{
	if( t.m_texture ) t.m_texture->Release();
	if( t.m_shaderResource ) t.m_shaderResource->Release();
	t.Invalidate();
}

void Device::Release(IndexBuffer& ib)
{
	if( ib.m_buffer ) ib.m_buffer->Release();
	if( ib.m_ownedBuffer ) delete [] ib.m_ownedBuffer;
	ib.Invalidate();
}

void Device::Release(VertexBuffer& vb)
{
	if( vb.m_buffer ) vb.m_buffer->Release();
	if( vb.m_ownedBuffer ) delete [] vb.m_ownedBuffer;
	vb.Invalidate();
}

void Device::Release( DepthStencilBuffer& d )
{
	if( d.m_renderTarget ) d.m_renderTarget->Release();
	if( d.m_surface.IsValid() ) Release( d.m_surface );
	d.Invalidate();
}

void Device::Release( Rendertarget& r )
{
	if( r.m_rendertarget ) r.m_rendertarget->Release();
	r.Invalidate();
}

