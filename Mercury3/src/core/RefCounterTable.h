#pragma once

#include <vector>
#include <stdint.h>

template<typename T>
class RefCountedTable {
public:
	using SelfType = RefCountedTable<T>;
	using DataType = T;
	//using IndexType = IndexType<SelfType>;

	class IndexType {
	public:
		using IdxType = uint32_t;

		IndexType() : m_idx(0)
		{
			m_idx = SelfType::Singleton().NewRecord() + 1;
		}

		~IndexType()
		{
			Decrement();
		}
		IndexType(const IndexType& o)
			: m_idx(o.m_idx)
		{
			Increment();
		}

		IndexType& operator=(IndexType other) noexcept
		{
			//I don't think decrement or increment need to happen here.
			//Take it from the copied argument
			std::swap(m_idx, other.m_idx);
			return *this;
		}

		IndexType(IndexType&& other) noexcept
			: m_idx(std::exchange(other.m_idx, 0)) 
		{}

		IdxType Index() const { return m_idx - 1; }

		SelfType::DataType& Record() const
		{
			if (m_idx != 0) {
				return SelfType::Singleton().getRecord(*this);
			}
			return SelfType::DataType(); //eewwww
		}
	private:
		void Decrement()
		{
			if (m_idx != 0) {
				SelfType::Singleton().DecrementRecordCount(*this);
			}
		}

		void Increment()
		{
			if (m_idx != 0) {
				SelfType::Singleton().IncrementRecordCount(*this);
			}
		}

		explicit IndexType(IdxType idx) : m_idx(idx + 1)
		{}

		//void IndexType(IndexType idx) { m_idx = idx+1; }

		IdxType m_idx;
		friend class SelfType;
	};

	IndexType InsertRecord(T& vboRec)
	{
		IndexType::IdxType index;
		if (m_unusedRecords.size() > 0) {
			index = m_unusedRecords.back();
			m_unusedRecords.pop_back();
			m_records[index] = vboRec;
			m_useCount[index] = 1;
		}
		else {
			index = (IndexType::IdxType)m_records.size();
			m_records.push_back(vboRec);
			m_useCount.push_back(1);
		}
		return IndexType(index);
	}

	//Create new record with default constructor
	auto NewRecord()
	{
		IndexType::IdxType index;
		if (m_unusedRecords.size() > 0) {
			index = m_unusedRecords.back();
			m_unusedRecords.pop_back();
			//m_records[index] = vboRec;
			m_useCount[index] = 1;
		}
		else {
			index = (IndexType::IdxType)m_records.size();
			m_records.push_back(T());
			m_useCount.push_back(1);
		}
		return index;
	}

	inline const T& getRecord(const IndexType& x) const
	{
		const auto idx = x.Index();
		return m_records[idx];
	}

	inline T& getRecord(const IndexType& x)
	{
		const auto idx = x.Index();
		return m_records[idx];
	}

	static SelfType& Singleton() { return singleton; }

private:
	void IncrementRecordCount(const IndexType& x)
	{
		auto idx = x.Index();
		if (m_useCount.size() > idx) {
			m_useCount[idx]++;
		}
	}
	void DecrementRecordCount(const IndexType& x)
	{
		auto idx = x.Index();
		if (m_useCount.size() > idx) {
			m_useCount[idx]--;
			if (m_useCount[idx] == 0) {
				m_unusedRecords.push_back(idx);
				//fprintf(stderr, "reference count reached zero\n");
				m_records[idx] = T();
				//run cleanup routine here
			}
		}
	}

	std::vector< T > m_records;
	std::vector< uint32_t > m_useCount;
	std::vector< uint32_t > m_unusedRecords;

	static SelfType singleton;
	friend class IndexType;
};