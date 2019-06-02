#include <G4VSensitiveDetector.hh>
#include <G4SDManager.hh>
#include <thread>

/**
 * @brief Handles the steps of the particles in all sensitive devices
 */
class SensitiveDetectorActionG4 : public G4VSensitiveDetector {
public:
    /**
     * @brief Constructs the action handling for every sensitive detector
     */
    SensitiveDetectorActionG4() : G4VSensitiveDetector("SensitiveDetector") {
        // Add the sensor to the internal sensitive detector manager
        G4SDManager* sd_man_g4 = G4SDManager::GetSDMpointer();
        sd_man_g4->AddNewDetector(this);

        std::cerr<< "SensitiveDetectorActionG4" << std::endl;
    };

    /**
     * @brief Process a single step of a particle passage through this sensor
     * @param step Information about the step
     * @param history Parameter not used
     */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override {
        // Get the step parameters
        auto edep = step->GetTotalEnergyDeposit();
        G4StepPoint* preStepPoint = step->GetPreStepPoint();
        G4StepPoint* postStepPoint = step->GetPostStepPoint();

        // Get Transportaion Matrix
        G4TouchableHandle theTouchable = step->GetPreStepPoint()->GetTouchableHandle();

        // Put the charge deposit in the middle of the step
        G4ThreeVector mid_pos = (preStepPoint->GetPosition() + postStepPoint->GetPosition()) / 2;
        double mid_time = (preStepPoint->GetGlobalTime() + postStepPoint->GetGlobalTime()) / 2;

        std::cerr << std::this_thread::get_id() <<  " Step. E=" << edep << " PosX=" << mid_pos.x() << " t=" << mid_time << std::endl;
        return true;
    };
};
