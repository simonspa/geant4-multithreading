#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"
#include "tools/ThreadPool.hpp"

#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4UserWorkerThreadInitialization.hh>
#include <G4WorkerThread.hh>
#include <G4UImanager.hh>
#include <G4MTRunManagerKernel.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4VUserPhysicsList.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserActionInitialization.hh>

#include "Module.hpp"
#include "SimpleMasterRunManager.hpp"

int main(int argc, char *argv[]) {
    // How many threads do we use?
    std::vector<std::string> args{argv + 1, argv + argc};
    int threads_num = args.size() > 0 ? std::stoi(args[0]) : 1;
    std::cout << "Using " << threads_num << " thread(s).\n";

    SimpleMasterRunManager* run_manager_ = new SimpleMasterRunManager;

    // Initialize the geometry:
    auto geometry_construction = new GeometryConstructionG4();
    run_manager_->SetUserInitialization(geometry_construction);
    run_manager_->InitializeGeometry();

    // Initialize physics
    G4PhysListFactory physListFactory;
    G4VModularPhysicsList* physicsList = physListFactory.GetReferencePhysList("FTFP_BERT_EMZ");
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    run_manager_->SetUserInitialization(physicsList);
    run_manager_->InitializePhysics();

    // Particle source
    run_manager_->SetUserInitialization(new GeneratorActionInitialization());
    // run_manager_->SetUserAction(new GeneratorActionG4());

    // Set the seeds before calling initialize
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
    // This call will initialize the manager's event loop and as such enable later calls
    // for BeamOn on multiple threads
    run_manager_->Initialize();

    auto module = std::make_unique<Module>(run_manager_);
    module->init();

    std::vector<std::future<bool>> module_futures;

    // Start new thread pool and create module object:
    ThreadPool pool(threads_num, [module = module.get()]() {
        // cleanup all thread local stuff
        module->finializeForThread();
    });

    // The event loop:
    for (int i = 0; i < 5; i++) {
        // Define module execution:
        auto execute_module = [module = module.get(), event_num = i + 1]() {
            return module->run(event_num);
        };

        module_futures.push_back(pool.submit(execute_module));
    }

    // Ask for futures:
    for(auto& module_future : module_futures) {
        module_future.get();
    }

    pool.shutdown();

    module->finialize();

    delete run_manager_;

    std::cout << "Finished all work." << std::endl;
    return 0;
}
