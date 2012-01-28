#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#include <map>
#include "core/string_hashing.h"

class TiXmlElement;
class TiXmlText;

class ConfigElementBase
{
public:
	
};

template<class TYPE>
class ConfigElement : ConfigElementBase
{
public:
	ConfigElement(TYPE value);

	inline TYPE getValue() const;
protected:
private:
	TYPE m_value;
};

template<class TYPE>
inline TYPE ConfigElement<TYPE>::getValue() const
{
	return m_value;
}

template<class TYPE>
ConfigElement<TYPE>::ConfigElement( TYPE value )
{
	m_value = value;
}

class Config
{
public:
	Config();
	~Config();

	bool appendFromFile( std::string filename );

	template< typename TYPE >
	bool addElement( const char* name, TYPE value ); 

	template< typename TYPE >
	bool getElement( const char* name, ConfigElement<TYPE>** element );
protected:
private:

	bool addElementOfType( const char* name, const char* type, const char* valueText );
	bool parseGroups( std::string parentName, TiXmlElement* groups );
	bool parseValue( std::string groupName, TiXmlElement* value );

	typedef std::map<StringHashing::StringHash , ConfigElementBase*> ElementList;
	ElementList m_elements;
};

template< typename TYPE >
bool Config::getElement( const char* name, ConfigElement<TYPE>** element )
{
	StringHashing::StringHash hash = StringHashing::getHash( name );
	ElementList::iterator foundItem = m_elements.find( hash );
	if( foundItem != m_elements.end() )
	{
		ConfigElementBase* elResult = (*foundItem).second;

		*element = (ConfigElement<TYPE>*)elResult;

		return true;
	}

	return false;
}

template< typename TYPE >
bool Config::addElement( const char* name, TYPE value )
{
	ConfigElement<TYPE>* element = new ConfigElement<TYPE>(value);

	StringHashing::StringHash hash = StringHashing::getHash( name );

	if( element != NULL )
	{
		m_elements.insert( std::pair< StringHashing::StringHash, ConfigElementBase* >( hash, (ConfigElementBase*)element ) );

		return true;
	}

	return false;
}


#endif