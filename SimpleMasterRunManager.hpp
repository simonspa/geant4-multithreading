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
};
