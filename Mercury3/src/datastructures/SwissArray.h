#pragma once

#include <vector>

// Array that can have holes in it. Like swiss cheese :/
template<typename T>
class SwissArray {
public:
	class iterator;

	SwissArray() : m_size(0), m_usedSize(0), m_allocatedSize(0), head(nullptr) {
		allocate(1);
	}

	SwissArray(uint32_t reserve) : m_size(0), m_usedSize(0), m_allocatedSize(0), head(nullptr) {
		//		m_entities.reserve(reserve);
		allocate(reserve);
	}

	~SwissArray() {
		for (size_t i = 0; i < m_size; ++i) {
			if (m_used[i]) head[i].~T();
		}
		if (head != nullptr) m_alloc.deallocate(head, m_allocatedSize);
	}
	
	inline T& newItem() {
		for (size_t i = 0; i < m_size; ++i) {
			if (!m_used[i]) {
				m_used[i] = true;
				m_usedSize++;
				T& tmp = head[i];
				new (&tmp) T();
				return tmp;
			}
		}
		//		m_size++;
		//		m_entities.push_back(T());
		T& tmp = push_back();
		new (&tmp) T();
		m_used.push_back(true);
		m_usedSize++;
		return tmp;
	}

	inline void remove(const T& x) {
		//super scalar the next 2 lines?
		ptrdiff_t i = &x - head;
		if ((&x - head) > ptrdiff_t(m_size)) return;
		head[i].~T();
		m_used[i] = false;
		m_usedSize--;
		//		m_remove.insert(&x);
	}

	inline iterator erase(const iterator& itr) {
		iterator r(itr);

		if (itr.itr_ < m_size) {
			T& tmp = head[itr.itr_];
			tmp.~T();
			m_used[itr.itr_] = false;
			m_usedSize--;
			r++;
		}

		return r;
	}

	inline iterator at(uint32_t idx)
	{
		return iterator(idx, this);
	}

	class iterator
	{
	public:
		typedef iterator self_type;
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;

		iterator(size_t i, SwissArray<T>* c) : itr_(i), sa(c) { }

		inline self_type& operator++() { //prefix
			do {
				itr_++;
				if (itr_ >= sa->m_size) break;
				if (sa->m_used[itr_]) break;
			} while (true);
			return *this;
		}

		inline self_type operator++(int junk) { //postfix
			self_type copy(*this);
			do {
				itr_++;
				if (itr_ >= sa->m_size) break;
				if (sa->m_used[itr_]) break;
			} while (true);
			return copy;
		}

		inline value_type& operator*() { return sa->operator[](itr_); }
		inline value_type* operator->() { return &sa->operator[](itr_); }
		inline bool operator==(const self_type& rhs) const { return (sa == rhs.sa) && (itr_ == rhs.itr_); }
		inline bool operator!=(const self_type& rhs) const { return (sa != rhs.sa) || (itr_ != rhs.itr_); }

		inline bool itemValid() const { return sa->m_used[itr_]; }

		inline auto index() const { return itr_; }
	private:
		SwissArray<T>* sa;
		size_t itr_;
		friend SwissArray<T>;
	};


	//class const_iterator
	//{
	//public:
	//	typedef const_iterator self_type;
	//	typedef T value_type;
	//	typedef T& reference;
	//	typedef T* pointer;
	//	typedef std::forward_iterator_tag iterator_category;
	//	typedef int difference_type;

	//	const_iterator(size_t i, UpdatableCollection* c) : itr_(i), collection(c) { }

	//	self_type operator++() {
	//		self_type i = *this;
	//		do {
	//			itr_++;
	//			if (itr_ >= collection->m_size) break;
	//			if (collection->m_used[itr_]) break;
	//		} while (true);
	//		return i;
	//	}

	//	self_type operator++(int junk) {  //post increment
	//		do {
	//			itr_++;
	//			if (itr_ >= collection->m_size) break;
	//			if (collection->m_used[itr_]) break;
	//		} while (true);
	//		return *this;
	//	}

	//	const reference operator*() { return collection->operator[](itr_); }
	//	const pointer operator->() { return &collection->operator[](itr_); }
	//	bool operator==(const self_type& rhs) { return(collection == rhs.collection) || (itr_ == rhs.itr_); }
	//	bool operator!=(const self_type& rhs) { return (collection != rhs.collection) || (itr_ != rhs.itr_); }
	//private:
	//	UpdatableCollection<T>* collection;
	//	size_t itr_;
	//	friend UpdatableCollection<T>;
	//};


	iterator begin()
	{
		for (size_t i = 0; i < m_used.size(); ++i) {
			if (m_used[i]) return iterator(i, this);
		}
		return end();
	}

	iterator end()
	{
		return iterator(m_size, this);
	}

	//const_iterator begin() const
	//{
	//	size_t i = 0;
	//	for (; i < m_used.size(); ++i) {
	//		if (m_used[i]) break;
	//	}
	//	return iterator(i, this);
	//}

	//const_iterator end() const
	//{
	//	return iterator(m_entities.size(), this);
	//}

	inline bool empty() const { return m_usedSize == 0; }

	//Number of instantiated items
	inline size_t count() const { return m_usedSize; }

	//Highwater mark of instantiated items
	inline size_t size() const { return m_size; }

private:
	inline T& operator[](size_t index)
	{
		return head[index];
	}

	//add uninitalized element
	T& push_back() {
		if (m_size >= m_allocatedSize) reallocate();
		T& p = head[m_size];
		m_size++;
		return p;
	}

	void reallocate() {
		size_t newSize = m_allocatedSize * 2;
		allocate(newSize);
	}

	void allocate(size_t size) {
		T* p = m_alloc.allocate(size);
		for (size_t i = 0; i < m_allocatedSize; i++)
		{
			if (m_used[i])
			{
				new (p + i) T(std::move(head[i])); //move constructor
				(head+i)->~T();
			}
		}

		//memcpy(p, head, sizeof(T)*m_allocatedSize);
		m_alloc.deallocate(head, m_allocatedSize);
		m_allocatedSize = size;
		head = p;
	}

	size_t m_size; //number of elementes in head and m_used
	size_t m_usedSize; //number of used items. <= m_size

	size_t m_allocatedSize;
	T* head;

	std::allocator<T> m_alloc;
//	std::vector<T> m_entities;

	std::vector<bool> m_used;

	
};