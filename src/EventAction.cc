#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4ios.hh"


EventAction::EventAction(EventStore* store) : m_store(store) {
  auto* man = G4AnalysisManager::Instance();

  man->CreateNtuple("calo_events", "calo hits");
  man->CreateNtupleDColumn("edep", m_store->edep);
  man->CreateNtupleDColumn("x_global",    m_store->x_global);
  man->CreateNtupleDColumn("y_global",    m_store->y_global);
  man->CreateNtupleDColumn("z_global",    m_store->z_global);
  man->CreateNtupleDColumn("x_local",     m_store->x_local);
  man->CreateNtupleDColumn("y_local",     m_store->y_local);
  man->CreateNtupleDColumn("z_local",     m_store->z_local);

  man->CreateNtupleIColumn("type",    m_store->type);
  man->CreateNtupleIColumn("section", m_store->section);
  man->CreateNtupleIColumn("layer",   m_store->layer);
  man->CreateNtupleIColumn("vol",     m_store->vol);
  man->CreateNtupleIColumn("hcal",     m_store->hcal);
  man->CreateNtupleIColumn("hpl_subsection",     m_store->hpl_sublayer);
  man->CreateNtupleIColumn("hexant",     m_store->hexant);

  man->FinishNtuple();
}

void EventAction::BeginOfEventAction(const G4Event* event) {
  m_store->clear();
  G4cout << "\n=== Event " << event->GetEventID() << " ===" << G4endl;
}

void EventAction::EndOfEventAction(const G4Event*) {
  G4AnalysisManager::Instance()->AddNtupleRow();
}
