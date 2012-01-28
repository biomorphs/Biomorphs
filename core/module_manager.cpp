#include "core/module_manager.h"

#include "external\tinyxml\tinyxml.h"

const char c_moduleLabel[] = "Module";
const char c_nameLabel[] = "Name";
const char c_typeLabel[] = "Type";

ModuleManager::FactoryMap ModuleManager::s_factories;

ModuleManager::ModuleManager()
{
}

ModuleManager::~ModuleManager()
{
}

bool ModuleManager::appendFromFile( std::string filename )
{
	TiXmlDocument document( filename.c_str() );

	if( document.LoadFile() )
	{
		TiXmlElement* node = document.FirstChildElement();
		while( node )
		{
			if( strcmp( node->Value(), c_moduleLabel )==0 )
			{
				const char* name = node->Attribute( c_nameLabel );
				const char* type = node->Attribute( c_typeLabel );

				if( !addModule( type, name ) )
				{
					return false;
				}

				node = node->NextSiblingElement();
			}
			else
			{
				node = node->FirstChildElement();
			}
		}

		return true;
	}

	return false;
}

Module* ModuleManager::getModule( std::string moduleName )
{
	StringHashing::StringHash hash = StringHashing::getHash(moduleName.c_str());
	ModuleMap::iterator theModule = m_modules.find(hash);

	if( theModule != m_modules.end() )
	{
		return theModule->second.Module;
	}

	return NULL;
}

bool ModuleManager::postConnect()
{
	bool result=true;
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end() && result;
		++module )
	{
		if( module->second.Module )
		{
			result &= module->second.Module->postConnect();
		}
	}

	return result;
}

bool ModuleManager::connect()
{
	bool result=true;
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end() && result;
		++module )
	{
		if( module->second.Module )
		{
			result &= module->second.Module->connect( *this );
		}
	}

	return result;
}
	
bool ModuleManager::startup()
{
	bool result=true;
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end() && result;
		++module )
	{
		if( module->second.Module )
		{
			result &= module->second.Module->startup();
		}
	}

	return result;
}

bool ModuleManager::update( Timer& timer )
{
	bool result=true;
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end() && result;
		++module )
	{
		if( module->second.Module )
		{
			result &= module->second.Module->update( timer );
		}
	}

	return result;
}

bool ModuleManager::shutdown()
{
	bool result=true;
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end();
		++module )
	{
		if( module->second.Module )
		{
			result &= module->second.Module->shutdown();
		}
	}
	for( ModuleMap::iterator module = m_modules.begin();
		module != m_modules.end() && result;
		++module )
	{
		if( module->second.Module )
		{
			delete module->second.Module;
		}
	}

	return result;
}

bool ModuleManager::addModule( std::string moduleTypeName, std::string moduleName )
{
	StringHashing::StringHash hash = StringHashing::getHash( moduleTypeName.c_str() );

	FactoryMap::iterator foundFactory = s_factories.find(hash);
	if( foundFactory != s_factories.end() )
	{
		Module* newModule = foundFactory->second->createModule();
		if( newModule )
		{
			StringHashing::StringHash nameHash = StringHashing::getHash( moduleName.c_str() );
			m_modules.insert( ModulePair( nameHash, ModuleDescriptor( moduleName, newModule ) ) );

			return true;
		}
	}

	return false;
}

bool ModuleManager::registerFactory( std::string moduleTypeName, ModuleFactory* factory )
{
	StringHashing::StringHash hash = StringHashing::getHash( moduleTypeName.c_str() );
	if( s_factories.find( hash ) != s_factories.end() )
	{
		// factory with this type already exists
		return false;
	}

	s_factories.insert(FactoryPair( hash, factory ));

	return true;
}