#ifndef CONTAINERS_H_INCLUDED
#define CONTAINERS_H_INCLUDED

#include <map>
#include <vector>

#define DEFINE_LIST(name,type) typedef std::list<type> nameList;	\
							   typedef std::list<type>::iterator nameListIterator;

#define DEFINE_MAP(name,keyType, dataType) typedef std::map<keyType,dataType> nameMap;	\
										   typedef std::map<keyType,dataType>::iterator nameMapIterator;	\
										   typedef std::pair<keyType,dataType> nameMapPair;

#endif