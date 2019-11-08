#include <HgTimer.h>
#include <Windows.h>
#include <cmath>

HgTime HgTime::msec(double t)
{
	HgTime tmp(t * 1000000.0);
	return tmp;
}

HgTime HgTime::seconds(double t)
{
	HgTime tmp(t * 1000000000.0);
	return tmp;
}

HgTime HgTime::microseconds(uint64_t t)
{
	HgTime tmp(t * 1000);
	return tmp;
}

HgTime HgTime::nanoseconds(uint64_t t)
{
	HgTime tmp(t);
	return tmp;
}

HgTimer::HgTimer()
	:m_startTime(0)
{

}

void HgTimer::start() {
	m_startTime = currentTime();
}

uint64_t HgTimer::currentTime() const {
	uint64_t wTime;
	//	return GetTickCount64();
	//use QueryUnbiasedInterruptTime in place of GetTickCount64 as GetTickCount64 has a resolution between 10 and 16 milliseconds
	QueryUnbiasedInterruptTime((PULONGLONG)&wTime); //100 nano seconds
	return wTime*100; //nanoseconds
}

//HgTime::HgTime(uint64_t begin, uint64_t end) {
//	uint64_t dt = begin - end;
//	m_msec = (uint32_t)dt; //should not overflow unless dt is greater than 49 days
//}