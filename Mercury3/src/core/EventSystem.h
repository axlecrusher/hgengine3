#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

namespace EventSystem
{

template<typename T>
class EventObserver
{
public:
	static void PublishEvent(const T& evt)
	{
		for (auto& registrant : m_registrants)
		{
			registrant.callback(evt);
		}

	}

	static void Register(void* ptr, std::function<void(const T&)> clbk)
	{
		registration reg;
		reg.ptr = ptr;
		reg.callback = std::move(clbk);
		m_registrants.push_back(reg);
	}

	static void Unregister(void* ptr)
	{
		auto itr = std::find(m_registrants.begin(), m_registrants.end(), ptr);
		if (itr != m_registrants.end())
		{
			auto last = m_registrants.end() - 1;
			std::iter_swap(itr, last);
			m_registrants.pop_back();
		}
	}
private:
	struct registration
	{
		registration()
			:ptr(nullptr)
		{}

		inline bool operator<(const registration& rhs) const { return ptr < rhs.ptr; }
		inline bool operator==(const registration& rhs) const { return ptr == rhs.ptr; }

		inline bool operator<(const void* rhs) const { return ptr < rhs; }
		inline bool operator==(const void* rhs) const { return ptr == rhs; }

		void* ptr;
		std::function<void(const T&)> callback;
	};

	static std::vector< registration > m_registrants;
};

class IRecipt
{
public:
	virtual ~IRecipt() {}

	virtual void Unregister() = 0;

	inline void* getPtr() const { return ptr; }
	inline void setPtr(void* p) { ptr = p; }


protected:
	void* ptr;
};

template<typename T>
class RegistrationRecipt : public IRecipt
{
public:
	RegistrationRecipt(void* p = nullptr)
	{
		ptr = p;
	}

	~RegistrationRecipt()
	{
		Unregister();
	}

	void Unregister()
	{
		if (ptr != nullptr) EventObserver<T>::Unregister(ptr);
	}
};

//Helps keep track of registrations. Makes cleanup easier.
class RegistrationRecipts
{
public:
	static void Insert(std::unique_ptr< IRecipt >& r)
	{
		auto recipts = getRecipts();
		void* ptr = r->getPtr();
		(*recipts)[ptr].push_back(std::move(r));
	}

	static void RemoveAll(void* ptr)
	{
		auto recipts = getRecipts();
		auto itr = recipts->find(ptr);
		if (itr != recipts->end())
		{
			recipts->erase(itr);
		}
	}

	template<typename T>
	static void Remove(void* ptr)
	{
		auto recipts = getRecipts();
		auto itr = recipts->find(ptr);
		if (itr != recipts->end())
		{
			auto& vector = itr->second;

			auto vitr = std::find_if(vector.begin(), vector.end(),
				[](const auto& r) {
					return dynamic_cast<RegistrationRecipt<T>*>(r.get()) != nullptr; //gross
			});

			if (vitr != vector.end())
			{
				auto last = vector.end() - 1;
				std::iter_swap(vitr, last);
				vector.pop_back();
				return;
			}
		}
	}

private:
	typedef std::unordered_map<void*, std::vector<std::unique_ptr< IRecipt >>> mapType;
	static mapType* getRecipts()
	{
		static mapType* recipts = nullptr;
		if (recipts == nullptr)
		{
			recipts = new mapType();
		}
		return recipts;
	}
};

//Publish event to all registrants
template<typename T>
inline void PublishEvent(const T& evt)
{
	EventObserver<T>::PublishEvent(evt);
}

//Registers callback function to an event type. Its a good idea to call UnregisterAll when destroying.
template<typename T>
inline void Register(void* ptr, std::function<void(const T&)> clbk)
{
	std::unique_ptr< IRecipt > recipt(new RegistrationRecipt<T>(ptr));
	EventObserver<T>::Register(ptr, std::move(clbk));
	RegistrationRecipts::Insert(recipt);
}

template<typename T>
inline void Unregister(void* ptr)
{
	RegistrationRecipts::Remove<T>(ptr); //should take care of calling EventObserver<T>::Unregister(ptr);
//	EventObserver<T>::Unregister(ptr);
}

//Unregister from every event type. Good idea to call in destructor.
inline void UnregisterAll(void* ptr)
{
	RegistrationRecipts::RemoveAll(ptr);
}
}

#define REGISTER_EVENT_TYPE( type ) \
	std::vector< EventSystem::EventObserver<##type>::registration > EventSystem::EventObserver<##type>::m_registrants;
