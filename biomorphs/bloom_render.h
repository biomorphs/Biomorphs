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

	void Create(Device* d, Parameters& p);
	void Release();
	void Render();

private:

	struct BloomRT
	{
		Texture2D mTexture;
		Rendertarget mRT;
		DepthStencilBuffer mDSB;
	};

	void CombineTargets( BloomRT& fullTarget, BloomRT& tinyBlur );
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
	BloomRT m_tinyBlur;
};

#endif