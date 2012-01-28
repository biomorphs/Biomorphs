#ifndef MODULE_MANAGER
#define MODULE_MANAGER

#include <map>
#include "core/string_hashing.h"
#include "core/module.h"
#include "core/module_factory.h"

class ModuleManager : public IModuleConnector
{
public:
	ModuleManager();
	~ModuleManager();

	bool appendFromFile( std::string filename );

	bool registerFactory( std::string moduleTypeName, ModuleFactory* factory );
	bool addModule( std::string moduleTypeName, std::string moduleName );

	bool connect();
	bool postConnect();
	bool startup();

	bool update( Timer& timer );
	bool shutdown();

	virtual Module* getModule( std::string moduleName );

protected:
	struct ModuleDescriptor
	{
		ModuleDescriptor( std::string name, Module* module )
			: Name(name), Module(module)
		{
		}
		std::string Name;
		Module* Module;
	};

	typedef std::pair< StringHashing::StringHash, ModuleDescriptor > ModulePair;
	typedef std::map< StringHashing::StringHash, ModuleDescriptor > ModuleMap;
	ModuleMap m_modules;

private:
	typedef std::pair<StringHashing::StringHash, ModuleFactory*> FactoryPair;
	typedef std::map<StringHashing::StringHash, ModuleFactory*> FactoryMap;
	static FactoryMap s_factories;
};

#endif