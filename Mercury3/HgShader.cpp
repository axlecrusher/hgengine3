#include <HgShader.h>
#include <HgEntity.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <FileWatch.h>
#include <str_utils.h>
#include <stdio.h>
#include <Logging.h>

#include <map>

//#pragma warning(disable:4996)

HgShader::createShaderCallback HgShader::Create = nullptr;

std::mutex m_shaderMapMutex;
std::map<std::string, std::weak_ptr<IShaderImpl>> shaderMap;

static void ShaderFileChanged(std::weak_ptr<IShaderImpl> shader) {
	auto impl = shader.lock();
	if (impl)
	{
		LOG_ERROR("Shader file changed:%s", impl->getVertexPath().c_str());
		impl->load();
	}
	else
	{
		//TODO: disable watching somehow
	}
}

HgShader HgShader::acquire(const HgShader& shader)
{
	return shader; //copy;
}

HgShader HgShader::acquire(const char* vert, const char* frag)
{
	std::string v(vert), f(frag);
	std::string name = v + f;

	std::shared_ptr<IShaderImpl> shaderImpl;

	{
		std::lock_guard<std::mutex> m(m_shaderMapMutex);
		const auto itr = shaderMap.find(name);
		if (itr != shaderMap.end())
		{
			shaderImpl = itr->second.lock();
			if (shaderImpl)
			{
				return HgShader(shaderImpl);
			}
		}

		shaderImpl = HgShader::Create(vert, frag);
		shaderMap[name] = shaderImpl;
	}

	auto shader = shaderImpl.get();
	shader->setFragmentPath(f);
	shader->setVertexPath(v);

	auto weakPtr = shaderImpl;

	WatchFileForChange(frag, [weakPtr]() {
		ShaderFileChanged(weakPtr);
		});

	WatchFileForChange(vert, [weakPtr]() {
		ShaderFileChanged(weakPtr);
	});

	shader->load();

	return HgShader(shaderImpl);
}


int32_t IShaderImpl::getAttributeLocation(attributeNameType name) const
{
	for (uint32_t i = 0; i < m_attribLocations.size(); i++)
	{
		if (m_attribLocations[i].first == name)
		{
			return m_attribLocations[i].second;
		}
	}
	return -1;
}