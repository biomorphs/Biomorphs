#include "bloom_render.h"
#include "framework\graphics\device.h"

void BloomRender::CombineTargets( BloomRT& fullTarget, BloomRT& tinyBlur )
{
	m_device->ResetShaderState();	// flush currently bound rt
	m_device->SetRenderTargets( &m_device->GetBackBuffer(), &m_device->GetDepthStencilBuffer() );

	// Set the viewport
	Viewport vp;
	vp.topLeft = Vector2(0,0);
	vp.depthRange = Vector2f(0.0f,1.0f);
	vp.dimensions = Vector2(m_params.mWidth, m_params.mHeight);
	m_device->SetViewport( vp );

	// clear all colour to 0
	static float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_device->ClearTarget( m_device->GetBackBuffer(), clearColour );

	// Clear depth/stencil
	m_device->ClearTarget(m_device->GetDepthStencilBuffer(), 1.0f, 0 );
	m_spriteRender.GetTexture() = m_fullscreen.mTexture;	// use the fullscreen rt as base texture

	// set the blur sampler
	EffectTechnique t = m_effect.GetTechniqueByName("Combine");
	TextureSampler tinyblur = t.GetSamplerByName("Tinyblur");
	tinyblur.Set( tinyBlur.mTexture );

	// set the bloom constants
	static float th = 0.08f;
	static float m = 3.0f;
	VectorConstant ps = t.GetVectorConstant("BloomConsts");
	D3DXVECTOR4 v = D3DXVECTOR4(th,m,0.0f,0.0f);
	ps.Set(v);
	ps.Apply();

	float aspect = 1.0f;//(float)m_params.mWidth / (float)m_params.mHeight;
	float scale = 1.0f;
	m_spriteRender.Draw( *m_device, D3DXVECTOR2(0.0f,0.0f), D3DXVECTOR2(scale,scale*aspect), "Combine" );

	m_device->ResetShaderState();
}

void BloomRender::RenderTargetToTarget( BloomRT& src, BloomRT& dst, const char* technique )
{
	m_device->ResetShaderState();	// flush currently bound rt
	m_device->SetRenderTargets( &dst.mRT, &dst.mDSB );

	// Set the viewport
	Viewport vp;
	vp.topLeft = Vector2(0,0);
	vp.depthRange = Vector2f(0.0f,1.0f);
	vp.dimensions = Vector2(dst.mTexture.GetParameters().width, dst.mTexture.GetParameters().height);
	m_device->SetViewport( vp );

	// clear all colour to 0
	static float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_device->ClearTarget( dst.mRT, clearColour );

	// Clear depth/stencil
	m_device->ClearTarget(dst.mDSB, 1.0f, 0 );

	// set the pixel size constant
	EffectTechnique t = m_effect.GetTechniqueByName(technique);
	VectorConstant ps = t.GetVectorConstant("PixelSize");
	D3DXVECTOR4 pixelSize = D3DXVECTOR4( 1.0f / (float)dst.mTexture.GetParameters().height,
										 1.0f / (float)dst.mTexture.GetParameters().width,
										 0.0f, 0.0f );
	pixelSize = pixelSize * 0.5f;
	ps.Set( pixelSize );
	ps.Apply();

	// set the bloom constants

	m_spriteRender.GetTexture() = src.mTexture;	// use the source rt as a texture
		 
	m_spriteRender.RemoveSprites();
	m_spriteRender.AddSprite( 0, D3DXVECTOR2(-1.0f,-1.0f), D3DXVECTOR2(2.0f,2.0f) );

	float aspect = (float)dst.mTexture.GetParameters().width / (float)dst.mTexture.GetParameters().width;
	float scale = 1.0f;
	m_spriteRender.Draw( *m_device, D3DXVECTOR2(0.0f,0.0f), D3DXVECTOR2(scale,scale*aspect), technique );
}

