#pragma once

#include <G4MTRunManager.hh>

class SimpleWorkerRunManager;

// A custom RunManager for Geant4 to replace the G4RunManager.
// Doesn't manage an event loop or threads.
class SimpleMasterRunManager : public G4MTRunManager {
    friend class SimpleWorkerRunManager;
public:
    SimpleMasterRunManager();
    virtual ~SimpleMasterRunManager();

    // Must be called before running.
    void InitializeForCustomThreads(G4int) {
        // This is needed to draw random seeds and fill the internal seed array
        // use nSeedsMax to fill as much as possible now and hopefully avoid
        // refilling later
        G4MTRunManager::InitializeEventLoop(nSeedsMax, nullptr, 0);
    }

    // Wrapper around BeamOn. It doesn't actually call BeamOn of this manager
    // but rather of the thread specific manager managed internall by this 
    // object.
    void Run(G4int n_event);

    // Must be called by each custom thread that ever called the Run method
    // to clean thread local stuff
    void CleanUpWorker();
protected:
    // Original G4MTRunManager API

    virtual WorkerActionRequest ThisWorkerWaitForNextAction() override {
        return WorkerActionRequest::UNDEFINED;
    }
    virtual void CreateAndStartWorkers() override {}
    virtual void NewActionRequest( WorkerActionRequest ) override {}
    virtual void RequestWorkersProcessCommandsStack() override {}

    // Reimplemented to seed events as needed by each thread worker
    virtual G4bool SetUpAnEvent(G4Event*, long& s1, long& s2, long& s3, G4bool reseedRequired=true) override;

    // Not needed, since most of the time we run BeamOn(1), so we just setup an event at a time
    virtual G4int SetUpNEvents(G4Event*, G4SeedsQueue*, G4bool) override {
        return 0;
    }

    virtual void TerminateWorkers() override {}
    virtual void ThisWorkerEndEventLoop() override {}
    virtual void ThisWorkerProcessCommandsStackDone() override {}
    virtual void ThisWorkerReady() override {}
    virtual void WaitForEndEventLoopWorkers() override {}
    virtual void WaitForReadyWorkers() override {}
private:
    static G4ThreadLocal SimpleWorkerRunManager* worker_run_manager_; 
};
