#include <HgGPUBuffer.h>

#include <RenderBackend.h>
#include "OGL/OGLGpuBuffer.h"

void IHgGPUBuffer::sendToGPU()
{
	m_bufferUse->SendToGPU(this);
	setNeedsLoadToGPU(false);
}

//std::unique_ptr<IGPUBufferImpl> IHgGPUBuffer::from_api_type() {
//	switch (RENDERER()->Type()) {
//	case OPENGL:
//		return std::move( std::make_unique<OGLHgGPUBuffer >());
//		break;
//	default:
//		return nullptr;
//	}
//}
