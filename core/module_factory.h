#ifndef MODULE_FACTORY_INCLUDED
#define MODULE_FACTORY_INCLUDED

#include "core/module.h"

class ModuleFactory
{
public:
	ModuleFactory()
	{
	};
	virtual Module* createModule() {}
};

template<class ModuleType>
class ModuleFactoryGeneric
{
public:
	ModuleFactoryGeneric( void* userData=NULL )
		: m_userData(userData)
	{
	}

	virtual Module* createModule()
	{
		return new ModuleType((void*)m_userData);
	}

private:
	void* m_userData;
};

#endif