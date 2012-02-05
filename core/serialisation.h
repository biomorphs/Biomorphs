#ifndef SERIALISATION_H_INCLUDED
#define SERIALISATION_H_INCLUDED

#include "serialiser.h"

enum SerialMode
{
	MODE_WRITE,
	MODE_READ
};

#define DECLARE_SERIALISED(className) \
	inline void className::Serialise( Serialiser& s, SerialMode mode );

#define SERIALISE_BEGIN(className) \
	inline void className::Serialise( Serialiser& s, SerialMode mode ) \
	{ \

#define SERIALISE_END(className) \
	}

#define DEFINE_SERIALISED_CLASS(className, vars) \
	SERIALISE_BEGIN(className) \
	vars \
	SERIALISE_END(className)

#define DECLARE_VALUE( variable ) \
	if( mode == MODE_WRITE ) \
	{ \
		s.AddNode( #variable, variable );	\
	} \
	else \
	{	\
		Serialiser::SerialNode* n = s.GetNode( #variable );	\
		if( n ) \
			n->GetValue( variable );	\
	}

#define DECLARE_STRING( variable ) \
	if( mode == MODE_WRITE ) \
	{ \
		s.AddNode( #variable, (const char*)variable );	\
	}	\
	else	\
	{	\
		Serialiser::SerialNode* n = s.GetNode( #variable );	\
		if( n ) \
		{	\
			char* v = (char*)variable;	\
			n->GetValue(v);	\
		}	\
	}

#endif