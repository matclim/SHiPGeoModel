#pragma once
#include "G4UserEventAction.hh"
#include "EventStore.hh"

class EventAction : public G4UserEventAction {
public:
  EventAction(EventStore* store);
  void BeginOfEventAction(const G4Event*) override;
  void EndOfEventAction(const G4Event*) override;
private:
  EventStore* m_store = nullptr;
};
