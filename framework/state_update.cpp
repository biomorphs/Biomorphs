#include "framework/state_update.h"
#include "framework/input.h"

StateUpdateModule::StateUpdateModule( void* userData )
{
}

StateUpdateModule::~StateUpdateModule()
{
}

bool StateUpdateModule::startup()
{
	return true;
}

bool StateUpdateModule::shutdown()
{
	return true;
}

bool StateUpdateModule::connect( IModuleConnector& connector )
{
	m_inputModule = (InputModule*)connector.getModule( "Input" );

	return m_inputModule != NULL ? true : false;
}

bool StateUpdateModule::update( Timer& timer )
{
	float timeEscapePressed = 0.0f;
	if( m_inputModule && m_inputModule->keyPressed( VK_ESCAPE, timeEscapePressed ) )
	{
		if( timeEscapePressed > 1.0f )
		{
			return false;
		}
	}

	return true;
}