#pragma once
#include "G4UserRunAction.hh"

class RunAction : public G4UserRunAction {
public:
  RunAction();
  RunAction(int Beamenergy);
  ~RunAction() override;
  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;
};
