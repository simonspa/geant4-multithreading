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
    G4cout << "SimpleWorker DTOR!" << G4endl;

    G4MTRunManager* masterRM = G4MTRunManager::GetMasterRunManager();

    //===============================
    //Step-6: Terminate worker thread
    //===============================
    if(masterRM->GetUserWorkerInitialization())
    { masterRM->GetUserWorkerInitialization()->WorkerStop(); }

    //===============================
    //Step-7: Cleanup split classes
    //===============================
    //G4WorkerThread::DestroyGeometryAndPhysicsVector();
}


G4Event* SimpleWorkerRunManager::GenerateEvent(G4int i_event)
{
    
    if(!userPrimaryGeneratorAction)
    {
        G4Exception("G4RunManager::GenerateEvent()", "Run0032", FatalException,
                "G4VUserPrimaryGeneratorAction is not defined!");
        return 0;
    }

    G4Event* anEvent = nullptr;
    long s1, s2, s3;
    s1 = s2 = s3 = 0;

    if( numberOfEventProcessed < numberOfEventToBeProcessed && !runAborted ) {
        anEvent  = new G4Event(numberOfEventProcessed);
        
        // must ask master to seed the event to ensure event reproducability.
        eventLoopOnGoing = G4MTRunManager::GetMasterRunManager()
                       ->SetUpAnEvent(anEvent,s1,s2,s3,true);

        // seed RNG for this event run
        long seeds[3] = { s1, s2, 0 };
        G4Random::setTheSeeds(seeds,-1);
        // runIsSeeded = true;
            
        //if(printModulo > 0 && anEvent->GetEventID()%printModulo == 0 )
            G4cout << "--> Event " << anEvent->GetEventID() << " starts";
        //  { G4cout << " with initial seeds (" << s1 << "," << s2 << ")"; }
        //  G4cout << "." << G4endl;

        userPrimaryGeneratorAction->GeneratePrimaries(anEvent);
    } else {
        // This flag must be set so the event loop exits if no more events
        // to be processed
        eventLoopOnGoing = false;
    }

  return anEvent;
}

SimpleWorkerRunManager* SimpleWorkerRunManager::GetNewInstanceForThread()
{
    SimpleWorkerRunManager* localRM = nullptr;
    G4MTRunManager* masterRM = G4MTRunManager::GetMasterRunManager();

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
    const CLHEP::HepRandomEngine* masterEngine = masterRM->getMasterRandomEngine();
    masterRM->GetUserWorkerThreadInitialization()->SetupRNGEngine(masterEngine);

    //============================
    //Step-2: Initialize worker thread
    //============================
    if(masterRM->GetUserWorkerInitialization())
    masterRM->GetUserWorkerInitialization()->WorkerInitialize();
    if(masterRM->GetUserActionInitialization()) {
        G4VSteppingVerbose* sv =
            masterRM->GetUserActionInitialization()->InitializeSteppingVerbose();
        if (sv) G4VSteppingVerbose::SetInstance(sv);
    }
    //Now initialize worker part of shared objects (geometry/physics)
    G4WorkerThread::BuildGeometryAndPhysicsVector();
    localRM = new SimpleWorkerRunManager;

    //================================
    //Step-3: Setup worker run manager
    //================================
    // Set the detector and physics list to the worker thread. Share with master
    const G4VUserDetectorConstruction* detector = 
        masterRM->GetUserDetectorConstruction();

    localRM->G4RunManager::SetUserInitialization(
        const_cast<G4VUserDetectorConstruction*>(detector));

    const G4VUserPhysicsList* physicslist = masterRM->GetUserPhysicsList();
    localRM->SetUserInitialization(const_cast<G4VUserPhysicsList*>(physicslist));

    //================================
    //Step-4: Initialize worker run manager
    //================================
    if(masterRM->GetUserActionInitialization())
    { masterRM->GetNonConstUserActionInitialization()->Build(); }
    if(masterRM->GetUserWorkerInitialization())
    { masterRM->GetUserWorkerInitialization()->WorkerStart(); }

    localRM->Initialize();

    // Execute UI commands stored in the masther UI manager
    std::vector<G4String> cmds = masterRM->GetCommandStack();
    G4UImanager* uimgr = G4UImanager::GetUIpointer(); //TLS instance
    std::vector<G4String>::const_iterator it = cmds.begin();
    for(;it!=cmds.end();it++)
    { 
        G4cout << *it << G4endl;
        uimgr->ApplyCommand(*it);
    }

    return localRM;
}
