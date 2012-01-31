#ifndef MINMAX_H_INCLUDED
#define MINMAX_H_INCLUDED

#include <D3D10.h>

namespace Bounds
{

template<typename T>
T Min(const T& t0, const T& t1)
{
	return (t0 < t1) ? t0 : t1;
}

template<typename T>
T Max(const T& t0, const T& t1)
{
	return (t0 > t1) ? t0 : t1;
}

}

#endif