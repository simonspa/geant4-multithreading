#include <G4Event.hh>
#include <G4GeneralParticleSource.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleTable.hh>

/**
 * @brief Generates the particles in every event
 */
class GeneratorActionG4 : public G4VUserPrimaryGeneratorAction {
public:
    /**
     * @brief Constructs the generator action
     * @param config Configuration of the \ref DepositionGeant4Module module
     */
    explicit GeneratorActionG4(): particle_source_(std::make_unique<G4GeneralParticleSource>()) {
        auto single_source = particle_source_->GetCurrentSource();
        single_source->GetPosDist()->SetPosDisType("Beam");
        single_source->GetPosDist()->SetBeamSigmaInR(1.);
        single_source->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(0,0,1));

        auto particle = G4ParticleTable::GetParticleTable()->FindParticle("pi+");
        single_source->SetParticleDefinition(particle);
        single_source->SetNumberOfParticles(1);
        single_source->SetParticleTime(0.0);

        single_source->GetEneDist()->SetEnergyDisType("Gauss");
        single_source->GetEneDist()->SetMonoEnergy(120);
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
