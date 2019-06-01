#pragma once

#include "CMSRunManager.hpp"

#include <memory>

class G4Event;
class G4SimEvent;
class G4Run;
class G4RunManagerKernel;

class CMSRunManagerWorker {
public:
    CMSRunManagerWorker(CMSRunManager& run_manager);
    ~CMSRunManagerWorker();


    void BeamOn(int event_number);

private:

    void Initialize();

    CMSRunManager& run_manager_master_;

    G4SimEvent* sim_event_;

    // Thread local stuff
    static thread_local std::unique_ptr<G4RunManagerKernel> kernel_;
    static thread_local bool thread_initialized_;
    static thread_local bool run_terminated_;
};