#pragma once

#include <Logging.h>
#include <HgScene2.h>
#include <HgEngine.h>
#include <Logging.h>

namespace Engine {

std::unordered_map<std::string, factoryCallback> HgScene::m_entityFactories;

HgScene::HgScene()
{
	//auto vBuffer = std::make_shared<HgVectorBuffer<Instancing::GPUTransformationMatrix>>();

	//std::unique_ptr<IGLBufferUse> action = std::make_unique<MatrixVertexAttribute>("ModelMatrix");
	//vBuffer->setUseClass(action);

	//m_vBuffer = vBuffer;

	m_tranformMatrix = std::make_unique<MatrixVertexAttribute>("ModelMatrix");
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
		:rdPair(nullptr), drawOrder(0)
	{}

	EntityRDPair* rdPair;
	int8_t drawOrder;

	inline bool isSameGroup(const RdDrawOrder& rhs)
	{
		return (drawOrder == rhs.drawOrder)
			&& (rdPair && rdPair->rd == rhs.rdPair->rd);
	}
};

struct {
	bool operator()(const EntityRDPair& a, const EntityRDPair& b) const
	{
		//sort to ascending entity id
		if (a.rd == b.rd)
		{
			return a.entityId < b.entityId;
		}
		return a.rd < b.rd;
	}
} orderByRenderData;

struct {
	bool operator()(const RdDrawOrder& a, const RdDrawOrder& b) const
	{
		return (a.drawOrder < b.drawOrder) ||
		((a.drawOrder == b.drawOrder) && (a.rdPair->rd < b.rdPair->rd)) ||
		((a.drawOrder == b.drawOrder) && (a.rdPair->rd == b.rdPair->rd) && (a.rdPair->entityId < b.rdPair->entityId));

	//if (a.drawOrder == b.drawOrder)
	//	{
	//		//sort to ascending entity id
	//		if (a.rdPair->rd == b.rdPair->rd)
	//		{
	//			return a.rdPair.entity < b.rdPair.entity;
	//		}
	//		return a.rdPair->rd < b.rdPair->rd;
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

	if (m_renderDatas.size() < m_entities.size())
	{
		m_renderDatas.resize(m_entities.size());
	}

	//RenderData pointer is used for grouping draw calls. We don't really
	//the pointer. We just need to know its the same RenderData across entities.
	//Figure out how to do this without a pointer...

	const auto renderCount = RenderDataTable::Manager().getRenderDataForEntities(m_entities.data(), m_entities.size(), m_renderDatas.data());
	
	std::vector<RdDrawOrder> rdoList;
	rdoList.resize(renderCount);

	auto& entityTable = EntityTable::Singleton();
	auto& entityIdTable = EntityIdTable::Singleton();

	uint32_t rdoCount = 0;
	for (int i = 0; i < renderCount; i++)
	{
		auto& rdp = m_renderDatas[i];
		//auto entity = entityTable.getPtr(&entityIdTable, rdp.entityId);

		RdDrawOrder t;
		t.rdPair = &m_renderDatas[i];

		t.drawOrder = entityTable.getDrawOrder(rdp.entityId);
		const auto flags = entityTable.getFlags(rdp.entityId);

		if (!flags.hidden)
		{
			rdoList[rdoCount] = std::move(t);
			rdoCount++;
		}
		//else
		//{
		//	LOG("Entity hidden, skip");
		//}
	}

	rdoList.resize(rdoCount);

	m_tranformMatrix->AllocateCountOnGPU(rdoCount);

	std::vector< Instancing::InstancingMetaData > instances;

	//sort by draworder, renderdata, entityID
	std::sort(rdoList.begin(), rdoList.end(), orderEntitesForDraw);

	//group by draworder, renderdata
	RdDrawOrder lastRDO;
	Instancing::InstancingMetaData imd;
	uint32_t matrixOffset = 0;

	m_tranformMatrix->SwapBuffer();
	auto mappedMem = m_tranformMatrix->getGPUMemoryPtr();
	float* matrixMem = (float*)mappedMem.ptr;

	EntityIdList batchEntities;
	batchEntities.resize(rdoList.size());

	auto& matrices = getEntityMatrixTable();

	for (const auto& t : rdoList)
	{
		if (!lastRDO.isSameGroup(t))
		{
			if (imd.instanceCount > 0)
			{
				//push previous instance meta data into instances
				instances.push_back(imd);

				auto CopyPerInstanceData = imd.renderData->CopyPerInstanceData;
				if (CopyPerInstanceData)
				{
					CopyPerInstanceData(batchEntities.data(), imd.instanceCount);
				}
			}
			imd = Instancing::InstancingMetaData();
			imd.renderData = t.rdPair->rd;
			//imd.instanceData = m_vBuffer;

			{
				GPUBufferMapSettings matrixBufferSettings;
				matrixBufferSettings.gpuBuffer = (IGPUBuffer*)m_tranformMatrix.get();
				matrixBufferSettings.byteOffset = sizeof(Instancing::GPUTransformationMatrix) * matrixOffset;
				imd.gpuBufferSettings.push_back(matrixBufferSettings);
			}

			for (auto gpuBuffer : imd.renderData->perInstanceVertexAttributes)
			{
				GPUBufferMapSettings bufferSettings;
				bufferSettings.gpuBuffer = gpuBuffer;
				imd.gpuBufferSettings.push_back(bufferSettings);
			}

			lastRDO = t;
		}

		auto entity = entityTable.getPtr(&entityIdTable, t.rdPair->entityId);
		batchEntities[imd.instanceCount] = t.rdPair->entityId;

		//Computing the matrix is the slowest part. How can it be made faster?
		entity->computeWorldSpaceMatrix().store(matrixMem);

		//TODO: Fill per instance vertex attributes here

		imd.instanceCount++;
		matrixOffset++;

		matrixMem += 16; //advance 1 matrix
	}

	if (imd.instanceCount > 0)
	{
		instances.push_back(imd);

		auto CopyPerInstanceData = imd.renderData->CopyPerInstanceData;
		if (CopyPerInstanceData)
		{
			CopyPerInstanceData(batchEntities.data(), imd.instanceCount);
		}
	}

	for (const auto& imd : instances)
	{
		Renderer::prepare(&imd);
		queue->Enqueue(imd);
	}
}

} //Engine