#pragma once

#include <Logging.h>
#include <HgScene2.h>
#include <HgEngine.h>
#include <Logging.h>

namespace Engine {

std::unordered_map<std::string, factoryCallback> HgScene::m_entityFactories;

HgScene::HgScene()
{
	auto vBuffer = std::make_shared<HgVectorBuffer<Instancing::GPUTransformationMatrix>>();

	std::unique_ptr<IGLBufferUse> action = std::make_unique<MatrixVertexAttribute>("ModelMatrix");
	vBuffer->setUseClass(action);

	m_vBuffer = vBuffer;
}

HgEntity* HgScene::create_entity(const char* type_str)
{
	auto factory = m_entityFactories.find(type_str);

	if (factory == m_entityFactories.end()) {
		LOG_ERROR("Unable to find entity type \"%s\"", type_str);
		return nullptr;
	}
	factoryCallback clbk = factory->second;
	HgEntity* entity = clbk(this);

	return entity;
}

void HgScene::RegisterEntityFactory(const char* str, Engine::factoryCallback clbk)
{
	m_entityFactories[str] = clbk;
}

void HgScene::update(HgTime dtime)
{
	for (auto& i : m_collections) {
		i->update(dtime);
	}

	RemoveInvalidEntities();
}

void HgScene::RemoveInvalidEntities()
{
	m_tmpEntities.clear();
	if (m_tmpEntities.capacity() < m_entities.size())
	{
		m_tmpEntities.reserve(m_entities.size());
	}

	auto& entityTable = EntityIdTable::Singleton();

	for (auto id : m_entities)
	{
		if (entityTable.exists(id))
		{
			m_tmpEntities.push_back(id);
		}
		//else
		//{
		//	LOG("Removing entity %d", id);
		//}
	}
	std::swap(m_entities, m_tmpEntities);
}

struct RdDrawOrder
{
	RdDrawOrder()
		:drawOrder(0)
	{}

	EntityRDPair rdPair;
	int8_t drawOrder;

	inline bool isSameGroup(const RdDrawOrder& rhs)
	{
		return (drawOrder == rhs.drawOrder)
			&& (rdPair.ptr == rhs.rdPair.ptr);
	}
};

struct {
	bool operator()(const EntityRDPair& a, const EntityRDPair& b) const
	{
		//sort to ascending entity id
		if (a.ptr == b.ptr)
		{
			return a.entity < b.entity;
		}
		return a.ptr < b.ptr;
	}
} orderByRenderData;

struct {
	bool operator()(const RdDrawOrder& a, const RdDrawOrder& b) const
	{
		return (a.drawOrder < b.drawOrder) ||
		((a.drawOrder == b.drawOrder) && (a.rdPair.ptr < b.rdPair.ptr)) ||
		((a.drawOrder == b.drawOrder) && (a.rdPair.ptr == b.rdPair.ptr) && (a.rdPair.ptr < b.rdPair.ptr));

	//if (a.drawOrder == b.drawOrder)
	//	{
	//		//sort to ascending entity id
	//		if (a.rdPair.ptr == b.rdPair.ptr)
	//		{
	//			return a.rdPair.entity < b.rdPair.entity;
	//		}
	//		return a.rdPair.ptr < b.rdPair.ptr;
	//	}
	//	return a.drawOrder < b.drawOrder;
	}
} orderEntitesForDraw;

void HgScene::EnqueueForRender(RenderQueue* queue, HgTime dt) {

	//This function does not scale well with huge entity counts

	for (auto& i : m_collections) {
		i->EnqueueForRender(queue, dt);
	}

	if (m_entities.empty()) return;

	auto renderDatas = RenderDataTable::Manager().getRenderDataForEntities(m_entities.data(), m_entities.size());
	
	std::vector<RdDrawOrder> rdoList;
	rdoList.reserve(renderDatas.size());

	auto& entityTable = EntityTable::Singleton();
	auto& entityIdTable = EntityIdTable::Singleton();

	for (auto& rdp : renderDatas)
	{
		RdDrawOrder t;
		t.rdPair = { rdp.entity, rdp.ptr };

		auto entity = entityTable.getPtr(&entityIdTable, rdp.entity);

		t.drawOrder = entity->getDrawOrder();

		if (!entity->getFlags().hidden)
		{
			rdoList.push_back(t);
		}
		//else
		//{
		//	LOG("Entity hidden, skip");
		//}
	}

	m_modelMatrices.resize(rdoList.size()); //always resize to send the smallest size to the gpu

	std::vector< Instancing::InstancingMetaData > instances;

	m_vBuffer->setDataSource(m_modelMatrices);
	m_vBuffer->setNeedsLoadToGPU(true); //entire vector contents needs to be sent to the GPU

	//sort by draworder, renderdata, entityID
	std::sort(rdoList.begin(), rdoList.end(), orderEntitesForDraw);

	//group by draworder, renderdata
	RdDrawOrder lastRDO;
	Instancing::InstancingMetaData imd;
	uint32_t matrixOffset = 0;
	for (const auto& t : rdoList)
	{
		if (!lastRDO.isSameGroup(t))
		{
			if (imd.instanceCount > 0)
			{
				//push previous instance meta data into instances
				instances.push_back(imd);
			}
			imd = Instancing::InstancingMetaData();
			imd.byteOffset = sizeof(Instancing::GPUTransformationMatrix) * matrixOffset;
			imd.renderData = t.rdPair.ptr;
			imd.instanceData = m_vBuffer;
			lastRDO = t;
		}

		auto entity = entityTable.getPtr(&entityIdTable, t.rdPair.entity);
		const auto m = entity->computeWorldSpaceMatrix();

		if (matrixOffset < m_modelMatrices.size())
		{
			m.store(m_modelMatrices[matrixOffset].matrix);
		}
		else
		{
			LOG_ERROR("matrixOffset >= m_modelMatrices.size: %d < %d\n", matrixOffset, m_modelMatrices.size());
		}

		imd.instanceCount++;
		matrixOffset++;
	}

	if (imd.instanceCount > 0)
	{
		instances.push_back(imd);
	}

	for (const auto& i : instances)
	{
		queue->Enqueue(i);
	}
}

} //Engine