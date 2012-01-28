#ifndef D3D_APP_INCLUDED
#define D3D_APP_INCLUDED

#include "core/module_manager.h"
#include "core/timer.h"
#include "core/vector2.h"

class D3DApp;

class D3DAppConfig
{
public:
	D3DAppConfig(int w, int h, int bpp, bool fullscreen, std::string windowTitle, HINSTANCE hInst)
		: m_windowWidth(w)
		, m_windowHeight(h)
		, m_bpp(bpp)
		, m_fullscreen(fullscreen)
		, m_windowTitle(windowTitle)
		, m_app(NULL)
		, m_hInst(hInst)
	{
	}

	D3DAppConfig()
		: m_windowWidth(640)
		, m_windowHeight(480)
		, m_bpp(32)
		, m_fullscreen(false)
		, m_app(NULL)
	{
	}

	int m_windowWidth;
	int m_windowHeight;
	int m_bpp;
	bool m_fullscreen;
	std::string m_windowTitle;
	HINSTANCE m_hInst;
	D3DApp* m_app;
};

class D3DApp
{
public:
	D3DApp(D3DAppConfig config);
	virtual ~D3DApp();

	bool execute();
	
protected:
	virtual bool init();
	virtual bool cleanup();
	virtual bool registerModuleFactories();

	bool createWindow();

	D3DAppConfig m_config;

	bool m_quit;
	ModuleManager m_moduleManager;
	Timer m_appTimer;

	HINSTANCE m_hInst;
	HWND m_window;
};

#endif