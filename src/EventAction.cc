#include "EventAction.hh"
#include "G4AnalysisManager.hh"

EventAction::EventAction(EventStore* store) : m_store(store) {
  auto* man = G4AnalysisManager::Instance();

  man->CreateNtuple("calo_events", "calo hits");
  man->CreateNtupleDColumn("edep", m_store->edep);
  man->CreateNtupleDColumn("x",    m_store->x);
  man->CreateNtupleDColumn("y",    m_store->y);
  man->CreateNtupleDColumn("z",    m_store->z);

  man->CreateNtupleIColumn("type",    m_store->type);
  man->CreateNtupleIColumn("section", m_store->section);
  man->CreateNtupleIColumn("layer",   m_store->layer);
  man->CreateNtupleIColumn("vol",     m_store->vol);
  man->CreateNtupleIColumn("hcal",     m_store->hcal);
  man->CreateNtupleIColumn("hpl_subsection",     m_store->hpl_sublayer);
  man->CreateNtupleIColumn("hexant",     m_store->hexant);

  man->FinishNtuple();
}

void EventAction::BeginOfEventAction(const G4Event*) {
  m_store->clear();
}

void EventAction::EndOfEventAction(const G4Event*) {
  G4AnalysisManager::Instance()->AddNtupleRow();
}
