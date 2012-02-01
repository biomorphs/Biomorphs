#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#include "containers.h"
#include "timer.h"
#include "string_hashing.h"

#define PROFILER_RESET() ProfilerSingleton::it()->Reset()
#define PROFILER_CLEANUP() ProfilerSingleton::it()->Cleanup()
#define SCOPED_PROFILE(Name) ScopedProfiler Name_profile(#Name)
#define PROFILER_ITERATE_DATA(ItName) for( ProfilerSingleton::ProfileDataList::iterator ItName = ProfilerSingleton::it()->begin(); \
											ItName != ProfilerSingleton::it()->end();	\
											++ItName )
#define PROFILER_TOTAL() ProfilerSingleton::it()->TotalData();

class IProfilerSet
{
public:
	virtual int SetProfileData( const char* name, float timeStamp, int id=0 ) = 0;
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
			: mID(0)
		{
		}
		float mTimeDiff;
		float mTimestamp;
		int mID;
		int mStackLevel;
		std::string mName;
	};
	DEFINE_LIST(ProfileDataList, ProfileData);

	ProfileDataList::iterator begin()
	{
		return m_profileData.begin();
	}
	ProfileDataList::iterator end()
	{
		return m_profileData.end();
	}
	inline int TotalData()
	{
		return m_profileData.size();
	}

private:
	inline virtual int SetProfileData( const char* name, float timeStamp, int id );
	virtual Timer& GetTimer(){
		return m_timer;
	}

	ProfilerSingleton()
	{
		m_timer.reset();
		m_firstID = 1;
		m_stackLevel = 0;
	}

	ProfileDataList m_profileData;
	Timer m_timer;
	int m_firstID;
	int m_stackLevel;
	static ProfilerSingleton* s_it;
};

inline int ProfilerSingleton::SetProfileData( const char* name, float timeStamp, int id )
{
	if( id != 0 )
	{
		for( ProfileDataList::iterator it = m_profileData.begin();
			it != m_profileData.end();
			++it )
		{
			if((*it).mID == id )
			{
				(*it).mTimeDiff = timeStamp - (*it).mTimestamp;
				m_stackLevel--;
				return id;
			}
		}
	}
	else
	{
		ProfileDataList::iterator it = m_profileData.begin();

		ProfileData newPd;
		newPd.mName = name;
		newPd.mID = m_firstID++;
		newPd.mTimestamp = timeStamp;
		newPd.mTimeDiff = timeStamp;
		newPd.mStackLevel = m_stackLevel++;

		m_profileData.push_back( newPd );

		return newPd.mID;
	}

	return -1;
}

inline void ProfilerSingleton::Reset()
{
	m_profileData.clear();
	m_firstID = 1;
	m_stackLevel = 0;
}

class ScopedProfiler
{
public:
	ScopedProfiler(const char* name)
		: mName(name)
	{
		ProfilerSingleton* ps = ProfilerSingleton::it();
		float timeStamp = ((IProfilerSet*)ps)->GetTimer().getSystemTime();
		mID = ((IProfilerSet*)ps)->SetProfileData( name, timeStamp );
	};
	~ScopedProfiler()
	{
		ProfilerSingleton* ps = ProfilerSingleton::it();
		float timeStamp = ((IProfilerSet*)ps)->GetTimer().getSystemTime();
		((IProfilerSet*)ps)->SetProfileData( mName, timeStamp, mID );
	}
private:
	int mID;
	const char* mName;
};

#endif