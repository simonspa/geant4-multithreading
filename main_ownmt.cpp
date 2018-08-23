#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"
#include "tools/ThreadPool.hpp"

#include <G4RunManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>

class Module {
    public:
        Module() = default;

        // Init to be called from main thread
        void init() {
            // FIXME Here we would initialize the module, i.e. create the necessary Kernel instances,
            // load physics lists and geometry etc.
        };

        bool run(int evt_nr) const {
            std::cout << "Running event " << evt_nr << std::endl;

            // FIXME here we would call the eqivalent of BeamOn(1), i.e. process one event
            return true;
        }
};

int main(int argc, char *argv[]) {
    // How many threads do we use?
    std::vector<std::string> args{argv + 1, argv + argc};
    int threads_num = args.size() > 0 ? std::stoi(args[0]) : 1;
    std::cout << "Using " << threads_num << " thread(s).\n";

    // Start new thread pool and create module object:
    ThreadPool pool(threads_num);
    auto module = std::make_unique<Module>();
    module->init();

    std::vector<std::future<bool>> module_futures;

    // The event loop:
    for (int i = 0; i < 10000; i++) {
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

    std::cout << "Finished all work." << std::endl;
    return 0;
}
