#ifndef BLOOM_RENDER_H_INCLUDED
#define BLOOM_RENDER_H_INCLUDED

#include "framework\graphics\device_types.h"
#include "framework\graphics\sprite_render.h"
#include "core\containers.h"

class BloomRender
{
public:
	struct Parameters
	{
		int mWidth;
		int mHeight;
	};

	struct DrawParameters
	{
		DrawParameters(float extraTinyThres=0.0f, float extraTinyMul = 0.0f,
					   float tinyThresh=0.0f, float tinyMul=0.0f,
					   float quarterThresh=0.0f, float quarterMul=0.0f,
					   float halfThresh=0.0f, float halfMul=0.0f)
					   : TinyBlurConsts(tinyThresh, tinyMul,0.0f,0.0f)
					   , QuarterBlurConsts(quarterThresh, quarterMul,0.0f,0.0f)
					   , HalfBlurConsts(halfThresh, halfMul,0.0f,0.0f)
					   , ExtraTinyBlurConsts(extraTinyThres, extraTinyMul, 0.0f,0.0f)
		{
		}

		D3DXVECTOR4 ExtraTinyBlurConsts;
		D3DXVECTOR4 TinyBlurConsts;
		D3DXVECTOR4 QuarterBlurConsts;
		D3DXVECTOR4 HalfBlurConsts;
	};

	void Create(Device* d, Parameters& p);
	void Release();
	void Render( const DrawParameters& p );

private:

	struct BloomRT
	{
		Texture2D mTexture;
		Rendertarget mRT;
		DepthStencilBuffer mDSB;
	};

	void DebugTarget( BloomRT& source );
	void CombineTargets( const DrawParameters& p );
	void RenderTargetToTarget( BloomRT& src, BloomRT& dst, const char* technique );

	Texture2D CreateRTTexture(int width, int height, Texture2D::TextureFormat format);
	DepthStencilBuffer CreateRTDepthStencil(int width, int height);
	BloomRT CreateRenderTarget(int width, int height, Texture2D::TextureFormat format);
	void Release( BloomRT& rt );

	Parameters m_params;

	SpriteRender m_spriteRender;
	Device* m_device;

	Effect m_effect;	// bloom shader

	BloomRT m_fullscreen;	// original FS copy
	BloomRT m_halfRes;		// half res (quarter size) f32 target
	BloomRT m_quarterRes;	// quarter res (1/16 size) f32 target
	BloomRT m_tiny;			// 1 / 16 res f32 target
	BloomRT m_extraTiny;

	// working textures
	BloomRT m_extraTinyBlur;
	BloomRT m_tinyBlur;
	BloomRT m_quarterBlur;
	BloomRT m_halfBlur;;
};

#endif