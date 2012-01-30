#ifndef APP_INCLUDED
#define APP_INCLUDED

#include "framework\graphics\d3d_app.h"
#include "biomorphs.h"

class App : public D3DApp
{
public:
	App(D3DAppConfig& config);
	virtual ~App();

private:
	virtual bool registerModuleFactories();

	ModuleFactoryGeneric<InputModule> m_inputModuleFactory;
	ModuleFactoryGeneric<Biomorphs> m_moduleFactory;
};

#endif