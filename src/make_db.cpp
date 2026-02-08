#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/Units.h"

#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"

#include "LeadPlateGeoPlugin.h"
#include "MaterialManager.h"
#include "ConfigReader.h"
#include "CalorimeterBuilder.h"



#include <fstream>
#include <iomanip>

using namespace GeoModelKernelUnits;

static GeoPhysVol* createWorld()
{
  MaterialManager MM;

  auto* air_mat = MM.air();
  // World size: generous margin around the 2.16 m plate
  const double worldXY = 3.0 * m;   // full size
  const double worldZ  = 3.0 * m;   // full size

  auto worldShape = new GeoBox(0.5 * worldXY, 0.5 * worldXY, 0.5 * worldZ);
  auto worldLog   = new GeoLogVol("WorldLog", worldShape, air_mat);
  return new GeoPhysVol(worldLog);
}

static GeoPhysVol* createWorld(MaterialManager& MM)
{
    auto* air_mat = MM.air();

    const double worldXY = 3.0 * m;
    const double worldZ  = 3.0 * m;

    auto worldShape = new GeoBox(0.5 * worldXY, 0.5 * worldXY, 0.5 * worldZ);
    auto worldLog   = new GeoLogVol("WorldLog", worldShape, air_mat);
    auto worldPhys  = new GeoPhysVol(worldLog);

    return worldPhys;
}


static void writeGmexMatVisJSON(const std::string& filename, const MaterialManager& MM){
  auto dump = [&](const std::string& n){
    auto c = MM.rgbaFor(n); // std::array<double,4> {r,g,b,a}
    return std::string("    { \"name\": \"") + n +
           "\", \"diffuse\": [" + std::to_string(c[0]) + "," + std::to_string(c[1]) + "," +
                              std::to_string(c[2]) + "], \"opacity\": " + std::to_string(c[3]) + " }";
  };


  std::ofstream out(filename);
  out << "{\n";
  out << "  \"materials\": [\n";
  out << dump("Lead") << ",\n";
  out << dump("Iron") << ",\n";
  out << dump("PVT") << ",\n";
  out << dump("Polystyrene") << ",\n";
  out << dump("Aluminium") << ",\n";
  out << dump("Air") << "\n";
  out << "  ]\n";
  out << "}\n";
}


int main(int argc, char** argv)
{
  const std::string outFile = (argc > 1) ? argv[1] : "geometry.db";
  const std::string cfgFile = (argc > 2) ? argv[2] : "../calo.cfg";

  auto cfg = readConfigFile(cfgFile);

  MaterialManager MM;

  GeoPhysVol* world = createWorld(MM); // change createWorld to take mm and use mm.air()

  CalorimeterBuilder::buildStack(world, MM, cfg);


  // 2) Persistify to SQLite .db
  GMDBManager db(outFile);
  if (!db.checkIsDBOpen()) {
    std::cerr << "ERROR: could not open DB file: " << outFile << "\n";
    return 1;
  }

  GeoModelIO::WriteGeoModel dumper(db);
  world->exec(&dumper);
  dumper.saveToDB();

  writeGmexMatVisJSON("gmexMatVisAttributes.local.json", MM);

  std::cout << "Wrote GeoModel geometry to: " << outFile << "\n";
  std::cout << "Now run: gmex " << outFile << "\n";
  return 0;
}
