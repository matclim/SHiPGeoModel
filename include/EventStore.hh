#pragma once
#include "G4ThreeVector.hh"
#include <vector>
#include "CaloSD.hh"

class EventStore {
public:
  void clear();
  void addHit(const ParsedID& id, double edep, const G4ThreeVector& pos);

  // vectors filled per event
  std::vector<double> edep, x, y, z;
  std::vector<int> type, section, layer, vol, hcal, hpl_sublayer, hexant;
};
