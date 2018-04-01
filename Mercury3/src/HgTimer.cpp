#include <HgTimer.h>
#include <Windows.h>

HgTimer::HgTimer()
	:m_startTime(0)
{

}

void HgTimer::start() {
	m_startTime = currentTime();
}

uint64_t HgTimer::currentTime() const {
	//	return GetTickCount64();
	//use QueryUnbiasedInterruptTime in place of GetTickCount64 as GetTickCount64 has a resolution between 10 and 16 milliseconds
	QueryUnbiasedInterruptTime((PULONGLONG)&m_wtime); //
	return (m_wtime / 10000); //milliseconds
}
