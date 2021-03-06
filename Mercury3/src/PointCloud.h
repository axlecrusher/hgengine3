#pragma once

#include <HgVbo.h>
#include <InstancedCollection.h>

namespace PointCloud
{

struct gpuStruct {
	float matrix[16];
};

class PointCloud : public IUpdatableInstance<gpuStruct> {
public:
	using InstanceCollection = InstancedCollection<PointCloud, gpuStruct, 1>;
	using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

	virtual void update(HgTime tdelta);
	virtual void getInstanceData(gpuStruct* instanceData);

	void buildFromVertexData(const HgVboRecord& vborec);
	void buildFromVertexData(const vbo_layout_vnut* data, const uint32_t vertexCount);

	void init();
private:

	//vertices for drawing a line.
	std::vector<vbo_layout_vc> m_points;
};

}