#pragma once

#include <G4MTRunManager.hh>

class SimpleWorkerRunManager;

// A custom RunManager for Geant4 to replace the G4RunManager.
// Doesn't manage an event loop or threads. It only initialize geometry
// and state and share the workspace with workers.
// Workers are allocated on a per thread basis on demand.
class SimpleMasterRunManager : public G4MTRunManager {
    friend class SimpleWorkerRunManager;
public:
    SimpleMasterRunManager();
    virtual ~SimpleMasterRunManager();

    // Reimplemented to initialize the event loop with max number of events
    virtual void Initialize() override;

    // Wrapper around BeamOn. It doesn't actually call BeamOn of this manager
    // but rather of the thread specific manager managed internall by this 
    // object.
    void Run(G4int i_event, G4int n_event);

    // Must be called by each custom thread that ever called the Run method
    // to clean thread local stuff
    void CleanUpWorker();
protected:
    // Original G4MTRunManager API
    // All

    virtual WorkerActionRequest ThisWorkerWaitForNextAction() override {
        return WorkerActionRequest::UNDEFINED;
    }
    virtual void CreateAndStartWorkers() override {}
    virtual void NewActionRequest( WorkerActionRequest ) override {}
    virtual void RequestWorkersProcessCommandsStack() override {}

    // Not needed, since worker are initialized at the start of their run by this manager
    virtual G4bool SetUpAnEvent(G4Event*, long&, long&, long&, G4bool) override {
        return false;
    }

    // Not needed, since worker are initialized at the start of their run by this manager
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
    // Worker manager that carry out the actual work. It is allocated
    // on a per thread basis
    static G4ThreadLocal SimpleWorkerRunManager* worker_run_manager_; 
};
