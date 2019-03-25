#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "simulation/geometry.hpp"
#include "simulation/generator.hpp"
#include "tools/ThreadPool.hpp"

#include "MyRunManager.hpp"
#include "MyWorkerRunManager.hpp"

#include <G4StepLimiterPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4UserWorkerThreadInitialization.hh>
#include <G4WorkerThread.hh>
#include <G4UImanager.hh>
#include <G4MTRunManagerKernel.hh>

class Module {
    public:
        Module(MyRunManager* rm) : run_manager_(rm) {
        }

        // Init to be called from main thread
        void init() {
        };

        bool run(int evt_nr) const {
            std::cout << "Running event " << evt_nr << std::endl;

            /// This work is done to initialize the WorkerManager. It is done at the start of each thread
            /// created by the MTRunManager. We also need to do it here. Maybe only one...
            ///
            static thread_local G4WorkerThread* wThreadContext;
            static thread_local bool init1 = false;
            if (!init1) {
                wThreadContext = new G4WorkerThread;
                wThreadContext->SetThreadId(evt_nr);

                //G4MTRunManagerKernel* masterKern = run_manager_->GetMTMasterRunManagerKernel();
                //masterKern->wThreadContext = wThreadContext;

                // G4Threading::WorkerThreadJoinsPool();
                // wThreadContext = context;
                // G4MTRunManager* masterRM = G4MTRunManager::GetMasterRunManager();

                    
                //============================
                //Step-0: Thread ID
                //============================
                //Initliazie per-thread stream-output
                //The following line is needed before we actually do IO initialization
                //becasue the constructor of UI manager resets the IO destination.
                G4int thisID = wThreadContext->GetThreadId();
                G4Threading::G4SetThreadId(thisID);
                G4UImanager::GetUIpointer()->SetUpForAThread(thisID);

                //============================
                //Optimization: optional
                //============================
                //Enforce thread affinity if requested
                // wThreadContext->SetPinAffinity(masterRM->GetPinAffinity());

                //============================
                //Step-1: Random number engine
                //============================
                //RNG Engine needs to be initialized by "cloning" the master one.
                const CLHEP::HepRandomEngine* masterEngine = run_manager_->getMasterRandomEngine();
                run_manager_->GetUserWorkerThreadInitialization()->SetupRNGEngine(masterEngine);

                //============================
                //Step-2: Initialize worker thread
                //============================
                if(run_manager_->GetUserWorkerInitialization())
                { run_manager_->GetUserWorkerInitialization()->WorkerInitialize(); }
                if(run_manager_->GetUserActionInitialization())
                {
                    G4VSteppingVerbose* sv = run_manager_->GetUserActionInitialization()->InitializeSteppingVerbose();
                    if ( sv ) { G4VSteppingVerbose::SetInstance(sv); }
                }
                //Now initialize worker part of shared objects (geometry/physics)
                wThreadContext->BuildGeometryAndPhysicsVector();
                
                init1 = true;
            }

            static thread_local MyWorkerRunManager* worker_run_manager_
                // = run_manager_->GetUserWorkerThreadInitialization()->CreateWorkerRunManager();
                = new MyWorkerRunManager;

            static thread_local bool init2 = false;
            if (!init2) {
                // G4WorkerRunManager* wrm
                //         = masterRM->GetUserWorkerThreadInitialization()->CreateWorkerRunManager();
                worker_run_manager_->SetWorkerThread(wThreadContext);
                // G4AutoLock wrmm(&workerRMMutex);
                // workerRMvector->push_back(wrm);
                // wrmm.unlock();

                //================================
                //Step-3: Setup worker run manager
                //================================
                // Set the detector and physics list to the worker thread. Share with master
                const G4VUserDetectorConstruction* detector = run_manager_->GetUserDetectorConstruction();
                worker_run_manager_->G4RunManager::SetUserInitialization(const_cast<G4VUserDetectorConstruction*>(detector));
                const G4VUserPhysicsList* physicslist = run_manager_->GetUserPhysicsList();
                worker_run_manager_->SetUserInitialization(const_cast<G4VUserPhysicsList*>(physicslist));

                //================================
                //Step-4: Initialize worker run manager
                //================================
                if(run_manager_->GetUserActionInitialization())
                { run_manager_->GetNonConstUserActionInitialization()->Build(); }
                if(run_manager_->GetUserWorkerInitialization())
                { run_manager_->GetUserWorkerInitialization()->WorkerStart(); }

                // FIXME: This cases a sigfault while initializing the geometry
                // Looks like some initialization is missing!
                worker_run_manager_->Initialize();

                init2 = true;
            }

            // call BeamOn using the worker associated with the calling thread.
            worker_run_manager_->DoWork();

            std::cout << "Done with event " << evt_nr << std::endl;

            // done...
            return true;
        }
    private:
        MyRunManager* run_manager_;
};

int main(int argc, char *argv[]) {
    // How many threads do we use?
    std::vector<std::string> args{argv + 1, argv + argc};
    int threads_num = args.size() > 0 ? std::stoi(args[0]) : 1;
    std::cout << "Using " << threads_num << " thread(s).\n";

    // Start new thread pool and create module object:
    ThreadPool pool(threads_num);

    MyRunManager* run_manager_ = new MyRunManager;
    // Set custom thread initialization
    // run_manager_->SetUserInitialization(new MyThreadInitialization);

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

    // Initialize the full run manager to ensure correct state flags
    run_manager_->Initialize();

    auto module = std::make_unique<Module>(run_manager_);
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
