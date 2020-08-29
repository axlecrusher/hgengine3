#pragma once

#include <memory>
#include <stdint.h>
#include <RenderData.h>
#include <HgGPUBuffer.h>
#include <RenderData.h>

namespace Instancing
{
	struct GPUTransformationMatrix
	{
		float matrix[16];
	};

	/*
	Structure containing the instancing data required for instance rendering.
	This can indicate a subset of the instanceData buffer.
	*/
	struct InstancingMetaData
	{
		InstancingMetaData()
			:instanceCount(0), byteOffset(0), renderData(nullptr)
		{
		}

		InstancingMetaData(std::shared_ptr< IHgGPUBuffer >& dataBuffer)
			:instanceCount(0), byteOffset(0), instanceData(dataBuffer), renderData(nullptr)
		{
		}

		~InstancingMetaData()
		{
		}

		inline bool isValid() const { return instanceCount > 0; }

		//number of instances to render
		uint32_t instanceCount;

		//byte offset into the instanceData buffer where the instancing data begins
		uint32_t byteOffset;

		//RenderDataPtr renderData;
		RenderData* renderData;

		//pointer to the data buffer
		std::shared_ptr< IHgGPUBuffer > instanceData;
		std::shared_ptr< IGPUBuffer > transformMatrices;
	};

}