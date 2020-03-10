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

//typedef HgEntity&(*factoryCallback)(HgScene* scene);
typedef HgEntity*(*factoryCallback)(HgScene* scene);

/* TODO: Instancing must use the same render data (mesh, shader, and material).
 Only the transformation matrix should be allowed to change.
 We need to be able to instance the same mesh using different shaders.
 Currently we instance based on class type, forcing all instances to use
 the same shader. This needs to be fixed.



 shaders, vbo, textures?
 */

class HgScene
{
public:

	/*	Attempts to create an object of type type_str.
		Returns nullptr on failure. HgEntity pointer on success.
		Returned pointer is managed, do not delete.
	*/
	HgEntity* create_entity(const char* type_str);

	//Typed entity creation method
	template<typename T>
	T& create_entity() {
		auto collection = getCollectionOf<T::InstanceCollection>();
		return collection->newItem();
	}

	//Typed entity creation method
	template<typename T, typename InstanceCollectionType>
	T& create_entity2() {
		auto collection = getCollectionOf<InstanceCollectionType>();
		return collection->newItem();
	}

	//Typed entity creation method
	template<typename T>
	T& create_entity(std::function<T&(HgScene)> create) {
		return create(this);
	}

	void EnqueueForRender(RenderQueue* queue, HgTime dt);
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

	template<typename T>
	static HgEntity* generate_entity(Engine::HgScene* scene) {
		auto& typedObject = scene->create_entity<T>();
		return &typedObject.getEntity();
	}

	template<typename T, typename CollectionType>
	static HgEntity* generate_entity2(Engine::HgScene* scene) {
		auto& typedObject = scene->create_entity2<T, CollectionType>();
		return &typedObject.getEntity();
	}

private:
	using IUpdatableCollectionPtr = std::shared_ptr<IUpdatableCollection>;
	std::vector<IUpdatableCollectionPtr> m_collections;
	std::unordered_map< std::string, IUpdatableCollectionPtr> m_collectionMap;

	static std::unordered_map<std::string, factoryCallback> m_entityFactories;
};

}