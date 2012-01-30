#include "app.h"

App::App(D3DAppConfig& config)
	: D3DApp(config)
	, m_inputModuleFactory((void*)&config)
	, m_moduleFactory((void*)&config)
{
}

App::~App()
{
}

bool App::registerModuleFactories()
{
	if(!m_moduleManager.registerFactory("InputModule", (ModuleFactory*)&m_inputModuleFactory))
	{
		return false;
	}

	if(!m_moduleManager.registerFactory("Biomorphs", (ModuleFactory*)&m_moduleFactory))
	{
		return false;
	}

	return D3DApp::registerModuleFactories();
}