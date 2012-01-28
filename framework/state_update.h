#ifndef STATE_UPDATE_INCLUDED
#define STATE_UPDATE_INCLUDED

#include "core/module.h"

class InputModule;

class StateUpdateModule : public Module
{
public:
	StateUpdateModule( void* userData );
	virtual ~StateUpdateModule();

protected:

	virtual bool startup();
	virtual bool connect( IModuleConnector& connector );
	virtual bool shutdown();
	virtual bool update( Timer& timer );

	InputModule* m_inputModule;
};

#endif