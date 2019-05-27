#pragma once

#include <G4WorkerRunManager.hh>

class SimpleWorkerRunManager : public G4WorkerRunManager {
public:
    SimpleWorkerRunManager();
    virtual ~SimpleWorkerRunManager();
    virtual void ConstructScoringWorlds();
    virtual void MergePartialResults();
    virtual G4Event* GenerateEvent(G4int i_event);
};
