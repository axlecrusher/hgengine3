#pragma once

#include <memory>
#include <stdint.h>
#include <vector>
#include <typeinfo>
#include <unordered_map>

class HgEntity;
class RenderQueue;
class IUpdatableCollection;

#include <UpdatableCollection.h>

namespace Engine
{

class HgScene;

typedef HgEntity&(*factoryCallback)(HgScene* scene);

//void RegisterEntityType2(const char* c, Engine::factoryCallback);

class HgScene
{
public:

	bool create_entity(const char* type_str, HgEntity** entity);
	void EnqueueForRender(RenderQueue* queue);
	void update(HgTime dtime);


	template<typename T>
	std::shared_ptr<T> getCollectionOf()
	{
		const auto str = typeid(T).name();
		auto itr = m_collectionMap.find(str);
		if (itr == m_collectionMap.end())
		{
			auto ptr = std::make_shared<T>();
			m_collectionMap[str] = ptr;
			m_collections.push_back(ptr);
			return ptr;
		}

		return std::dynamic_pointer_cast<T>(itr->second);
	}

	static void RegisterEntityFactory(const char* str, Engine::factoryCallback clbk);
private:
	using IUpdatableCollectionPtr = std::shared_ptr<IUpdatableCollection>;
	std::vector<IUpdatableCollectionPtr> m_collections;
	std::unordered_map< std::string, IUpdatableCollectionPtr> m_collectionMap;

	//using factoryCallback = std::function<HgEntity&(HgScene* scene)>;
	static std::unordered_map<std::string, factoryCallback> m_entityFactories;
};

}