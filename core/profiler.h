#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#include "containers.h"
#include "timer.h"
#include "string_hashing.h"

#define PROFILER_RESET() ProfilerSingleton::it()->Reset()
#define PROFILER_CLEANUP() ProfilerSingleton::it()->Cleanup()
#define SCOPED_PROFILE(Name) ScopedProfiler Name_profile(#Name)
#define PROFILER_ITERATE_DATA(ItName) for( ProfilerSingleton::ProfileDataMap::iterator ItName = ProfilerSingleton::it()->begin(); \
											ItName != ProfilerSingleton::it()->end();	\
											++ItName )
#define PROFILER_TOTAL() ProfilerSingleton::it()->TotalData();

class IProfilerSet
{
public:
	virtual void SetProfileData( const char* name, float timeStamp, int stackMod ) = 0;
	virtual Timer& GetTimer() = 0;
};

class ProfilerSingleton : public IProfilerSet
{
public:
	static ProfilerSingleton* it()
	{
		if( s_it == NULL )
		{
			s_it = new ProfilerSingleton;
		}
		return s_it;
	}

	inline void Reset();

	void Cleanup()
	{
		Reset();
		delete s_it;
		s_it = NULL;
	}

	struct ProfileData
	{
		ProfileData()
			: mStackCount(0)
		{
		}
		float mTimeDiff;
		float mTimestamp;
		std::string mName;
		int mStackCount;
	};
	DEFINE_MAP(ProfileDataMap, StringHashing::StringHash, ProfileData);

	ProfileDataMap::iterator begin()
	{
		return m_profileData.begin();
	}
	ProfileDataMap::iterator end()
	{
		return m_profileData.end();
	}
	inline int TotalData()
	{
		return m_profileData.size();
	}

private:
	inline virtual void SetProfileData( const char* name, float timeStamp, int stackMod );
	virtual Timer& GetTimer(){
		return m_timer;
	}

	ProfilerSingleton()
	{
		m_timer.reset();
	}

	ProfileDataMap m_profileData;
	Timer m_timer;
	static ProfilerSingleton* s_it;
};

inline void ProfilerSingleton::SetProfileData( const char* name, float timeStamp, int stackMod )
{
	ProfileDataMap::iterator it = m_profileData.find( StringHashing::getHash(name) );
	if( it == m_profileData.end() )
	{
		ProfileData newPd;
		newPd.mName = name;
		newPd.mStackCount=1;
		newPd.mTimestamp = timeStamp;
		newPd.mTimeDiff = timeStamp;
		m_profileData.insert( std::pair<StringHashing::StringHash, ProfileData>( StringHashing::getHash(name),
																				 newPd ) );
	}
	else
	{
		(*it).second.mTimeDiff = timeStamp - (*it).second.mTimestamp;
		(*it).second.mStackCount += stackMod;
		(*it).second.mTimestamp = timeStamp;

	}
}

inline void ProfilerSingleton::Reset()
{
	m_profileData.clear();
}

class ScopedProfiler
{
public:
	ScopedProfiler(const char* name)
		: mName(name)
	{
		ProfilerSingleton* ps = ProfilerSingleton::it();
		float timeStamp = ((IProfilerSet*)ps)->GetTimer().getSystemTime();
		((IProfilerSet*)ps)->SetProfileData( name, timeStamp, 1 );
	};
	~ScopedProfiler()
	{
		ProfilerSingleton* ps = ProfilerSingleton::it();
		float timeStamp = ((IProfilerSet*)ps)->GetTimer().getSystemTime();
		((IProfilerSet*)ps)->SetProfileData( mName, timeStamp, -1 );
	}
private:
	const char* mName;
};

#endif