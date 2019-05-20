#include "SimpleWorkerRunManager.hpp"
#include <G4Run.hh>
#include <G4MTRunManager.hh>
#include <G4UserWorkerInitialization.hh>
#include <G4UserRunAction.hh>
#include <G4SDManager.hh>

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
