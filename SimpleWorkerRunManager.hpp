#pragma once

#include <G4WorkerRunManager.hh>

class SimpleMasterRunManager;

// The RunManager that executes on each thread. This is constructed
// on a per thread basis by the Master RunManager.
class SimpleWorkerRunManager : public G4WorkerRunManager {
    friend class SimpleMasterRunManager;
public:
    virtual ~SimpleWorkerRunManager();

    virtual void BeamOn(G4int n_event,const char* macroFile=0,G4int n_select=-1) override;

    // Factory method to create and correctly initialize a new worker
    static SimpleWorkerRunManager* GetNewInstanceForThread();

protected:
    SimpleWorkerRunManager();

    // Needed to construct a new Event
    virtual G4Event* GenerateEvent(G4int i_event) override;

    // Everything is the same except that we keep the seedsQueue as it is since master
    // has already pushed the seeds in it
    virtual void DoEventLoop(G4int n_event,const char* macroFile=0,G4int n_select=-1) override;
    
    // We have no work from Master to do
    virtual void DoWork() override {}

    // We don't need to merge the results
    virtual void MergePartialResults() override {}
};
