#pragma once

#include <vector>
#include <cassert>
#include <stdint.h>

template< typename T >
void swap_last_erase(std::vector<T>& v, uint32_t i)
{
	const auto last = v.size() - 1;
	std::swap(v[i], v[last]);
	v.pop_back();
}

template< typename T >
typename std::vector<T>::iterator
insert_sorted(std::vector<T>& vec, const T& item)
{
	const auto itr = std::upper_bound(vec.begin(), vec.end(), item);
	if ((itr != vec.end()) && (*itr == item))
	{
		*itr = item;
		return itr;
	}
	return vec.insert(itr, item);
}

template< typename T >
typename std::vector<T>::const_iterator
binary_search(const std::vector<T>& vec, const T& item)
{
	auto itr = lower_bound(vec.cbegin(), vec.cend(), item);
	if ((itr == vec.cend()) || (*itr != item))
	{
		return vec.cend();
	}

	return itr;
}

template<typename T>
class OrderedVector {
public:
	void insert(const T& x)
	{
		template< typename T >
		typename std::vector<T>::iterator
			insert_sorted(std::vector<T> & vec, T const& item)
		{
			return vec.insert
			(
				std::upper_bound(vec.begin(), vec.end(), item),
				item
			);
		}
	}

private:
	std::vector<T> m_data;
};