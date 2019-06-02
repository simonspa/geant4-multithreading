#include "SimpleMasterRunManager.hpp"
#include "SimpleWorkerRunManager.hpp"

namespace {
 G4Mutex setUpEventMutex = G4MUTEX_INITIALIZER;
}

G4ThreadLocal SimpleWorkerRunManager* SimpleMasterRunManager::worker_run_manager_ = nullptr;

SimpleMasterRunManager::SimpleMasterRunManager() : 
    G4MTRunManager()
{
}

SimpleMasterRunManager::~SimpleMasterRunManager()
{
}

void SimpleMasterRunManager::CleanUpWorker()
{
    G4cout << "Master Terminating worker " << worker_run_manager_ << G4endl;
    worker_run_manager_->RunTermination();
    delete worker_run_manager_;
    worker_run_manager_ = nullptr;
}

G4bool SimpleMasterRunManager::SetUpAnEvent(G4Event*, long& s1, long& s2, long& s3, G4bool reseedRequired)
{
    (void)reseedRequired;

    G4AutoLock l(&setUpEventMutex);
    G4RNGHelper* helper = G4RNGHelper::GetInstance();
    G4int idx_rndm = nSeedsPerEvent*nSeedsUsed;
    s1 = helper->GetSeed(idx_rndm);
    s2 = helper->GetSeed(idx_rndm+1);
    G4cout << "SetUpAnEvent s1=" << s1 << " s2=" << s2 << G4endl;
    if(nSeedsPerEvent==3) s3 = helper->GetSeed(idx_rndm+2);
    nSeedsUsed++;
    if(nSeedsUsed==nSeedsFilled) {
        // The RefillSeeds call will refill the array with
        numberOfEventToBeProcessed = nSeedsFilled + 1024;
        RefillSeeds();
    }
    numberOfEventProcessed++;
    return true;
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

    for (G4int i = 0; i < n_event; ++i) {
        G4int idx_rndm = nSeedsPerEvent*nSeedsUsed;
        long s1 = helper->GetSeed(idx_rndm), s2 = helper->GetSeed(idx_rndm+1);
        worker_run_manager_->seedsQueue.push(s1);
        worker_run_manager_->seedsQueue.push(s2);
        G4cout << "SetUpAnEvent s1=" << s1 << " s2=" << s2 << G4endl;

        // TODO: check if needed
        if(nSeedsPerEvent==3)
            worker_run_manager_->seedsQueue.push(helper->GetSeed(idx_rndm+2));

        nSeedsUsed++;
        if(nSeedsUsed==nSeedsFilled) {
            // The RefillSeeds call will refill the array with 1024 new entries
            numberOfEventToBeProcessed = nSeedsFilled + 1024;
            RefillSeeds();
        }
    }

    numberOfEventProcessed += n_event;

    worker_run_manager_->BeamOn(n_event);
}