#pragma once

#include <mutex>
#include <condition_variable>

class ConditionalWait {
public:
	ConditionalWait() : m_continue(false)
	{}

	void wait() {
		m_continue = false;
		std::unique_lock<std::mutex> lock(m_mutex);
		while (!canContinue()) m_condition.wait(lock);
	}
	void resume() {
		m_continue = true;
		m_condition.notify_one();
	}
private:
	bool canContinue() const { return m_continue; }

	std::atomic<bool> m_continue;
	std::mutex m_mutex;
	std::condition_variable m_condition;
};
