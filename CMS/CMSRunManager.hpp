#pragma once

#include <memory>

class CMSRunManagerWorker;

class G4MTRunManagerKernel;
class G4Run;
class G4Event;
class G4Field;
class G4StateManager;
class G4GeometryManager;
class G4VUserPhysicsList;
class G4VUserDetectorConstruction;
class G4VUserActionInitialization;
class G4VUserPrimaryGeneratorAction;

class CMSRunManager {
    friend CMSRunManagerWorker;
public:
    CMSRunManager();

    void Initialize();
    void InitializeGeometry();
    void InitializePhysics();

    void SetUserInitialization(G4VUserDetectorConstruction* userDC) { user_detector_ = userDC; }
    void SetUserInitialization(G4VUserPhysicsList* userPL);
    void SetUserInitialization(G4VUserActionInitialization* userInit);
    void SetUserAction(G4VUserPrimaryGeneratorAction* userAction) { user_primary_action_ = userAction; }

    void BeamOn(int nevent);

private:
    G4VUserPrimaryGeneratorAction* user_primary_action_ = nullptr;
    G4VUserActionInitialization * user_action_ = nullptr;
    G4VUserDetectorConstruction* user_detector_ = nullptr;
    G4VUserPhysicsList* physics_list_ = nullptr;
    G4MTRunManagerKernel* kernel_ = nullptr;
    G4Run* current_run_ = nullptr;
    G4StateManager* state_manager_ = nullptr;
    G4GeometryManager* geometry_manager_ = nullptr;

    static thread_local CMSRunManagerWorker* run_manager_worker_;
};