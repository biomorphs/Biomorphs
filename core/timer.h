#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#include <Windows.h>

class Timer
{
public:
	Timer();
	~Timer();

	inline void setMultiplier( float mul )
	{
		m_multiplier = mul;
	}

	inline float getMultiplier()
	{
		return m_multiplier;
	}

	void reset();
	void nextTick();
	float getDelta();
	float elapsedSeconds();
protected:
private:
	LARGE_INTEGER	m_frequency;
	LARGE_INTEGER	m_startTime;		// start and current time are separated so we can get a consistent time through the frame
	LARGE_INTEGER	m_currentTime;
	LARGE_INTEGER	m_deltaTime;
	float			m_multiplier;		// allows us to slooow down or speeeed up time!
	float			m_deltaSeconds;
	float			m_currentSeconds;
};

#endif