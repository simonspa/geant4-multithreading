#pragma once

#include <G4WorkerRunManager.hh>

// The RunManager that executes on each thread. This is constructed
// on a per thread basis by the Master RunManager.
class SimpleWorkerRunManager : public G4WorkerRunManager {
public:
    virtual ~SimpleWorkerRunManager();

    // TODO: override if needed
    //virtual void DoEventLoop(G4int n_event,const char* macroFile=0,G4int n_select=-1);

    static SimpleWorkerRunManager* GetNewInstanceForThread();

protected:
    SimpleWorkerRunManager();

    // Needed to construct a new Event
    virtual G4Event* GenerateEvent(G4int i_event);
    
    // We have no work from Master to do
    virtual void DoWork() {}

    // We don't need to merge the results
    virtual void MergePartialResults() {}
};
