#pragma once

#include <stdint.h>



class HgTime {
public:
	HgTime() : m_nanoseconds(0) {}
	//HgTime(uint64_t begin, uint64_t end);
	
	static HgTime msec(double t);
	static HgTime seconds(double t);
	static HgTime microseconds(uint64_t t);
	static HgTime nanoseconds(uint64_t t);
	//inline static HgTime seconds(uint32_t t) { return HgTime((uint32_t)(t*1000.0f)); }

	void zero();

	inline double msec() const {	return m_nanoseconds / 1000000.0; }
	inline double seconds() const { return m_nanoseconds / 1000000000.0; }

	inline HgTime operator+(HgTime t) const { return HgTime(m_nanoseconds + t.m_nanoseconds);  }
	inline HgTime& operator+=(HgTime t) { m_nanoseconds += t.m_nanoseconds; return *this; }

	inline HgTime operator-(HgTime t) const { return HgTime(m_nanoseconds - t.m_nanoseconds); }
	inline HgTime& operator-=(HgTime t) { m_nanoseconds -= t.m_nanoseconds; return *this; }

	inline bool operator<(HgTime t) const { return m_nanoseconds < t.m_nanoseconds; }
	inline bool operator<=(HgTime t) const { return m_nanoseconds <= t.m_nanoseconds; }

	inline bool operator>(HgTime t) const { return m_nanoseconds > t.m_nanoseconds; }
	inline bool operator>=(HgTime t) const { return m_nanoseconds >= t.m_nanoseconds; }

private:
	//HgTime(double ms) : m_nanoseconds(ms * 1000000) {}
	HgTime(uint64_t nanoseconds) : m_nanoseconds(nanoseconds) {}
	uint64_t m_nanoseconds;
};

class HgTimer {
public:
	HgTimer();
	void start();

	inline HgTime getElasped() const
	{
		const auto dt = ( currentTime() - m_startTime );
		return HgTime::nanoseconds(dt);
	}

	inline HgTime getElaspedAndRestart()
	{
		const auto t = currentTime();
		const auto dt = (t - m_startTime);
		m_startTime = t;
		return HgTime::nanoseconds(dt);
	}

private:

	//Returns nanoseconds
	uint64_t currentTime() const;

	uint64_t m_startTime;
};

