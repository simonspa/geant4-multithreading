#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"

#include <G4MTRunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4UImanager.hh>

int main(int argc, char *argv[]) {
    // How many threads do we use?
    std::vector<std::string> args{argv + 1, argv + argc};
    int threads_num = args.size() > 0 ? std::stoi(args[0]) : 1;
    std::cout << "Using " << threads_num << " thread(s).\n";

    // Create the G4 run manager
    std::unique_ptr<G4MTRunManager> run_manager_g4_ = std::make_unique<G4MTRunManager>();
    run_manager_g4_->SetNumberOfThreads(threads_num);

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


    #define G4_NUM_SEEDS 100
    //std::string seed_command = "/random/setSeeds ";
    //for(int i = 0; i < G4_NUM_SEEDS; ++i) {
    //    seed_command += std::to_string(0);
    //    if(i != G4_NUM_SEEDS - 1) {
    //        seed_command += " ";
    //    }
    //}
    //G4UImanager* ui_g4 = G4UImanager::GetUIpointer();
    //ui_g4->ApplyCommand(seed_command);

    // Execute the event loop:
    run_manager_g4_->BeamOn(10);

    return 0;
}
