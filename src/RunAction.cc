#include "RunAction.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction() {
  auto* man = G4AnalysisManager::Instance();
  man->SetDefaultFileType("root");
  man->SetFileName("calosim_out");
}

RunAction::~RunAction() {
  //delete G4AnalysisManager::Instance();
}

void RunAction::BeginOfRunAction(const G4Run*) {
  G4AnalysisManager::Instance()->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run*) {
  auto* man = G4AnalysisManager::Instance();
  man->Write();
  man->CloseFile();
}
