#include "SimpleWorkerRunManager.hpp"
#include <G4Run.hh>
#include <G4MTRunManager.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4UserRunAction.hh>
#include <G4SDManager.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

SimpleWorkerRunManager::SimpleWorkerRunManager() :
    G4WorkerRunManager()
{}

SimpleWorkerRunManager::~SimpleWorkerRunManager()
{}

void SimpleWorkerRunManager::MergePartialResults()
{
    //TBB ??????
}

void SimpleWorkerRunManager::ConstructScoringWorlds()
{
    //Forward call to protected method, that's all
    G4WorkerRunManager::ConstructScoringWorlds();
}
G4Event* SimpleWorkerRunManager::GenerateEvent(G4int i_event)
{
    
  if(!userPrimaryGeneratorAction)
  {
    G4Exception("G4RunManager::GenerateEvent()", "Run0032", FatalException,
                "G4VUserPrimaryGeneratorAction is not defined!");
    return 0;
  }

    G4Event* anEvent = nullptr;

    if( numberOfEventProcessed < numberOfEventToBeProcessed && !runAborted ) {
        anEvent  = new G4Event(numberOfEventProcessed);

        // long s1 = 0;
        //long s2 = 0;
        //long s3 = 0;

        //         s1 = seedsQueue.front(); seedsQueue.pop();
        //     s2 = seedsQueue.front(); seedsQueue.pop();

        // long seeds[3] = { s1, s2, 0 };
        // G4Random::setTheSeeds(seeds,-1);
        // runIsSeeded = true;
            
        //if(printModulo > 0 && anEvent->GetEventID()%printModulo == 0 )
            G4cout << "--> Event " << anEvent->GetEventID() << " starts";
        //  { G4cout << " with initial seeds (" << s1 << "," << s2 << ")"; }
        //  G4cout << "." << G4endl;
        userPrimaryGeneratorAction->GeneratePrimaries(anEvent);
    } else {
        eventLoopOnGoing = false;
    }



  return anEvent;
}