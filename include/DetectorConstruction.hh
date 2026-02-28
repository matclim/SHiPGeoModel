#pragma once

#include "G4VUserDetectorConstruction.hh"
#include <string>
#include "RunConfig.hh"
#include <memory>
class Geo2G4AssemblyVolume;



class EventStore;

// Forward declarations from GeoModel
class GeoPhysVol;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction(EventStore* store, std::string cfgFile, int write_gdml);
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;

private:
  EventStore*  m_store = nullptr;
  std::string  m_cfgFile;
  int          write_gdml;

  Geo2G4AssemblyVolume* m_assembly = nullptr;
  GeoPhysVol*  m_geoWorld = nullptr;  // GeoModel world built in memory

  GeoPhysVol* buildGeoModelWorld();   // builds from calo.cfg
};
