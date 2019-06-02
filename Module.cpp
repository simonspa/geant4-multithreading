#include "Module.hpp"
#include "SimpleMasterRunManager.hpp"

Module::Module(SimpleMasterRunManager* runmanager)
: run_manager_(runmanager)
{
}
void Module::init(){
  run_manager_->InitializeForCustomThreads(2);
}
bool Module::run(int e)
{
  (void)e;
    run_manager_->Run(e, 1);
    return true;
}

void Module::finialize() {
    run_manager_->RunTermination();
}
void Module::cleanup()
{
  run_manager_->CleanUpWorker();
}