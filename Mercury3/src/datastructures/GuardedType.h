#pragma once

#include <mutex>

//Mutex guarded access to type variable
template<typename T>
class GuardedType
{
public:
	GuardedType<T>()
	{}

	GuardedType<T>(const GuardedType<T>& other)
	{
		*this = other;
	}

	GuardedType<T>& operator=(const T& rhs) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_value = rhs;
		return *this;
	}

	GuardedType<T>& operator=(const GuardedType<T>& rhs)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::lock_guard<std::mutex> lock_rhs(rhs.m_mutex);
		m_value = rhs;
		return *this;
	}

	//operator T() {
	//	std::lock_guard<std::mutex> lock(m_mutex);
	//	return m_value;
	//}

	operator T() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_value;
	}

	class Accessor
	{
	public:
		Accessor(const GuardedType<T> x)
			:m_lock(x.m_mutex), m_ptr(&x.m_value)
		{}

		T& get() const { return *m_ptr; }
	private:
		T* m_ptr;
		std::lock_guard<std::mutex> m_lock;
	};


private:
	T m_value;
	mutable std::mutex m_mutex;
};