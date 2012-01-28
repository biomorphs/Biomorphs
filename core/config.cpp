#include "config.h"

#include "external\tinyxml\tinyxml.h"

const char c_groupLabel[]	=	"Group";
const char c_elementLabel[] =	"Element";
const char c_nameLabel[]	=	"Name";
const char c_typeLabel[]	=	"Type";
const char c_valueLabel[]	=	"Value";

const char c_typeString[]   =   "String";
const char c_typeBoolean[]	=	"Boolean";
const char c_typeUInt[]		=	"Unsigned Int";

bool Config::addElementOfType( const char* name, const char* type, const char* valueText )
{
	StringHashing::StringHash hash = StringHashing::getHash( name );

	if( m_elements.find( hash ) != m_elements.end() )
	{
		assert(false && "Element already exists");
		return false;
	}

	if( strcmp(type, c_typeString)==0 )
	{
		return addElement<std::string>( name, valueText );
	}
	else if( strcmp( type, c_typeBoolean )==0 )
	{
		bool value = (strcmp( valueText, "True" ) == 0) ? true : false;
		return addElement<bool>( name, value );
	}
	else if( strcmp( type, c_typeUInt )==0 )
	{
		unsigned int value = 0;
		if( sscanf_s(valueText, "%d", &value) )
		{
			return addElement<unsigned int>( name, value );
		}
	}

	return false;
}

bool Config::parseValue( std::string groupName, TiXmlElement* value )
{
	const char* nameAttr = value->Attribute( c_nameLabel );
	const char* typeAttr = value->Attribute( c_typeLabel );
	const char* valueAttr = value->Attribute( c_valueLabel );

	std::string keyName = groupName;
	keyName.append( "." );
	keyName.append( nameAttr );

	if( nameAttr && typeAttr && valueAttr )
	{
		return addElementOfType( keyName.c_str(), typeAttr, valueAttr );
	}

	return false;
}

bool Config::parseGroups( std::string parentName, TiXmlElement* groups )
{
	TiXmlElement* child = groups;
	while( child )
	{
		if( strcmp(child->Value(), c_elementLabel)==0 )
		{
			if( !parseValue( parentName, child ) )
			{
				return false;
			}
		}
		else if( strcmp( child->Value(), c_groupLabel )==0 )
		{
			const char* groupName = child->Attribute( c_nameLabel );

			if( groupName )
			{
				std::string appended = parentName;
				if( parentName.length()>0 )
				{
					appended.append( "." );
				}
				appended.append( groupName );
				if( !parseGroups( appended, child->FirstChildElement() ) )
				{
					return false;
				}
			}
		}

		child = child->NextSiblingElement();
	}

	return true;
}

bool Config::appendFromFile( std::string filename )
{
	TiXmlDocument document( filename.c_str() );

	if( document.LoadFile() )
	{
		TiXmlElement* node = document.FirstChildElement();
		while( node )
		{
			parseGroups( "", node );

			node = node->NextSiblingElement();
		}
		

		return true;
	}

	return false;
}

Config::Config()
{
}

Config::~Config()
{
}