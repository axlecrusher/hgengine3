#include <HgEngine.h>
#include <HgEntity.h>
#include <EventSystem.h>

namespace ENGINE
{

GlobalInitalizer::GlobalInitalizer()
{
	HgEntity::Find(1); //force EntityLocator initilization, ewwwww
}

}
