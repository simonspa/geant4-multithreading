#include "SimpleWorkerRunManager.hpp"
#include <G4Run.hh>
#include <G4MTRunManager.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4UserRunAction.hh>
#include <G4SDManager.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <G4Threading.hh>
#include <G4String.hh>
#include <G4WorkerRunManager.hh>
#include <G4MTRunManager.hh>
#include <G4UImanager.hh>
#include <G4UserWorkerThreadInitialization.hh>
#include <G4WorkerThread.hh>
#include <G4MTRunManagerKernel.hh>
#include <G4AutoLock.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4VUserActionInitialization.hh>

#include <atomic>

static std::atomic<int> counter;

SimpleWorkerRunManager::SimpleWorkerRunManager() :
    G4WorkerRunManager()
{
}

SimpleWorkerRunManager::~SimpleWorkerRunManager()
{
    G4MTRunManager* master_run_manager = G4MTRunManager::GetMasterRunManager();

    //===============================
    //Step-6: Terminate worker thread
    //===============================
    if(master_run_manager->GetUserWorkerInitialization()) { 
        master_run_manager->GetUserWorkerInitialization()->WorkerStop();
    }

    //===============================
    //Step-7: Cleanup split classes
    //===============================
    // TODO: crashes! is it needed anyways?
    //G4WorkerThread::DestroyGeometryAndPhysicsVector();
}

void SimpleWorkerRunManager::BeamOn(G4int n_event,const char* macroFile,G4int n_select)
{
    G4MTRunManager* mrm = G4MTRunManager::GetMasterRunManager();

    // G4Comment: The following code deals with changing materials between runs
    // While we don't really change materials between runs, but this is here
    // for completeness.
    static G4ThreadLocal G4bool skipInitialization = true;
    if(skipInitialization)
    {
        // G4Comment: re-initialization is not necessary for the first run
        skipInitialization = false;
    }
    else
    {
        G4WorkerThread::UpdateGeometryAndPhysicsVectorFromMaster();
    }

    // G4Comment: Execute UI commands stored in the master UI manager
    std::vector<G4String> cmds = mrm->GetCommandStack();
    G4UImanager* uimgr = G4UImanager::GetUIpointer(); //TLS instance
    std::vector<G4String>::const_iterator it = cmds.begin();
    for(;it!=cmds.end();it++)
    { uimgr->ApplyCommand(*it); }

    G4RunManager::BeamOn(n_event, macroFile, n_select);
}

G4Event* SimpleWorkerRunManager::GenerateEvent(G4int i_event)
{
    (void)i_event;
    if(!userPrimaryGeneratorAction)
    {
        G4Exception("SimpleWorkerRunManager::GenerateEvent()", "Run0032", FatalException,
                "G4VUserPrimaryGeneratorAction is not defined!");
        return 0;
    }

    G4Event* anEvent = nullptr;
    long s1, s2, s3;
    s1 = s2 = s3 = 0;

    if( numberOfEventProcessed < numberOfEventToBeProcessed && !runAborted ) {
        anEvent  = new G4Event(numberOfEventProcessed);

        // Seeds are stored in this queue to ensure we can reproduce the results of events
        // each event will reseed the random number generator
        s1 = seedsQueue.front(); seedsQueue.pop();
        s2 = seedsQueue.front(); seedsQueue.pop();
        if (seedsQueue.size() > 0) {
            G4Exception("SimpleWorkerRunManager::GenerateEvent()", "Run0032", FatalException,
            "SeedsQueue should be empty!");
        }

        // seed RNG for this event run
        long seeds[3] = { s1, s2, 0 };
        G4Random::setTheSeeds(seeds,-1);
        runIsSeeded = true;

        userPrimaryGeneratorAction->GeneratePrimaries(anEvent);
    } else {
        // This flag must be set so the event loop exits if no more events
        // to be processed
        eventLoopOnGoing = false;
    }

  return anEvent;
}

