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
    //Instead of pthread based workers, create tbbTask
    //G4int ntasks = numberOfEventToBeProcessed/nEvtsPerTask;
    //G4int remn = numberOfEventToBeProcessed % nEvtsPerTask;
    //for ( G4int nt = 0 ; nt < ntasks ; ++nt )
    //{
    //    G4int evts= nEvtsPerTask;
    //    if ( nt == ntasks - 1 ) evts+=remn;
    //    CreateTask(nt,evts);
    //}
}

void SimpleMasterRunManager::CreateTask(G4int id,G4int evts)
{
    //tbbTask& task = * new(tbb::task::allocate_root())
    //tbbTask( id , nullptr , evts ); //Add output for merging
    //theTasks->push_back( task );


}

void SimpleMasterRunManager::RunTermination()
{
    // Reduce results ....
    G4MTRunManager::RunTermination();
}
