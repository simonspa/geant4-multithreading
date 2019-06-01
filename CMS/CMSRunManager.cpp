#include "CMSRunManager.hpp"
#include "CMSRunManagerWorker.hpp"

#include <G4GeometryManager.hh>
#include <G4StateManager.hh>
#include <G4ApplicationState.hh>
#include <G4MTRunManagerKernel.hh>
#include <G4UImanager.hh>

#include <G4EventManager.hh>
#include <G4Run.hh>
#include <G4Event.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

using namespace std;

thread_local CMSRunManagerWorker* CMSRunManager::run_manager_worker_ = nullptr;

CMSRunManager::CMSRunManager()
{
    kernel_ = new G4MTRunManagerKernel();
    state_manager_ = G4StateManager::GetStateManager();
    geometry_manager_ = G4GeometryManager::GetInstance();
}

void CMSRunManager::SetUserInitialization(G4VUserActionInitialization* userInit)
{
    user_action_ = userInit;
    user_action_->Build();
}
void CMSRunManager::SetUserInitialization(G4VUserPhysicsList* userPL)
{
    physics_list_ = userPL;
    kernel_->SetPhysics(userPL);
}

void CMSRunManager::Initialize()
{
    G4cout << "CMSRunManager::Initialize" << G4endl;
    G4StateManager* stateManager = G4StateManager::GetStateManager();
    G4ApplicationState currentState = stateManager->GetCurrentState();
    if(currentState!=G4State_PreInit && currentState!=G4State_Idle)
    {
    G4cerr << "Illegal application state - "
            << "G4RunManager::Initialize() ignored." << G4endl;
    return;
    }

    stateManager->SetNewState(G4State_Init);
    //if(!geometryInitialized) InitializeGeometry();
    //if(!physicsInitialized) InitializePhysics();
    if(stateManager->GetCurrentState()!=G4State_Idle)
    { stateManager->SetNewState(G4State_Idle); }
}

void CMSRunManager::InitializeGeometry()
{
    G4cout << "CMSRunManager::InitializeGeometry" << G4endl;
     if(!user_detector_)
    {
        // TODO: exception
        return;
    }

    G4StateManager* stateManager = G4StateManager::GetStateManager();
    G4ApplicationState currentState = stateManager->GetCurrentState();
    if(currentState==G4State_PreInit || currentState==G4State_Idle)
    { stateManager->SetNewState(G4State_Init); }

    kernel_->DefineWorldVolume(user_detector_->Construct(),false);
    user_detector_->ConstructSDandField();
    int nParallelWorlds = user_detector_->ConstructParallelGeometries();
    user_detector_->ConstructParallelSD();
    kernel_->SetNumberOfParallelWorld(nParallelWorlds);
    stateManager->SetNewState(currentState); 
}

void CMSRunManager::InitializePhysics()
{
    G4cout << "CMSRunManager::InitializePhysics" << G4endl;
    G4StateManager* stateManager = G4StateManager::GetStateManager();
    G4ApplicationState currentState = stateManager->GetCurrentState();
    if(currentState==G4State_PreInit || currentState==G4State_Idle)
    { stateManager->SetNewState(G4State_Init); }

    if(physics_list_)
    {
        kernel_->InitializePhysics();
    }
    else
    {

    }
    stateManager->SetNewState(currentState); 
}


void CMSRunManager::BeamOn(int nevent)
{
    G4cout << "CMSRunManager::BeamOn" << G4endl;
    if (!run_manager_worker_) {
        run_manager_worker_ = new CMSRunManagerWorker(*this);
    }

    run_manager_worker_->BeamOn(nevent);
}