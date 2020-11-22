#pragma once
#include <stdint.h>

/*
*	Credit to Alexander Dahlin (github.com/Mumsfilibaba) for implementation that I stole
*/

/*
*	Timestamp is a container for a time that is kept in nano seconds.
	It gives shorthand functions to get seconds, milliseconds, and microseconds.
*/

namespace Poly
{
	class Timestamp
	{
	public:
		inline Timestamp(uint64_t nanoSeconds = 0)
			: m_NanoSeconds(nanoSeconds)
		{}

		inline Timestamp(const Timestamp& other)
			: m_NanoSeconds(other.m_NanoSeconds)
		{}

		inline Timestamp(Timestamp&& other)
			: m_NanoSeconds(other.m_NanoSeconds)
		{ other.m_NanoSeconds = 0; }

		inline double Seconds() const
		{
			constexpr double SECONDS = 1000.0 * 1000.0 * 1000.0;
			return static_cast<double>(m_NanoSeconds) / SECONDS;
		}

		inline double MilliSeconds() const
		{
			constexpr double MILLI_SECONDS = 1000.0 * 1000.0;
			return static_cast<double>(m_NanoSeconds) / MILLI_SECONDS;
		}

		inline double MicroSeconds() const
		{
			constexpr double MICRO_SECONDS = 1000.0;
			return static_cast<double>(m_NanoSeconds) / MICRO_SECONDS;
		}

		inline uint64_t NanoSeconds() const
		{ return m_NanoSeconds; }

		inline Timestamp& operator=(const Timestamp& other)
		{
			m_NanoSeconds = other.m_NanoSeconds;
			return *this;
		}

		inline Timestamp& operator=(Timestamp&& other) noexcept
		{
			m_NanoSeconds = other.m_NanoSeconds;
			other.m_NanoSeconds = 0;
			return *this;
		}

		inline bool operator==(const Timestamp& other) const
		{
			return m_NanoSeconds == other.m_NanoSeconds;
		}

		inline bool operator!=(const Timestamp& other) const
		{
			return m_NanoSeconds != other.m_NanoSeconds;
		}

		inline Timestamp& operator+=(const Timestamp& other)
		{
			m_NanoSeconds += other.m_NanoSeconds;
			return *this;
		}

		inline Timestamp& operator-=(const Timestamp& other)
		{
			m_NanoSeconds -= other.m_NanoSeconds;
			return *this;
		}

		inline Timestamp& operator*=(const Timestamp& other)
		{
			m_NanoSeconds *= other.m_NanoSeconds;
			return *this;
		}

		inline Timestamp& operator/=(const Timestamp& other)
		{
			m_NanoSeconds /= other.m_NanoSeconds;
			return *this;
		}

		friend Timestamp operator+(const Timestamp& lhs, const Timestamp& rhs);
		friend Timestamp operator-(const Timestamp& lhs, const Timestamp& rhs);
		friend Timestamp operator*(const Timestamp& lhs, const Timestamp& rhs);
		friend Timestamp operator/(const Timestamp& lhs, const Timestamp& rhs);
		friend bool operator>(const Timestamp& lhs, const Timestamp& rhs);
		friend bool operator<(const Timestamp& lhs, const Timestamp& rhs);
		friend bool operator>=(const Timestamp& lhs, const Timestamp& rhs);
		friend bool operator<=(const Timestamp& lhs, const Timestamp& rhs);

	private:
		uint64_t m_NanoSeconds = 0;
	};


	inline Timestamp operator+(const Timestamp& left, const Timestamp& right)
	{
		return Timestamp(left.m_NanoSeconds + right.m_NanoSeconds);
	}

	inline Timestamp operator-(const Timestamp& left, const Timestamp& right)
	{
		return Timestamp(left.m_NanoSeconds - right.m_NanoSeconds);
	}

	inline Timestamp operator*(const Timestamp& left, const Timestamp& right)
	{
		return Timestamp(left.m_NanoSeconds * right.m_NanoSeconds);
	}

	inline Timestamp operator/(const Timestamp& left, const Timestamp& right)
	{
		return Timestamp(left.m_NanoSeconds / right.m_NanoSeconds);
	}

	inline bool operator>(const Timestamp& left, const Timestamp& right)
	{
		return left.m_NanoSeconds > right.m_NanoSeconds;
	}

	inline bool operator<(const Timestamp& left, const Timestamp& right)
	{
		return left.m_NanoSeconds < right.m_NanoSeconds;
	}

	inline bool operator>=(const Timestamp& left, const Timestamp& right)
	{
		return (left.m_NanoSeconds >= right.m_NanoSeconds);
	}

	inline bool operator<=(const Timestamp& left, const Timestamp& right)
	{
		return (left.m_NanoSeconds <= right.m_NanoSeconds);
	}
}