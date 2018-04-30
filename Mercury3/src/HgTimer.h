#pragma once

#include <stdint.h>

class HgTime {
public:
	HgTime() : m_msec(0) {}
	//HgTime(uint64_t begin, uint64_t end);
	
	inline static HgTime msec(uint32_t t) { return HgTime(t); }
	inline static HgTime seconds(uint32_t t) { return HgTime((uint32_t)(t*1000.0f)); }

	inline uint32_t msec() const {	return m_msec; }
	inline float seconds() const { return m_msec / 1000.0f; }

	inline HgTime operator+(HgTime t) const { return HgTime(m_msec + t.m_msec);  }
	inline HgTime& operator+=(HgTime t) { m_msec += t.m_msec; return *this; }

	inline HgTime operator-(HgTime t) const { return HgTime(m_msec - t.m_msec); }
	inline HgTime& operator-=(HgTime t) { m_msec -= t.m_msec; return *this; }

	inline bool operator<(HgTime t) const { return m_msec < t.m_msec; }
	inline bool operator<=(HgTime t) const { return m_msec <= t.m_msec; }

	inline bool operator>(HgTime t) const { return m_msec > t.m_msec; }
	inline bool operator>=(HgTime t) const { return m_msec >= t.m_msec; }

private:
	HgTime(uint32_t t) : m_msec(t) {}
	uint32_t m_msec;
};

class HgTimer {
public:
	HgTimer();
	void start();
	inline HgTime getElasped() const { uint64_t t = (currentTime() / 1000) - (m_startTime / 1000); return HgTime::msec((uint32_t)t); }
//	inline float f_millisecondsElasped() const { return (currentTime() - m_startTime) / 1000.0f; }
private:
	uint64_t currentTime() const;

	uint64_t m_startTime;

	uint64_t m_wtime;
};

