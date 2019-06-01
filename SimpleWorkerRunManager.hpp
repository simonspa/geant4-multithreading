#pragma once

#include <G4WorkerRunManager.hh>

class SimpleWorkerRunManager : public G4WorkerRunManager {
public:
    SimpleWorkerRunManager();
    virtual ~SimpleWorkerRunManager();

    void StartWorker();

    // TODO: override if needed
    //virtual void DoEventLoop(G4int n_event,const char* macroFile=0,G4int n_select=-1);


protected:
    virtual G4Event* GenerateEvent(G4int i_event);
    virtual void DoWork() {}
    // virtual void ConstructScoringWorlds();
    // virtual void StoreRNGStatus(const G4String& filenamePrefix );
    virtual void MergePartialResults() {}
};
