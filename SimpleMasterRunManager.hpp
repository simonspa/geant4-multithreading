#pragma once

#include <G4MTRunManager.hh>

class SimpleWorkerRunManager;

class SimpleMasterRunManager : public G4MTRunManager {
    friend class SimpleWorkerRunManager;
public:
    SimpleMasterRunManager();
    virtual ~SimpleMasterRunManager();


    void InitializeForCustomThreads(G4int n_event) {
        // Hacky ways of coding...
        G4MTRunManager::InitializeEventLoop(n_event, nullptr, 0);
    }


protected:

    virtual G4bool SetUpAnEvent(G4Event*, long& s1, long& s2, long& s3, G4bool reseedRequired=true) override;

    virtual G4int SetUpNEvents(G4Event*, G4SeedsQueue*, G4bool reseedRequired=true) override {
        (void)reseedRequired;
        return 0;
    }
    virtual void RequestWorkersProcessCommandsStack() override {}
    virtual void ThisWorkerProcessCommandsStackDone() override {}
    virtual void CreateAndStartWorkers() override {
        // Master creates threads and start Workers
        // Nothing to do here...
    }

    virtual void TerminateWorkers() override {
        // Master terminate Workers
        // Nothing to do here...
    }

    virtual void WaitForReadyWorkers() override {
        // Master used to wait here to synchronize with Workers
        // Nothing to do here...
    }

    virtual void WaitForEndEventLoopWorkers() override {
        // Master used to wait here for workers to finish
        // Nothing to do here...
    }

    virtual void ThisWorkerReady() override {
        // Workers used to wait here to synchronize with Master
        // Nothing to do here...
    }

    virtual void ThisWorkerEndEventLoop() override {
        // Worker tell Master they finished.
        // Nothing to do here...
    }

    virtual WorkerActionRequest ThisWorkerWaitForNextAction() override {
        // Workers used to wait here until Master tell them what to do
        // Nothing to do here...
        return WorkerActionRequest::UNDEFINED;
    }

    virtual void NewActionRequest( WorkerActionRequest ) override {
        // Issue a new command to workers.
        // Nothing to do here...
    }
};
