#pragma once

#include <stdint.h>

class HgDeltaTime {
public:
	HgDeltaTime() : m_msec(0) {}
	HgDeltaTime(uint64_t begin, uint64_t end);
	uint32_t milliseconds() const {	return m_msec; }
private:
	uint32_t m_msec;
};

class HgTimer {
public:
	HgTimer();
	void start();
	inline HgDeltaTime getElasped() const { return HgDeltaTime(currentTime()/1000, m_startTime/1000); }
//	inline float f_millisecondsElasped() const { return (currentTime() - m_startTime) / 1000.0f; }
private:
	uint64_t currentTime() const;

	uint64_t m_startTime;

	uint64_t m_wtime;
};

