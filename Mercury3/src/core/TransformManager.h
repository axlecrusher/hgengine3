#pragma once

#include <memory>
#include <stdint.h>
#include <unordered_map>

#include <EntityIdType.h>

struct Transform
{
	Transform();
	//Transform(const Transform&) = default;

	static const Transform Identity;
	float matrix[16];
};

class TransformManager
{
public:
	static inline int32_t NOT_FOUND() { return std::numeric_limits<int32_t>::max(); }

	void insert(EntityIdType, const Transform& tm);
	void remove(EntityIdType);
	const Transform& get(EntityIdType) const;
private:
	int32_t find(EntityIdType) const;

	std::unordered_map<EntityIdType, int32_t> m_indices;
	std::vector<Transform> m_matrices;
	std::vector<EntityIdType> m_ids;
};
