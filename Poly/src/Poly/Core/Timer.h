#pragma once

#include "Timestamp.h"

/*
*	Timer tracks time from when it is created and is updated with tick.
*/

namespace Poly
{
	class Timer
	{
	public:
		Timer();
		~Timer() = default;

		/*
		*	Update delta and total time.
		*	Note: Total time is calculated from when the object was created.
		*/
		void Tick();

		/*
		*	Reset the delta and total time.
		*/
		void Reset();

		Timestamp GetDeltaTime() const;
		Timestamp GetTotalTime() const;
		
	private:
		Timestamp	m_TotalTime;
		Timestamp	m_DeltaTime;
		uint64_t	m_PrevTime;
	}
}