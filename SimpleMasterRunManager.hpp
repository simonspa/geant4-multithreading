#pragma once

#include <G4MTRunManager.hh>

class SimpleMasterRunManager : public G4MTRunManager {
public:
    SimpleMasterRunManager(); // G4RunManagerKernel* existingRMK=0);
    // Default constructor, 
    //  tasklist is the tbb::task_list to which the created tasks will be added.
    //  nEvents is the number of events for which each tbb::task is responsible
    //
    virtual ~SimpleMasterRunManager();
    virtual void RunTermination();
   // void SetTaskList( tbb::task_list* tl ) { theTasks = tl; }
    //Set a reference to the output task list where new tasks will
    //be added to
   // void SetNumberEventsPerTask( G4int nt ) { nEvtsPerTask = nt; }
    //Specify number of events that each simulation task is responsible
    //for
protected:
    virtual void CreateAndStartWorkers();
    virtual void TerminateWorkers();
    virtual void CreateTask(G4int id,G4int evts);
    //Creates a concrete tbb::task with index id
    //responsible for evts events
protected:
    //Barriers mechanism for TBB is non existing
    virtual void WaitForReadyWorkers() {}
    virtual void WaitForEndEventLoopWorkers() {}
    virtual void ThisWorkerReady() {}
    virtual void ThisWorkerEndEventLoop() {}
    virtual WorkerActionRequest ThisWorkerWaitForNextAction()
      { return WorkerActionRequest::UNDEFINED; }
    virtual void NewActionRequest( WorkerActionRequest /*newRequest*/ ) {}
private:
    //tbb::task_list* theTasks;
    //G4int nEvtsPerTask;
};
