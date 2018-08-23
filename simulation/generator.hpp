#include <G4Event.hh>
#include <G4GeneralParticleSource.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4ParticleTable.hh>

/**
 * @brief Generates the particles in every event
 */
class GeneratorActionG4 : public G4VUserPrimaryGeneratorAction {
public:
    /**
     * @brief Constructs the generator action
     */
    explicit GeneratorActionG4(): particle_source_(std::make_unique<G4GeneralParticleSource>()) {
        auto source = particle_source_->GetCurrentSource();
        source->GetPosDist()->SetPosDisType("Beam");
        source->GetPosDist()->SetBeamSigmaInR(1.);
        source->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(0,0,1));

        source->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("pi+"));
        source->SetNumberOfParticles(1);

        source->GetEneDist()->SetEnergyDisType("Gauss");
        source->GetEneDist()->SetMonoEnergy(120);
    };

    /**
     * @brief Generate the particle for every event
     */
    void GeneratePrimaries(G4Event* event) override {
        particle_source_->GeneratePrimaryVertex(event);
    };

private:
    std::unique_ptr<G4GeneralParticleSource> particle_source_;
};

/**
 * @brief Initializer for the generator action, required for G4MTRunManager, optional for sequential processing
 */
class GeneratorActionInitialization : public G4VUserActionInitialization {
public:
    explicit GeneratorActionInitialization() = default;

    /**
     * @brief Build the user action to be executed by the worker
     */
    void Build() const override {
        SetUserAction(new GeneratorActionG4());
    };

};
