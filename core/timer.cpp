#include "core/timer.h"

Timer::Timer()
	: m_multiplier(1.0f)
{
	QueryPerformanceFrequency(&m_frequency);

	reset();
}

Timer::~Timer()
{
}

void Timer::reset()
{
	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_startTime);
	m_currentTime = m_startTime;
	m_currentSeconds = 0.0f;
	m_deltaSeconds = 0.0f;
}

float Timer::getSystemTime()
{
	LARGE_INTEGER thisTime;
	QueryPerformanceCounter( &thisTime );
	float result = (float)(thisTime.QuadPart - m_startTime.QuadPart) / (float)m_frequency.QuadPart;

	return result;
}

void Timer::nextTick()
{
	LARGE_INTEGER thisTime;
	QueryPerformanceCounter( &thisTime );

	m_deltaTime.QuadPart = max(thisTime.QuadPart - (m_currentTime.QuadPart + m_startTime.QuadPart),0);
	m_currentTime.QuadPart = thisTime.QuadPart - m_startTime.QuadPart;

	m_currentSeconds = m_multiplier * ((float)m_currentTime.QuadPart / (float)m_frequency.QuadPart);
	m_deltaSeconds = m_multiplier * ((float)m_deltaTime.QuadPart / (float)m_frequency.QuadPart);
}

float Timer::getDelta()
{
	return m_deltaSeconds;
}

float Timer::elapsedSeconds()
{
	return m_currentSeconds;
}