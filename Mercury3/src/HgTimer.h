#pragma once

#include <stdint.h>

class HgTimer {
public:
	HgTimer();
	void start();
	inline uint64_t millisecondsElasped() const { return currentTime() - m_startTime; }
private:
	uint64_t currentTime() const;

	uint64_t m_startTime;

	uint64_t m_wtime;
};
