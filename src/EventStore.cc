#include "EventStore.hh"
#include "G4SystemOfUnits.hh"

void EventStore::clear() {
  edep.clear(); x.clear(); y.clear(); z.clear();
  type.clear(); section.clear(); layer.clear(); vol.clear();
}

void EventStore::addHit(const ParsedID& id, double e, const G4ThreeVector& p) {
  edep.push_back(e);         // in Geant4 energy units (MeV by default)
  x.push_back(p.x()); y.push_back(p.y()); z.push_back(p.z()); // in mm
  type.push_back(id.type);
  section.push_back(id.section);
  layer.push_back(id.layer);
  vol.push_back(id.vol);
  hcal.push_back(id.hcal);
  hpl_sublayer.push_back(id.hpl_sublayer);
}
