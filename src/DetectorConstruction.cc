#include "DetectorConstruction.hh"

#include "EventStore.hh"
#include "CaloSD.hh"

// GeoModel builder
#include "MaterialManager.h"
#include "ConfigReader.h"
#include "CalorimeterBuilder.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/Units.h"

#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"



// GeoModel2G4
#include "GeoModel2G4/Geo2G4AssemblyFactory.h"
#include "GeoModel2G4/Geo2G4AssemblyVolume.h"
#include "GeoModel2G4/ExtParameterisedVolumeBuilder.h"

// Geant4
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

using namespace GeoModelKernelUnits;

static G4VisAttributes* MakeVis(double r, double g, double b, double a=1.0) {
    auto* v = new G4VisAttributes(G4Colour(r, g, b, a));
    v->SetVisibility(true);
    v->SetForceSolid(true);
    return v;
}

double GetSystemThickness(std::vector<int> vec_layers,double thickness_wide, double thickness_thin, double thickness_hpl, double thickness_passive, double thickness_airgap){

    double total_thickness = 0;

    for(int N_layer=0;N_layer<vec_layers.size();N_layer++){
        int layercode = vec_layers[N_layer];
        switch(layercode){
            case 1: total_thickness += thickness_wide; break;
            case 2: total_thickness += thickness_wide; break;
            case 3: total_thickness += thickness_thin; break;
            case 4: total_thickness += thickness_thin; break;
            case 5: total_thickness += thickness_hpl; break;
            case 6: total_thickness += thickness_hpl; break;
            case 7: total_thickness += thickness_passive; break;
            case 8: total_thickness += thickness_airgap; break;
        }
    }

    return total_thickness;
}


DetectorConstruction::DetectorConstruction(EventStore* store, std::string cfgFile)
: m_store(store), m_cfgFile(std::move(cfgFile)) {}

DetectorConstruction::~DetectorConstruction() {}

