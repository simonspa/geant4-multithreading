#include <G4VUserDetectorConstruction.hh>
#include <G4PVPlacement.hh>
#include <G4LogicalVolume.hh>
#include <G4Box.hh>
#include <G4NistManager.hh>

/**
* @brief Constructs the Geant4 geometry during Geant4 initialization
*/
class GeometryConstructionG4 : public G4VUserDetectorConstruction {
public:
    /**
    * @brief Constructs geometry construction module
    */
    GeometryConstructionG4() = default;

    /**
    * @brief Constructs the world geometry with all detectors
    * @return Physical volume representing the world
    */
    G4VPhysicalVolume* Construct() override {
        // Get Work material:
        G4NistManager* nistman = G4NistManager::Instance();
        auto world_material = nistman->FindOrBuildMaterial("G4_AIR");
        auto silicon  = nistman->FindOrBuildMaterial("G4_Si");

        auto world_box = new G4Box("World", 50, 50, 50);
        world_log_ = std::make_unique<G4LogicalVolume>(world_box, world_material, "World", nullptr, nullptr, nullptr);

        // Place the world at the center
        world_phys_ = std::make_unique<G4PVPlacement>(nullptr, G4ThreeVector(0., 0., 0.), world_log_.get(), "World", nullptr, false, 0);

        // Create the detector wrapper box and logical volume and place it
        auto wrapper_box = new G4Box("wrapper_detector", 1, 1, 1);
        auto wrapper_log = new G4LogicalVolume(wrapper_box, world_material, "wrapper_detector_log");
        auto wrapper_phys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), wrapper_log, "wrapper_detector_phys", world_log_.get(), false, 0, true);
        solids_.push_back(wrapper_box);

        // Create the sensor box and logical volume and place it
        auto sensor_box = new G4Box("sensor_detector", 1, 1, 0.5);
        sensor_log_ = new G4LogicalVolume(sensor_box, silicon, "sensor_detector_log");
        auto sensor_phys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), sensor_log_, "sensor_detector_phys", wrapper_log, false, 0, true);
        solids_.push_back(sensor_box);

        return world_phys_.get();
    };
    G4LogicalVolume * sensor_log_;

private:
    std::vector<G4VSolid*> solids_;
    std::unique_ptr<G4VPhysicalVolume> world_phys_;
    std::unique_ptr<G4LogicalVolume> world_log_;
};
