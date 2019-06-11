#include "Module.hpp"
#include "SimpleMasterRunManager.hpp"

Module::Module(SimpleMasterRunManager* runmanager)
: run_manager_(runmanager)
{
}

void Module::init(){
}

bool Module::run(int e)
{
    // Equivalent to BeamOn(1) 
    run_manager_->Run(e, 1);
    return true;
}

void Module::finializeThread()
{
    run_manager_->TerminateForThread();
}

void Module::finialize() {
    run_manager_->RunTermination();
}
