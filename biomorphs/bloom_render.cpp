#include "bloom_render.h"
#include "framework\graphics\device.h"
#include "core\profiler.h"

void BloomRender::CombineTargets( const DrawParameters& p )
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

	// set the blur samplers
	EffectTechnique t = m_effect.GetTechniqueByName("Combine");

	TextureSampler tinyblur = t.GetSamplerByName("Tinyblur");
	tinyblur.Set( m_tiny.mTexture );

	TextureSampler extb = t.GetSamplerByName("ExtraTinyBlur");
	extb.Set( m_extraTiny.mTexture );

	TextureSampler quarterblur = t.GetSamplerByName("Quarterblur");
	quarterblur.Set( m_quarterRes.mTexture );

	TextureSampler halfblur = t.GetSamplerByName("Halfblur");
	halfblur.Set( m_halfRes.mTexture );

	// set the bloom constants
	VectorConstant ps = t.GetVectorConstant("TinyBloomConsts");
	ps.Set(p.TinyBlurConsts);	ps.Apply();

	ps = t.GetVectorConstant("ExtraTinyBloomConsts");
	ps.Set(p.ExtraTinyBlurConsts);	ps.Apply();

	ps = t.GetVectorConstant("QuarterBloomConsts");
	ps.Set(p.QuarterBlurConsts);	ps.Apply();

	ps = t.GetVectorConstant("HalfBloomConsts");
	ps.Set(p.HalfBlurConsts);	ps.Apply();

	const float scale = 1.0f;
	m_spriteRender.Draw( *m_device, D3DXVECTOR2(0.0f,0.0f), D3DXVECTOR2(scale,scale), "Combine" );

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
	ps.Set( pixelSize );
	ps.Apply();

	// render to the target using the sprite renderer
	m_spriteRender.GetTexture() = src.mTexture;	// use the source rt as a texture
	m_spriteRender.RemoveSprites();
	m_spriteRender.AddSprite( 0, D3DXVECTOR2(-1.0f,-1.0f), D3DXVECTOR2(2.0f,2.0f) );

	float scale = 1.0f;
	m_spriteRender.Draw( *m_device, D3DXVECTOR2(0.0f,0.0f), D3DXVECTOR2(scale,scale), technique );
}

void BloomRender::DebugTarget( BloomRT& source )
{
	m_device->ResetShaderState();	// flush currently bound rt
	m_device->SetRenderTargets( &m_device->GetBackBuffer(), &m_device->GetDepthStencilBuffer() );

	// Set the viewport
	Viewport vp;
	vp.topLeft = Vector2(0,0);
	vp.depthRange = Vector2f(0.0f,1.0f);
	vp.dimensions = Vector2(m_device->GetBackBufferTexture().GetParameters().width,
							m_device->GetBackBufferTexture().GetParameters().height);
	m_device->SetViewport( vp );

	// clear all colour to 0
	static float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_device->ClearTarget( m_device->GetBackBuffer(), clearColour );

	// Clear depth/stencil
	m_device->ClearTarget( m_device->GetDepthStencilBuffer(), 1.0f, 0 );

	// render to the target using the sprite renderer
	m_spriteRender.GetTexture() = source.mTexture;	// use the source rt as a texture
	m_spriteRender.RemoveSprites();
	m_spriteRender.AddSprite( 0, D3DXVECTOR2(-1.0f,-1.0f), D3DXVECTOR2(2.0f,2.0f) );

	float scale = 1.0f;
	m_spriteRender.Draw( *m_device, D3DXVECTOR2(0.0f,0.0f), D3DXVECTOR2(scale,scale), "Debug" );
}

void BloomRender::Render(const DrawParameters& p)
{
	{
		SCOPED_PROFILE(BloomCopyBackbuffer);
		Texture2D backBufferTexture = m_device->GetBackBufferTexture();
	
		// first grab the back buffer to our fullscreen texture
		m_device->CopyTextureToTexture(backBufferTexture, m_fullscreen.mTexture);
	}

	{
		SCOPED_PROFILE(BloomDownsample);

		// downsample the full res to half res
		RenderTargetToTarget(m_fullscreen, m_halfRes, "Downsample");

		// downsample again to 1/4 res
		RenderTargetToTarget(m_halfRes, m_quarterRes, "Downsample");

		// .. and again
		RenderTargetToTarget(m_quarterRes, m_tiny, "Downsample");

		// extra tiny
		RenderTargetToTarget( m_tiny, m_extraTiny, "Downsample");
	}

	{
		SCOPED_PROFILE(BloomBlurHalf);
		// 2 pass blur on half
		RenderTargetToTarget(m_halfRes, m_halfBlur, "BlurH");
		RenderTargetToTarget(m_halfBlur, m_halfRes, "BlurV");
	}

	{
		SCOPED_PROFILE(BloomBlurQuarter);
		// 2 pass blur on quarter
		RenderTargetToTarget(m_quarterRes, m_quarterBlur, "BlurH");
		RenderTargetToTarget(m_quarterBlur, m_quarterRes, "BlurV");
	}

	{

		SCOPED_PROFILE(BloomBlurTiny);
		// 2 pass blur on tiny
		RenderTargetToTarget(m_tiny, m_tinyBlur, "BlurH");
		RenderTargetToTarget(m_tinyBlur, m_tiny, "BlurV");
	}

	{

		SCOPED_PROFILE(BloomBlurExtraTiny);
		// 2 pass blur on tiny
		RenderTargetToTarget(m_extraTiny, m_extraTinyBlur, "BlurH");
		RenderTargetToTarget(m_extraTinyBlur, m_extraTiny, "BlurV");
	}

	// final combine back to back buffer
	CombineTargets( p );
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
	m_extraTiny = CreateRenderTarget( p.mWidth / 16, p.mHeight / 16, Texture2D::TypeFloat32 );
	
	m_halfBlur = CreateRenderTarget( p.mWidth / 2, p.mHeight / 2, Texture2D::TypeFloat32 );
	m_quarterBlur = CreateRenderTarget( p.mWidth / 4, p.mHeight / 4, Texture2D::TypeFloat32 );
	m_tinyBlur = CreateRenderTarget( p.mWidth / 8, p.mHeight / 8, Texture2D::TypeFloat32 );
	m_extraTinyBlur = CreateRenderTarget( p.mWidth / 16, p.mHeight / 16, Texture2D::TypeFloat32 );

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

	Release( m_extraTinyBlur );
	Release( m_tinyBlur );
	Release( m_quarterBlur );
	Release( m_halfBlur );

	Release( m_extraTiny );
	Release( m_tiny );
	Release( m_quarterRes );
	Release( m_halfRes );
	Release( m_fullscreen );

	m_device->Release( m_effect );
}