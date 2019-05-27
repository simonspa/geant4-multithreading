#include "SimpleMasterRunManager.hpp"

SimpleMasterRunManager::SimpleMasterRunManager() : 
    G4MTRunManager()
    //theTasks(static_cast<tbb::task_list*>(nullptr)),
    //nEvtsPerTask(1)
{
}

SimpleMasterRunManager::~SimpleMasterRunManager()
{
}

void SimpleMasterRunManager::TerminateWorkers()
{
    //For TBB based example this should be empty
}

void SimpleMasterRunManager::CreateAndStartWorkers()
{
}

void SimpleMasterRunManager::CreateTask(G4int id,G4int evts)
{
}

void SimpleMasterRunManager::RunTermination()
{
    // Reduce results ....
    G4MTRunManager::RunTermination();
}
