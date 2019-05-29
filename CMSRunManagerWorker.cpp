#include "CMSRunManagerWorker.hpp"

#include <G4Event.hh>
#include <G4Run.hh>
#include <G4SystemOfUnits.hh>
#include <G4Threading.hh>
#include <G4UImanager.hh>
#include <G4WorkerThread.hh>
#include <G4WorkerRunManagerKernel.hh>
#include <G4StateManager.hh>
#include <G4TransportationManager.hh>
#include <G4VUserPhysicsList.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <atomic>
#include <thread>
#include <sstream>
#include <vector>

using namespace std;

namespace {
    std::atomic<int> thread_counter{0};
    int get_new_thread_index() { return thread_counter++; }
    thread_local int s_thread_index = get_new_thread_index();
    int getThreadIndex() { return s_thread_index; }
}

thread_local std::unique_ptr<G4RunManagerKernel> CMSRunManagerWorker:: kernel_ = nullptr;
thread_local bool CMSRunManagerWorker:: thread_initialized_ = false;
thread_local bool CMSRunManagerWorker:: run_terminated_ = false;

CMSRunManagerWorker::CMSRunManagerWorker(CMSRunManager& run_manager)
    : run_manager_master_(run_manager)
{
    
}

CMSRunManagerWorker::~CMSRunManagerWorker()
{
    // TODO: clear state
}

void CMSRunManagerWorker::Initialize()
{
    int this_id = getThreadIndex();
    G4cout << "Initializing for thread " << this_id << G4endl;

    // initialize per thread output
    G4Threading::G4SetThreadId(this_id);
    G4UImanager::GetUIpointer()->SetUpForAThread(this_id);

    // Initialize worker part of shared resources (geometry, physics)
    G4WorkerThread::BuildGeometryAndPhysicsVector();

    // create worker run manager
    kernel_.reset(G4WorkerRunManagerKernel::GetRunManagerKernel());
    if (kernel_) {
        kernel_.reset(new G4WorkerRunManagerKernel());
    }

    // define G4 exception handler
    //G4StateManager::GetStateManager()->SetExceptionHandler(new ExceptionHandler());

    // TODO: set the geometry for the worker, share from master
    //DDDWorld::WorkerSetAsWorld(runManagerMaster.world().GetWorldVolumeForWorker());

    // TODO: setup the magnetic field

    // set the physics list for the worker, share from master
    G4VUserPhysicsList* physicsList = run_manager_master_.physics_list_;

    // Geant4 UI commands in PreInit state
    // TODO:

    G4StateManager::GetStateManager()->SetNewState(G4State_Init);

    physicsList->InitializeWorker();
    kernel_->SetPhysics(physicsList);
    //kernel_->InitializePhysics();

    const bool kernel_init = kernel_->RunInitialization();
    if (!kernel_init) {
        //TODO: exception
    }
    // initialize user actions

    // set to idel state
    G4StateManager::GetStateManager()->SetNewState(G4State_Idle);

    thread_initialized_ = true;
}

void CMSRunManagerWorker::BeamOn(int event_number)
{
    if (!thread_initialized_) {
        Initialize();
    }

    // initialize the run
    G4Run* current_run_ = new G4Run();
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);

    G4Event* event = new G4Event(event_number);
    run_manager_master_.user_primary_action_->GeneratePrimaries(event);

    G4cout << "Processing " << event_number << G4endl;
    kernel_->GetEventManager()->ProcessOneEvent(event);

    kernel_->RunTermination();
    G4cout << "Finished " << event_number << G4endl;
}
