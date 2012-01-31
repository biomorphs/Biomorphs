#ifndef CONTAINERS_H_INCLUDED
#define CONTAINERS_H_INCLUDED

#include <map>
#include <vector>

#define DEFINE_PAIR(name,keyType, dataType)	typedef std::pair<keyType,dataType> name;

#define DEFINE_LIST(name,type) typedef std::list<type> name;	\
							   typedef std::list<type>::iterator nameIterator;

#define DEFINE_MAP(name,keyType, dataType) typedef std::map<keyType,dataType> name;	\
										   typedef std::pair<keyType,dataType> namePair;

#endif