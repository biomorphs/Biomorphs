#ifndef VECTOR2_INCLUDED
#define VECTOR2_INCLUDED

#include <math.h>

class Vector2
{
public:
	Vector2()
	{
	}

	Vector2(int x, int y)
		: m_x(x)
		, m_y(y)
	{
	}

	Vector2(int t)
		: m_x(t)
		, m_y(t)
	{
	}

	inline bool operator!=(const Vector2& t) const
	{
		return (m_x != t.m_x || m_y != t.m_y);
	}

	inline bool operator==(const Vector2& t) const
	{
		return !(t != *this);
	}

	inline int& x() { return m_x; }
	inline int& y() { return m_y; }

	inline Vector2 operator*(float f)
	{
		return Vector2((int)(m_x*f),(int)(m_y*f));
	}

	inline Vector2 operator+(Vector2 f)
	{
		return Vector2((int)(m_x+f.m_x),(int)(m_y+f.m_y));
	}

	inline Vector2 operator-(Vector2 f)
	{
		return Vector2((int)(m_x-f.m_x),(int)(m_y-f.m_y));
	}
	
	inline float length()
	{
		return sqrt((float)((m_x*m_x) + (m_y*m_y)));
	}

	inline Vector2 normalise()
	{
		float l=length();
		return *this * l;
	}

private:
	int m_x;
	int m_y;
};

class Vector2f
{
public:
	Vector2f()
	{
	}

	Vector2f(float x, float y)
		: m_x(x)
		, m_y(y)
	{
	}

	Vector2f(float v)
		: m_x(v)
		, m_y(v)
	{
	}

	inline bool operator!=(const Vector2f& t) const
	{
		return (m_x != t.m_x || m_y != t.m_y);
	}

	inline bool operator==(const Vector2f& t) const
	{
		return !(t != *this);
	}

	inline float& x() { return m_x; }
	inline float& y() { return m_y; }

	inline Vector2f operator*(float f)
	{
		return Vector2f(m_x*f,m_y*f);
	}

	inline Vector2f operator+(Vector2f f)
	{
		return Vector2f(m_x+f.m_x,m_y+f.m_y);
	}

	inline Vector2f operator-(Vector2f f)
	{
		return Vector2f((m_x-f.m_x),(m_y-f.m_y));
	}
	
	inline float length()
	{
		return sqrt((m_x*m_x) + (m_y*m_y));
	}

	inline Vector2f normalise()
	{
		float l=length();
		return *this * (1.0f / l);
	}

private:
	float m_x;
	float m_y;
};

#endif