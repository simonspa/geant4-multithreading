#include <string>
#include <iostream>

#include "geometry.hpp"
#include "generator.hpp"

#include <G4RunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>

int main(int argc, char *argv[]) {
    // How many threads do we use?
    std::vector<std::string> args{argv + 1, argv + argc};
    int threads_num = args.size() > 0 ? std::stoi(args[0]) : 1;
    std::cout << "Using " << threads_num << " thread(s).\n";

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

    // Initialize the full run manager to ensure correct state flags
    run_manager_g4_->Initialize();

    // Particle source
    auto generator = new GeneratorActionG4();
    run_manager_g4_->SetUserAction(generator);

    run_manager_g4_->BeamOn(10);
}