void BloomRender::Render()
{
	Texture2D backBufferTexture = m_device->GetBackBufferTexture();
	
	// first grab the back buffer to our fullscreen texture
	m_device->CopyTextureToTexture(backBufferTexture, m_fullscreen.mTexture);

	// downsample the full res to half res
	RenderTargetToTarget(m_fullscreen, m_halfRes, "Downsample");

	// downsample again to 1/4 res
	RenderTargetToTarget(m_halfRes, m_quarterRes, "Downsample");

	// .. and again
	RenderTargetToTarget(m_quarterRes, m_tiny, "Downsample");

	// 2 pass blur
	RenderTargetToTarget(m_tiny, m_tinyBlur, "BlurH");
	RenderTargetToTarget(m_tinyBlur, m_tiny, "BlurV");

	// final combine back to back buffer
	CombineTargets( m_fullscreen, m_tiny );
}

Texture2D BloomRender::CreateRTTexture(int width, int height, Texture2D::TextureFormat format)
{
	// create a texture to render to
	Texture2D::Parameters tp;
	tp.access = Texture2D::CpuNoAccess;
	tp.bindFlags = Texture2D::BindAsShaderResource | Texture2D::BindAsRenderTarget;
	tp.format = format;	// work at full precision
	tp.height = height;
	tp.width = width;
	tp.msaaCount = 1;
	tp.msaaQuality = 0;
	tp.numMips = 1;
	return m_device->CreateTexture( tp );
}

DepthStencilBuffer BloomRender::CreateRTDepthStencil(int width, int height)
{
	DepthStencilBuffer::Parameters dbp;
	dbp.m_format = DepthStencilBuffer::TypeDepthStencil32;
	dbp.m_width = width;
	dbp.m_height = height;
	dbp.m_msaaCount = 1;
	dbp.m_msaaQuality = 0;
	return m_device->CreateDepthStencil( dbp );
}

BloomRender::BloomRT BloomRender::CreateRenderTarget(int width, int height, Texture2D::TextureFormat format)
{
	BloomRT brt;
	brt.mTexture = CreateRTTexture( width, height, format );

	Rendertarget::Parameters rtp;
	rtp.target = brt.mTexture;
	brt.mRT = m_device->CreateRendertarget( rtp );

	brt.mDSB = CreateRTDepthStencil(width, height);

	return brt;
}

void BloomRender::Release( BloomRT& rt )
{
	m_device->Release( rt.mDSB );
	m_device->Release( rt.mRT );
	m_device->Release( rt.mTexture );
}

void BloomRender::Create(Device* d, Parameters& p)
{
	// create the first full screen texture
	m_device = d;
	m_params = p;

	// load shaders
	Effect::Parameters ep("shaders/bloom.fx");
	m_effect = m_device->CreateEffect(ep);

	// create rts
	m_fullscreen = CreateRenderTarget( p.mWidth, p.mHeight, Texture2D::TypeInt8UnNormalised );
	m_halfRes = CreateRenderTarget( p.mWidth / 2, p.mHeight / 2, Texture2D::TypeFloat32 );
	m_quarterRes = CreateRenderTarget( p.mWidth / 4, p.mHeight / 4, Texture2D::TypeFloat32 );
	m_tiny = CreateRenderTarget( p.mWidth / 8, p.mHeight / 8, Texture2D::TypeFloat32 );
	m_tinyBlur = CreateRenderTarget( p.mWidth / 8, p.mHeight / 8, Texture2D::TypeFloat32 );

	// create sprite renderer
	SpriteRender::Parameters sp;
	sp.mMaxSprites = 32;
	sp.shader = m_effect;
	sp.texture = m_fullscreen.mTexture;
	m_spriteRender.Create( *m_device, sp );
}

void BloomRender::Release()
{
	m_spriteRender.Release(*m_device);

	Release( m_tiny );
	Release( m_tinyBlur );
	Release( m_quarterRes );
	Release( m_halfRes );
	Release( m_fullscreen );

	m_device->Release( m_effect );
}