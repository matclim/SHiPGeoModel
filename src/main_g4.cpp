#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4PhysListFactory.hh"

#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "EventStore.hh"
#include "RunConfig.hh"
#include "PrimaryGeneratorAction.hh"


// Minimal primary generator
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"





int main(int argc, char** argv)
{
  // Usage:
  //   ./run_g4 calo.cfg [macro]
 
  std::string caloCfg = "../calo.cfg";
  std::string runCfg  = "../run.cfg";
  if (argc > 1) caloCfg = argv[1];
  if (argc > 2) runCfg  = argv[2];

  auto run = readRunConfigFile(runCfg);

  // optional seed
  if (run.seed != 0) {
    CLHEP::HepRandom::setTheSeed(run.seed);
  }

  


     
  const std::string cfgFile = (argc > 1) ? argv[1] : "../calo.cfg";

  auto* runManager = new G4RunManager;

  // Shared event store
  auto* store = new EventStore;

  runManager->SetUserInitialization(new DetectorConstruction(store, cfgFile));

  G4PhysListFactory factory;
  runManager->SetUserInitialization(factory.GetReferencePhysList("FTFP_BERT"));
  runManager->SetUserAction(new PrimaryGeneratorAction(run));

  runManager->SetUserAction(new RunAction());
  runManager->SetUserAction(new EventAction(store));

  runManager->Initialize();

  // Visualization + UI (optional)
  // Visualization manager only if requested (or if you enter interactive)
  G4VisExecutive* vis = nullptr;
  if (run.visualize || run.macro.empty()) {
    vis = new G4VisExecutive;
    vis->Initialize();
  }
  
  auto* UImanager = G4UImanager::GetUIpointer();
  
  if (run.visualize) {
    auto* app = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute " + G4String(run.vis_macro));
    app->SessionStart();
    delete app;
  } else {
    // your current batch behavior
    if (!run.macro.empty()) {
      UImanager->ApplyCommand("/control/execute " + G4String(run.macro));
      if (run.n_events > 0) {
        UImanager->ApplyCommand("/run/beamOn " + G4String(std::to_string(run.n_events)));
      }
    } else {
      // if you want: pure non-vis interactive disabled -> just beamOn or exit
      // (keep your old interactive branch if you still want it)
    }
  }

delete vis;
   if (!run.macro.empty()) {
  }  

  delete vis;
  delete runManager;
  delete store;
  return 0;
}
