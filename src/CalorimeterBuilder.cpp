#include "CalorimeterBuilder.h"
#include "MaterialManager.h"
#include "PVTBarLayer.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"

#include <string>

using namespace GeoModelKernelUnits;

void CalorimeterBuilder::buildStack(GeoVPhysVol* world, MaterialManager& MM, const CalorimeterConfig& cfg)
{
  auto* pb  = MM.lead();
  auto* pvt = MM.pvt();

  const double plateX = cfg.plate_xy_mm * mm;
  const double plateY = cfg.plate_xy_mm * mm;
  const double plateZ = cfg.lead_thickness_mm * mm;
  const double pvtZ   = cfg.pvt_thickness_mm  * mm;

  const double barX = 60.0 * mm;
  const double barY = cfg.plate_xy_mm * mm;  // 2160 mm
  const double barZ = cfg.pvt_thickness_mm * mm;
  
  auto barShape = new GeoBox(0.5*barX, 0.5*barY, 0.5*barZ);
  auto barLog   = new GeoLogVol("PVTBarLog", barShape, pvt);


  // Reusable logvol for Pb plates
  auto plateShape = new GeoBox(0.5*plateX, 0.5*plateY, 0.5*plateZ);
  auto plateLog   = new GeoLogVol("LeadPlateLog", plateShape, pb);

  // total thickness for centering (interleaved up to min(n_lead, n_pvt) plus leftovers)
  const int nPairs = std::min(cfg.n_lead, cfg.n_pvt);
  const int nLeadExtra = cfg.n_lead - nPairs;
  const int nPvtExtra  = cfg.n_pvt  - nPairs;

  const double totalZ =
      nPairs * (plateZ + pvtZ) +
      nLeadExtra * plateZ +
      nPvtExtra  * pvtZ;

  double zCursor = cfg.center_stack ? -0.5 * totalZ : 0.0;

  int iLead = 0, iPvt = 0;

  // Interleave pairs
  for (int i = 0; i < nPairs; ++i) {
    // Pb
    {
      auto platePhys = new GeoPhysVol(plateLog);
      world->add(new GeoNameTag(("LeadPlate_" + std::to_string(iLead)).c_str()));
      world->add(new GeoTransform(GeoTrf::Translate3D(0, 0, zCursor + 0.5*plateZ)));
      world->add(platePhys);
      zCursor += plateZ;
      ++iLead;
    }

    // PVT
    {
      const double zCenter = zCursor + 0.5 * pvtZ;
      PVTBarLayer::build(world, barLog, zCenter / mm, iPvt);
      zCursor += pvtZ;
      ++iPvt;
    }
  }

  // Leftover Pb
  for (; iLead < cfg.n_lead; ++iLead) {
    auto platePhys = new GeoPhysVol(plateLog);
    world->add(new GeoNameTag(("LeadPlate_" + std::to_string(iLead)).c_str()));
    world->add(new GeoTransform(GeoTrf::Translate3D(0, 0, zCursor + 0.5*plateZ)));
    world->add(platePhys);
    zCursor += plateZ;
  }

  // Leftover PVT
  for (; iPvt < cfg.n_pvt; ++iPvt) {
    const double zCenter = zCursor + 0.5 * pvtZ;
    PVTBarLayer::build(world, pvt, zCenter / mm);
    zCursor += pvtZ;
  }
}
