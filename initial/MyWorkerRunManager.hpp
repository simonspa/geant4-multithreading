#pragma once

#include <G4RNGHelper.hh>
#include <G4RunManager.hh>

class G4WorkerThread;
class G4WorkerRunManagerKernel;

class MyWorkerRunManager : public G4RunManager {
public:
    static MyWorkerRunManager* GetWorkerRunManager();
    static G4WorkerRunManagerKernel* GetWorkerRunManagerKernel();
    MyWorkerRunManager();
    ~MyWorkerRunManager();
    //Modified for worker behavior
    ////////virtual void BeamOn(G4int n_event,const char* macroFile=0,G4int n_select=-1);
    virtual void InitializeGeometry() override;
    virtual void InitializePhysics() override;
    virtual void RunInitialization() override;
    virtual void DoEventLoop(G4int n_event,const char* macroFile=0,G4int n_select=-1) override;
    virtual void ProcessOneEvent(G4int i_event) override;
    virtual G4Event* GenerateEvent(G4int i_event) override;
    //G4int NewCommands( const std::vector<G4String>& newCmdsToExecute , G4String& currentCmd );
    //Called by the MTRunManager when new UI commands are to be executed.
    //It is not assumed method is not thread-safe: i.e. should be called sequentially
    //Returns 0 if commands are executed corrected, otherwise returns error code (see G4UImanager::ApplyCommand)
    //In case of error currentCmd is set to the command that gave the problem
    virtual void RunTermination() override;
    virtual void TerminateEventLoop() override;

    //This function is called by the thread function: it should loop until some
    //work is requested
    virtual void DoWork();
protected:
    virtual void ConstructScoringWorlds();
    virtual void StoreRNGStatus(const G4String& filenamePrefix );
    virtual void MergePartialResults();
    //This method will merge (reduce) the results of this run into the
    //global run
public:
    //! Sets the worker context
        void SetWorkerThread( G4WorkerThread* wc ) { workerContext = wc; }
private:
    G4WorkerThread* workerContext;
    void SetupDefaultRNGEngine();

public:
    virtual void SetUserInitialization(G4VUserPhysicsList* userInit);
    virtual void SetUserInitialization(G4VUserDetectorConstruction* userInit);
    virtual void SetUserInitialization(G4VUserActionInitialization* userInit);
    virtual void SetUserInitialization(G4UserWorkerInitialization* userInit);
    virtual void SetUserInitialization(G4UserWorkerThreadInitialization* userInit);
    virtual void SetUserAction(G4UserRunAction* userAction);
    virtual void SetUserAction(G4VUserPrimaryGeneratorAction* userAction);
    virtual void SetUserAction(G4UserEventAction* userAction);
    virtual void SetUserAction(G4UserStackingAction* userAction);
    virtual void SetUserAction(G4UserTrackingAction* userAction);
    virtual void SetUserAction(G4UserSteppingAction* userAction);

protected:
    G4bool eventLoopOnGoing;
    G4bool runIsSeeded;
    G4int nevModulo;
    G4int currEvID;
    G4SeedsQueue seedsQueue;
    G4bool readStatusFromFile;

public:
    virtual void RestoreRndmEachEvent(G4bool flag) { readStatusFromFile = flag; }
private:
    G4bool visIsSetUp;
};

