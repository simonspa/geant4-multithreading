#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"

#include <G4RunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4UImanager.hh>

int main() {
    // Create the G4 run manager
    std::unique_ptr<G4RunManager> run_manager_g4_ = std::make_unique<G4RunManager>();

    // Initialize the geometry:
    auto geometry_construction = new GeometryConstructionG4();
    run_manager_g4_->SetUserInitialization(geometry_construction);
    run_manager_g4_->InitializeGeometry();

    // Initialize physics
    G4PhysListFactory physListFactory;
    G4VModularPhysicsList* physicsList = physListFactory.GetReferencePhysList("FTFP_BERT_EMZ");
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    run_manager_g4_->SetUserInitialization(physicsList);
    run_manager_g4_->InitializePhysics();

    // Particle source
    run_manager_g4_->SetUserInitialization(new GeneratorActionInitialization());

    G4UImanager* ui_g4 = G4UImanager::GetUIpointer();
    #define G4_NUM_SEEDS 10
    std::string seed_command = "/random/setSeeds ";
    for(int i = 0; i < G4_NUM_SEEDS; ++i) {
        seed_command += std::to_string(i);
        if(i != G4_NUM_SEEDS - 1) {
            seed_command += " ";
        }
    }
    ui_g4->ApplyCommand(seed_command);

    // Initialize the full run manager to ensure correct state flags
    run_manager_g4_->Initialize();

    // Run our own event loop:
    for(int i = 0; i < 5; i++) {
        run_manager_g4_->BeamOn(1);
    }

    return 0;
}