GeoPhysVol* DetectorConstruction::buildGeoModelWorld()
{
  MaterialManager MM;
  auto* air = MM.air();

  auto cfg = readConfigFile(m_cfgFile);
  

  const double worldXY = cfg.plate_xy_mm * GeoModelKernelUnits::mm;
  const double worldZ  = 6.0 * GeoModelKernelUnits::m;

  auto* worldShape = new GeoBox(0.5*worldXY, 0.5*worldXY, 0.5*worldZ);
  auto* worldLog   = new GeoLogVol("WorldLog", worldShape, air);
  auto* worldPhys  = new GeoPhysVol(worldLog);


const int nx = std::max(1, cfg.module_nx);
const int ny = std::max(1, cfg.module_ny);

const double pitchX = (cfg.module_pitch_x_mm > 0 ? cfg.module_pitch_x_mm : cfg.plate_xy_mm) * GeoModelKernelUnits::mm;
const double pitchY = (cfg.module_pitch_y_mm > 0 ? cfg.module_pitch_y_mm : cfg.plate_xy_mm) * GeoModelKernelUnits::mm;

// center the grid around (0,0)
const double x0 = -0.5 * (nx - 1) * pitchX;
const double y0 = -0.5 * (ny - 1) * pitchY;

for (int ix = 0; ix < nx; ++ix) {
  for (int iy = 0; iy < ny; ++iy) {
    const int mx = ix + 1;   // 1..nx
    const int my = iy + 1;   // 1..ny

    const double x = x0 + ix * pitchX;
    const double y = y0 + iy * pitchY;

    // Make a module container volume (air box) and build into it
    auto* modShape = new GeoBox(0.5*cfg.plate_xy_mm*GeoModelKernelUnits::mm, 0.5*cfg.plate_xy_mm*GeoModelKernelUnits::mm, 3.0*GeoModelKernelUnits::m); // generous Z
    auto* modLog   = new GeoLogVol("ModuleLog", modShape, MM.air());
    auto* modPhys  = new GeoPhysVol(modLog);

    worldPhys->add(new GeoNameTag(("MODULE_MX"+std::to_string(mx)+"Y"+std::to_string(my)).c_str()));
    worldPhys->add(new GeoTransform(GeoTrf::Translate3D(x, y, 0)));
    worldPhys->add(modPhys);

    CalorimeterBuilder::buildStack(modPhys, MM, cfg, mx, my);
  }
}

  G4cout << "[DetectorConstruction] GeoModel world children = "
         << worldPhys->getNChildVols() << G4endl;

  return worldPhys;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Build GeoModel world (contains stack as children)
  m_geoWorld = buildGeoModelWorld();

  //  Create a Geant4 world (mother volume) 
  auto* nist = G4NistManager::Instance();
  auto* g4Air = nist->FindOrBuildMaterial("G4_AIR");

  const auto worldXY = 3.0 * CLHEP::m;
  const auto worldZ  = 6.0 * CLHEP::m;

  auto* solidWorld = new G4Box("WorldSolid", 0.5*worldXY, 0.5*worldXY, 0.5*worldZ);
  auto* lvWorld    = new G4LogicalVolume(solidWorld, g4Air, "WorldLV");


  //  Convert GeoModel tree into an Assembly and imprint into lvWorld 
  bool ok = true;                 // builder doesn’t take ok in this API
  PVConstLink worldLink = m_geoWorld;
  
  // This builder is concrete (VolumeBuilder is abstract)
  ExtParameterisedVolumeBuilder vb("World");   // key string can be anything; "World" is fine
  
  G4LogicalVolume* g4WorldLV = vb.Build(worldLink);
  
  if (!g4WorldLV) {
    G4Exception("DetectorConstruction::Construct",
                "Geo2G4BuildFailed",
                FatalException,
                "ExtParameterisedVolumeBuilder::Build returned null LV.");
  }
  
  auto* pvWorld = new G4PVPlacement(nullptr, {}, g4WorldLV, "WorldPV", nullptr, false, 0);
  
  G4cout << "[DetectorConstruction] After Build: world daughters = "
         << g4WorldLV->GetNoDaughters()
         << " LVStore size = " << G4LogicalVolumeStore::GetInstance()->size()
         << G4endl;
  
  

  //  Sensitive detector 
  auto* sdman = G4SDManager::GetSDMpointer();
  auto* caloSD = new CaloSD("CaloSD", m_store);
  sdman->AddNewDetector(caloSD);

  int nSensitive = 0;
  for (auto* lv : *G4LogicalVolumeStore::GetInstance()) {
    const std::string ln = lv->GetName();

    // Update these patterns to match your GeoLogVol names (print a few if needed)
    if (ln.find("Wide") != std::string::npos && ln.find("PVT") != std::string::npos) {
      lv->SetSensitiveDetector(caloSD); ++nSensitive;
    } else if (ln.find("Thin") != std::string::npos &&
              (ln.find("PS") != std::string::npos || ln.find("Poly") != std::string::npos)) {
      lv->SetSensitiveDetector(caloSD); ++nSensitive;
    } else if (ln.find("HPL") != std::string::npos &&
              (ln.find("Fiber") != std::string::npos || ln.find("Fibre") != std::string::npos)) {
      lv->SetSensitiveDetector(caloSD); ++nSensitive;
    }
  }

  G4cout << "[DetectorConstruction] Sensitive LVs set: " << nSensitive << G4endl;
  auto* store = G4LogicalVolumeStore::GetInstance();
  
  auto* visWide   = MakeVis(0.0, 0.7, 0.7, 1.0);   // teal
  auto* visThin   = MakeVis(1.0, 0.55, 0.0, 1.0);  // orange
  auto* visFibre  = MakeVis(0.1, 0.2, 1.0, 1.0);   // blue
  auto* visLead   = MakeVis(0.5, 0.5, 0.5, 1.0);   // gray
  auto* visIron   = MakeVis(0.9, 0.0, 0.0, 1.0);   // red
  auto* visAlCase = MakeVis(0.8, 0.8, 0.8, 0.15);  // transparent-ish
  
  for (auto* lv : *store) {
      const auto& n = lv->GetName();
  
      // Adapt these substrings to your actual LV names:
      if (n.find("WidePVT") != std::string::npos) {
          lv->SetVisAttributes(visWide);
      } else if (n.find("ThinPS") != std::string::npos) {
          lv->SetVisAttributes(visThin);
      } else if (n.find("Fibre") != std::string::npos || n.find("F") != std::string::npos) {
          lv->SetVisAttributes(visFibre);
      } else if (n.find("Lead") != std::string::npos || n.find("Pb") != std::string::npos) {
          lv->SetVisAttributes(visLead);
      } else if (n.find("Iron") != std::string::npos || n.find("Fe") != std::string::npos) {
          lv->SetVisAttributes(visIron);
      } else if (n.find("HPL") != std::string::npos && n.find("Al") != std::string::npos) {
          lv->SetVisAttributes(visAlCase);
      }
  }



  return pvWorld;
}


// helper
static G4VisAttributes* MakeVis(double r, double g, double b, double a=1.0, bool solid=true) {
    auto* vis = new G4VisAttributes(G4Colour(r, g, b, a));
    vis->SetVisibility(true);
    vis->SetForceSolid(solid);
    return vis;
}
