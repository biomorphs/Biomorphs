#ifndef CORE_ANGLES_INCLUDED
#define CORE_ANGLES_INCLUDED

namespace Angles
{
	static const float PI = 3.1415926535897932384f;
	static const float OneOverPI = 1.0f / PI;

	static const float TwoPI = PI * 2.0f;
	static const float OneOverTwoPI = 1.0f / TwoPI;

	inline float ToRadians(float degrees)
	{
		return degrees * (PI / 180.0f);
	}
}

#endif