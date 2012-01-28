#ifndef MODULE_INCLUDED
#define MODULE_INCLUDED

#include <string>
#include "core/timer.h"

class Module;

class IModuleConnector
{
public:
	virtual Module* getModule( std::string moduleName ) = 0;
};

class Module
{
public:
	Module();
	virtual ~Module();

	virtual bool connect( IModuleConnector& connector )		{ return true; }
	virtual bool postConnect()								{ return true; }
	virtual bool startup()									{ return true; }

	virtual bool shutdown() = 0;

	virtual bool update( Timer& timer ) = 0;
protected:
private:
};

#endif