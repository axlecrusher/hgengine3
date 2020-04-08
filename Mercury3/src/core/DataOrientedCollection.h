#pragma once

#include <UpdatableCollection.h>

template<typename ItemType, typename StructOfVectors>
class DataOrientedCollection : public IUpdatableCollection
{
public:
	virtual void update(HgTime dt)
	{
		m_data.update(dt);
	}

	virtual void EnqueueForRender(RenderQueue* queue, HgTime dt)
	{
		return m_data.EnqueueForRender(queue, dt);
	}

	ItemType* newItem()
	{
		return m_data.newItem();
	}
private:

	StructOfVectors m_data;

};