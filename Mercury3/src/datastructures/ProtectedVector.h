#pragma once

//Thread safe vector
template<typename T>
class ProtectedVector
{
public:
	typedef  std::vector<T> vectorType;
	void push_back(T& x)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_values.push_back(x);
	}

	void swap(std::vector<T>& v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_values.swap(v);
	}

private:
	std::mutex m_mutex;
	std::vector<T> m_values;
};