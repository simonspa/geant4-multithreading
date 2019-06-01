#include "Module.hpp"
#include "SimpleMasterRunManager.hpp"
#include "SimpleWorkerRunManager.hpp"

Module::Module(SimpleMasterRunManager* runmanager)
: run_manager_(runmanager)
{
}
void Module::init(){
  run_manager_->InitializeForCustomThreads(2);
}
bool Module::run(int e)
{
  static thread_local std::unique_ptr<SimpleWorkerRunManager> worker_run_manager_;
    //run_manager_->BeamOn(1);

        if (!worker_run_manager_) {
        worker_run_manager_ = std::make_unique<SimpleWorkerRunManager>();
        worker_run_manager_->StartWorker();
    }

    worker_run_manager_->BeamOn(1);
  return true;
}

void Module::finialize() {
  run_manager_->RunTermination();
}
