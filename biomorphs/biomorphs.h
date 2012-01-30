#ifndef BIOMORPHS_MODULE_INCLUDED
#define BIOMORPHS_MODULE_INCLUDED

#include "morph_render.h"
#include "framework\graphics\d3d_app.h"
#include "framework\graphics\device.h"
#include "framework\graphics\shadowed_device.h"
#include "framework\graphics\screenshot_helper.h"
#include "framework\graphics\sprite_render.h"
#include "framework\input.h"

class Biomorphs : public Module
{
public:
	Biomorphs( void* userData );
	virtual ~Biomorphs();

	virtual bool startup();
	virtual bool connect( IModuleConnector& connector );
	virtual bool shutdown();
	virtual bool update( Timer& timer );

private:
	void _resetDNA();

	bool _update(Timer& timer);
	void _render();
	bool _initialise();

	bool initDevice();

	MorphDNA m_testDNA;
	int m_generation;
	bool m_doScreenshots;
	MorphRender m_morphRenderer;

	Font m_font;

	Effect m_spriteShader;

	InputModule* m_inputModule;
	D3DAppConfig m_appConfig;
	Device m_device;
	ShadowedDevice m_shadowDevice;
	ScreenshotHelper m_screenshotHelper;
	SpriteRender m_spriteRender;
};

#endif