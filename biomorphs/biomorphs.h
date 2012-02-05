#ifndef BIOMORPHS_MODULE_INCLUDED
#define BIOMORPHS_MODULE_INCLUDED

#include "biomorph_manager.h"
#include "framework\graphics\d3d_app.h"
#include "framework\graphics\device.h"
#include "framework\graphics\screenshot_helper.h"
#include "framework\graphics\sprite_render.h"
#include "framework\input.h"
#include "bloom_render.h"

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
	void _drawOverlay();
	void _drawMorphToScreen();

	bool _update(Timer& timer);
	void _render(Timer& timer);
	bool _initialise();

	bool initDevice();

	MorphDNA m_testDNA;
	int m_generation;

	BiomorphInstance mMorphInstance;
	BiomorphManager mBiomorphManager;	

	BloomRender m_bloom;

	Font m_font;
	Effect m_spriteShader;
	SpriteRender m_spriteRender;

	InputModule* m_inputModule;
	D3DAppConfig m_appConfig;
	Device m_device;
};

#endif