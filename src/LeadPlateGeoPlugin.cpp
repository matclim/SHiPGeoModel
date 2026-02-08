#include "LeadPlateGeoPlugin.h"
#include "PVTBarLayer.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"

#include "MaterialManager.h"

using namespace GeoModelKernelUnits;

void LeadPlateGeoPlugin::create(GeoVPhysVol* world, bool /*publish*/)
{
  // --- Lead material ---
  MaterialManager MM;

  auto* pb_mat  = MM.lead();
  auto* pvt_mat = MM.pvt();


  // --- Lead plate: 2160 x 2160 x 3 mm ---
  const double plateX = 2160.0 * mm;
  const double plateY = 2160.0 * mm;
  const double plateZ = 3.0   * mm;

  auto plateShape = new GeoBox(0.5 * plateX, 0.5 * plateY, 0.5 * plateZ);
  auto plateLog   = new GeoLogVol("LeadPlateLog", plateShape, pb_mat);
  auto platePhys  = new GeoPhysVol(plateLog);

  // Place plate at origin
  world->add(new GeoNameTag("LeadPlate"));
  world->add(new GeoTransform(GeoTrf::Translate3D(0, 0, 0)));
  world->add(platePhys);

  // --- Bars sit directly on top of the plate ---
  const double plateHalfZ = 0.5 * plateZ;      // 1.5 mm
  const double barHalfZ   = 0.5 * (10.0 * mm); // 5 mm  (since barZ=10mm)
  const double zBarCenter = plateHalfZ + barHalfZ; // 6.5 mm

  PVTBarLayer::build(world, barLog, zBarCenter / mm, iPvt);
}
