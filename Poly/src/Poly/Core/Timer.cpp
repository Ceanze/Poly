#include "polypch.h"
#include "Timer.h"

#include <chrono>

namespace Poly
{
	Timer::Timer()
		: m_DeltaTime(0)
		, m_TotalTime(0)
		, m_PrevTime(0)
	{
		// Will set the m_PrevTime to avoid a big delta after the first Tick is called
		Tick();
	}

	void Timer::Tick()
	{
		// static auto currTime = std::chrono::high_resolution_clock::now();
		// 	dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - currTime).count();
		// 	currTime = std::chrono::high_resolution_clock::now();
		const uint64_t now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		const uint64_t delta = now - m_PrevTime;
		m_PrevTime = now;

		m_DeltaTime = Timestamp(delta);
		m_TotalTime += m_DeltaTime;
	}

	void Timer::Reset()
	{
		m_TotalTime	= Timestamp(0);
		m_DeltaTime	= Timestamp(0);
		m_PrevTime	= 0;
	}

	Timestamp Timer::GetDeltaTime() const
	{
		return m_DeltaTime;
	}

	Timestamp Timer::GetTotalTime() const
	{
		return m_TotalTime;
	}
}