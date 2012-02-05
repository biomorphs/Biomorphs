#ifndef SERIALISATION_H_INCLUDED
#define SERIALISATION_H_INCLUDED

#include "serialiser.h"

#define DECLARE_SERIALISED(className) \
	void className::Serialise( Serialiser& s, bool save );

#define SERIALISE_BEGIN(className) \
	void className::Serialise( Serialiser& s, bool save ) \
	{ \

#define SERIALISE_END(className) \
	}

#define DEFINE_SERIALISATION(className, vars) \
	SERIALISE_BEGIN(className) \
	vars \
	SERIALISE_END(className)

#define DECLARE_VALUE( variable ) \
	if( save ) \
	{ \
		s.AddNode( #variable, variable );	\
	}

#endif