#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"

#include <G4RunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>

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

    // Initialize the full run manager to ensure correct state flags
    run_manager_g4_->Initialize();

    // Run our own event loop:
    for(int i = 0; i < 100000; i++) {
        run_manager_g4_->BeamOn(1);
    }

    return 0;
}
