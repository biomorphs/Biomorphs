#ifndef PERF_GRAB_INCLUDED
#define PERF_GRAB_INCLUDED

#define ENABLE_PERF_GRAB

#ifdef ENABLE_PERF_GRAB
	#define DoPerfGrab(stuff) stuff
	#define DeclarePerfGrab(Name, Slotcount, Slottype) \
	typedef PerfGrab<Slotcount, Slottype> Name; \
	int Name::m_slots[Slotcount];

#else
	#define DoPerfGrab(stuff)
#endif

template<int SlotCount, typename EnumName>
class PerfGrab
{
public:
	static void Reset(int slot=-1)
	{
		int start = ( slot >= 0 && slot < SlotCount ) ? slot : 0;
		int end = ( slot == -1 ) ? SlotCount : 0;
		for(int i=start; i<end; ++i)
		{
			m_slots[i] = 0;
		}
	}

	static inline void Increment(int slot, int count=1)
	{
		if ( slot >= 0 && slot < SlotCount )
		{
			m_slots[slot] += count;
		}
	}

	static inline void Decrement(int slot, int count=1)
	{
		if ( slot >= 0 && slot < SlotCount )
		{
			m_slots[slot] += count;
		}
	}
	
	static inline int Grab(int slot)
	{
		return ( slot >= 0 && slot < SlotCount ) ? m_slots[slot] : 0;		
	}

private:
	static int m_slots[SlotCount];
};

#endif