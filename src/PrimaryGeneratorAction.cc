#include "PrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(const RunConfig& cfg)
: m_cfg(cfg), m_gun(std::make_unique<G4ParticleGun>(1))
{
  auto* table = G4ParticleTable::GetParticleTable();
  auto* def = table->FindParticle(m_cfg.particle);
  if (!def) {
    G4Exception("PrimaryGeneratorAction", "BadParticle", FatalException,
                ("Unknown particle: " + m_cfg.particle).c_str());
  }

  m_gun->SetParticleDefinition(def);
  m_gun->SetParticleEnergy(m_cfg.energy_MeV * MeV);

  m_gun->SetParticleMomentumDirection(
    G4ThreeVector(m_cfg.direction[0], m_cfg.direction[1], m_cfg.direction[2]).unit()
  );
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() = default;

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  double x = m_cfg.position_mm[0] * mm;
  double y = m_cfg.position_mm[1] * mm;
  double z = m_cfg.position_mm[2] * mm;

  if (m_cfg.sigma_xy_mm > 0) {
    x += G4RandGauss::shoot(0.0, m_cfg.sigma_xy_mm * mm);
    y += G4RandGauss::shoot(0.0, m_cfg.sigma_xy_mm * mm);
  }

  m_gun->SetParticlePosition(G4ThreeVector(x,y,z));
  m_gun->GeneratePrimaryVertex(event);
}
