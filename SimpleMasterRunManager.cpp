#include "SimpleMasterRunManager.hpp"
#include "SimpleWorkerRunManager.hpp"

G4ThreadLocal SimpleWorkerRunManager* SimpleMasterRunManager::worker_run_manager_ = nullptr;

SimpleMasterRunManager::SimpleMasterRunManager() : 
    G4MTRunManager()
{
}

SimpleMasterRunManager::~SimpleMasterRunManager()
{
}

void SimpleMasterRunManager::Initialize()
{
    G4MTRunManager::Initialize();

    // This is needed to draw random seeds and fill the internal seed array
    // use nSeedsMax to fill as much as possible now and hopefully avoid
    // refilling later
    G4MTRunManager::InitializeEventLoop(nSeedsMax, nullptr, 0);
}

void SimpleMasterRunManager::TerminateForThread()
{
    worker_run_manager_->RunTermination();
    delete worker_run_manager_;
    worker_run_manager_ = nullptr;
}

void SimpleMasterRunManager::Run(G4int i_event, G4int n_event)
{
    if (!worker_run_manager_) {
        worker_run_manager_ = SimpleWorkerRunManager::GetNewInstanceForThread();
    }

    // the allpix event number, do we need it?
    worker_run_manager_->currEvID = i_event;

    // seed the run here first before we run on a seperate thread.
    G4RNGHelper* helper = G4RNGHelper::GetInstance();

    G4int idx_rndm = nSeedsPerEvent*nSeedsUsed;
    long s1 = helper->GetSeed(idx_rndm), s2 = helper->GetSeed(idx_rndm+1);
    worker_run_manager_->seedsQueue.push(s1);
    worker_run_manager_->seedsQueue.push(s2);
    G4cout << "SetUpAnEvent s1=" << s1 << " s2=" << s2 << G4endl;

    nSeedsUsed++;
    if(nSeedsUsed==nSeedsFilled) {
        // The RefillSeeds call will refill the array with 1024 new entries
        numberOfEventToBeProcessed = nSeedsFilled + 1024;
        RefillSeeds();
    }

    numberOfEventProcessed += n_event;

    worker_run_manager_->BeamOn(n_event);
}