void SimpleWorkerRunManager::DoEventLoop(G4int n_event,const char* macroFile,G4int n_select)
{
    if(!userPrimaryGeneratorAction)
    {
      G4Exception("SimpleWorkerRunManager::GenerateEvent()", "Run0032", FatalException,
                "G4VUserPrimaryGeneratorAction is not defined!");
    }

    //This is the same as in the sequential case, just the for-loop indexes are
    //different
    InitializeEventLoop(n_event,macroFile,n_select);

    runIsSeeded = true; 

    // Event loop
    eventLoopOnGoing = true;
    G4int i_event = -1;
    nevModulo = -1;
    currEvID = -1;

    while(eventLoopOnGoing)
    {
      ProcessOneEvent(i_event);
      if(eventLoopOnGoing)
      {
        TerminateOneEvent();
        if(runAborted)
        { eventLoopOnGoing = false; }
      }
    }
     
    TerminateEventLoop();
}

SimpleWorkerRunManager* SimpleWorkerRunManager::GetNewInstanceForThread()
{
    SimpleWorkerRunManager* thread_run_manager = nullptr;
    G4MTRunManager* master_run_manager = G4MTRunManager::GetMasterRunManager();

    //============================
    //Step-0: Thread ID
    //============================
    //Initliazie per-thread stream-output
    //The following line is needed before we actually do I/O initialization
    //because the constructor of UI manager resets the I/O destination.
    G4int thisId = counter.fetch_add(1);
    G4Threading::G4SetThreadId( thisId );
    G4UImanager::GetUIpointer()->SetUpForAThread( thisId );

    //============================
    //Step-1: Random number engine
    //============================
    //RNG Engine needs to be initialized by "cloning" the master one.
    const CLHEP::HepRandomEngine* masterEngine = master_run_manager->getMasterRandomEngine();
    master_run_manager->GetUserWorkerThreadInitialization()->SetupRNGEngine(masterEngine);

    //============================
    //Step-2: Initialize worker thread
    //============================
    if(master_run_manager->GetUserWorkerInitialization())
    master_run_manager->GetUserWorkerInitialization()->WorkerInitialize();
    if(master_run_manager->GetUserActionInitialization()) {
        G4VSteppingVerbose* sv =
            master_run_manager->GetUserActionInitialization()->InitializeSteppingVerbose();
        if (sv) G4VSteppingVerbose::SetInstance(sv);
    }
    //Now initialize worker part of shared objects (geometry/physics)
    G4WorkerThread::BuildGeometryAndPhysicsVector();
    thread_run_manager = new SimpleWorkerRunManager;

    //================================
    //Step-3: Setup worker run manager
    //================================
    // Set the detector and physics list to the worker thread. Share with master
    const G4VUserDetectorConstruction* detector = 
        master_run_manager->GetUserDetectorConstruction();

    thread_run_manager->G4RunManager::SetUserInitialization(
        const_cast<G4VUserDetectorConstruction*>(detector));

    const G4VUserPhysicsList* physicslist = master_run_manager->GetUserPhysicsList();
    thread_run_manager->SetUserInitialization(const_cast<G4VUserPhysicsList*>(physicslist));

    //================================
    //Step-4: Initialize worker run manager
    //================================
    if(master_run_manager->GetUserActionInitialization())
    { master_run_manager->GetNonConstUserActionInitialization()->Build(); }
    if(master_run_manager->GetUserWorkerInitialization())
    { master_run_manager->GetUserWorkerInitialization()->WorkerStart(); }

    thread_run_manager->Initialize();

    // Execute UI commands stored in the masther UI manager
    std::vector<G4String> cmds = master_run_manager->GetCommandStack();
    G4UImanager* uimgr = G4UImanager::GetUIpointer(); //TLS instance
    std::vector<G4String>::const_iterator it = cmds.begin();
    for(;it!=cmds.end();it++)
    { 
        G4cout << *it << G4endl;
        uimgr->ApplyCommand(*it);
    }

    return thread_run_manager;
}
