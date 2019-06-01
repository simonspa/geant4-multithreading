#include "SimpleMasterRunManager.hpp"

namespace {
 G4Mutex setUpEventMutex = G4MUTEX_INITIALIZER;
}

SimpleMasterRunManager::SimpleMasterRunManager() : 
    G4MTRunManager()
{
}

SimpleMasterRunManager::~SimpleMasterRunManager()
{
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
        numberOfEventToBeProcessed = nSeedsFilled + 1; /// HACKKKKK
        RefillSeeds();
    }
    numberOfEventProcessed++;
    return true;
}