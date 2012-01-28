#ifndef MINMAX_H_INCLUDED
#define MINMAX_H_INCLUDED

template<class T>
T min(const T& t0, const T& t1)
{
	return (t0 < t1) ? t0 : t1;
}

template<class T>
T max(const T& t0, const T& t1)
{
	return (t0 > t1) ? t0 : t1;
}

#endif