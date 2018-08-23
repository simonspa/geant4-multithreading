#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "geometry.hpp"
#include "generator.hpp"
#include "ThreadPool.hpp"

#include <G4RunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>

class Module {
    public:
        Module() {
            std::cout << "Constructed module" << std::endl;
        }

        bool run(int event) const {
            std::cout << "Running event " << event << std::endl;

            // FIXME this needs to be replaced.
            // auto run_manager_g4_ = G4RunManager::GetRunManager();
            // run_manager_g4_->BeamOn(1);

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            return true;
        }
};

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

    // Particle source
    run_manager_g4_->SetUserInitialization(new GeneratorActionInitialization());

    // Initialize the full run manager to ensure correct state flags
    run_manager_g4_->Initialize();


    // Start new thread pool and create module object:
    ThreadPool pool(threads_num);
    auto module = std::make_unique<Module>();
    std::vector<std::future<bool>> module_futures;

    // The event loop:
    for (int i = 0; i < 10; i++) {
        // Define module execution:
        auto execute_module = [module = module.get(), event_num = i + 1]() {
            return module->run(event_num);
        };

        module_futures.push_back(pool.submit(execute_module));
    }

    // FIXME Ask for futures:
    for(auto& module_future : module_futures) {
        module_future.get();
    }

    pool.shutdown();

    std::cout << "Finished all work." << std::endl;
    return 0;
}
