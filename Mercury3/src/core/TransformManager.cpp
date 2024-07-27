#pragma once

#include <TransformManager.h>

const Transform Transform::Identity;

Transform::Transform()
{
	memset(matrix, 0.0f, sizeof(float) * 16);
	matrix[0] = 1.0;
	matrix[5] = 1.0;
	matrix[10] = 1.0;
	matrix[15] = 1.0;
}

int32_t TransformManager::find(EntityIdType eid) const
{
	int32_t idx = NOT_FOUND();
	const auto itr = m_indices.find(eid);
	if (itr != m_indices.end())
	{
		idx = itr->second;
	}

	return idx;
}

void TransformManager::insert(EntityIdType eid, const Transform& t)
{
	auto idx = find(eid);
	if (idx == NOT_FOUND())
	{
		idx = (int32_t)m_matrices.size();
		m_matrices.push_back(t);
		m_ids.push_back(eid);
		m_indices[eid] = idx;
	}
	else
	{
		m_matrices[idx] = t;
	}
}

void TransformManager::remove(EntityIdType eid)
{
	auto idx = find(eid);
	if (idx != NOT_FOUND())
	{
		m_indices.erase(eid);
		const auto otherEntityId = m_ids.back(); //entity ID of the transform we will be moving

		m_matrices[idx] = m_matrices.back(); //replace with last transform
		m_ids[idx] = m_ids.back(); //replace with last id

		m_matrices.pop_back();
		m_ids.pop_back();

		m_indices[otherEntityId] = idx; //update index record for transform we just moved
	}
}

const Transform& TransformManager::get(EntityIdType eid) const
{
	auto idx = find(eid);
	if (idx != NOT_FOUND())
	{
		return m_matrices[idx];
	}
	return Transform::Identity;
}