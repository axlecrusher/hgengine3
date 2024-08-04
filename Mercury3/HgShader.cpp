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

	shaderMap[name] = weakPtr;

	return HgShader(shaderImpl);
}

int32_t IShaderImpl::getAttributeLocation(const std::string& name) const
{
	auto itr = m_attribLocations.find(name);
	if (itr == m_attribLocations.end())
	{
		//not found
		return -1;
	}

	return itr->second;
}