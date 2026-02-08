#include "CalorimeterBuilder.h"
#include "MaterialManager.h"
#include "BarLayer.h"
#include "Fibre_HPLayer.h"

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
  auto* leadMat = MM.lead();
  auto* pvtMat  = MM.pvt();
  auto* psMat   = MM.polystyrene();

  const double plateXY = cfg.plate_xy_mm * mm;
  const double leadZ   = cfg.lead_thickness_mm * mm;
  const double scintZ  = cfg.scint_thickness_mm * mm;
  const double airGapZ = cfg.airgap_mm * mm;
 
  const double wideW = 60.0 * mm;
  const double thinW = 10.0 * mm;

  const double hplZ    = cfg.hpl_thickness_mm * mm;

  // Wide PVT horizontal (replicate along X)
  auto* wideHShape = new GeoBox(0.5*wideW, 0.5*plateXY, 0.5*scintZ);
  auto* wideHLog   = new GeoLogVol("WidePVT_H_Log", wideHShape, pvtMat);
  
  // Wide PVT vertical (replicate along Y)
  auto* wideVShape = new GeoBox(0.5*plateXY, 0.5*wideW, 0.5*scintZ);
  auto* wideVLog   = new GeoLogVol("WidePVT_V_Log", wideVShape, pvtMat);
  
  // Thin PS horizontal
  auto* thinHShape = new GeoBox(0.5*thinW, 0.5*plateXY, 0.5*scintZ);
  auto* thinHLog   = new GeoLogVol("ThinPS_H_Log", thinHShape, psMat);
  
  // Thin PS vertical
  auto* thinVShape = new GeoBox(0.5*plateXY, 0.5*thinW, 0.5*scintZ);
  auto* thinVLog   = new GeoLogVol("ThinPS_V_Log", thinVShape, psMat);



  // Reusable logvols
  auto* leadShape = new GeoBox(0.5*plateXY, 0.5*plateXY, 0.5*leadZ);
  auto* leadLog   = new GeoLogVol("LeadPlateLog", leadShape, leadMat);

  auto* wideShape = new GeoBox(0.5*(60.0*mm), 0.5*plateXY, 0.5*scintZ);
  auto* wideLog   = new GeoLogVol("WidePVTBarLog", wideShape, pvtMat);

  auto* thinShape = new GeoBox(0.5*(10.0*mm), 0.5*plateXY, 0.5*scintZ);
  auto* thinLog   = new GeoLogVol("ThinPSBarLog", thinShape, psMat);

  // Total thickness for centering
  double totalZ = 0.0;
  for (int code : cfg.layers) {
    if (code == 7) totalZ += leadZ;
    else if (code == 1 || code == 2 || code == 3 || code == 4) totalZ += scintZ;
    else if (code == 5) totalZ += hplZ;
    else if (code == 8) totalZ += airGapZ;
    else throw std::runtime_error("Unknown layer code: " + std::to_string(code));
  }
  
  int iWideH=0, iWideV=0, iThinH=0, iThinV=0, iLead=0, iGap=0, iHPL=0;
  
  double zCursor = cfg.center_stack ? -0.5 * totalZ : 0.0;


  for (int code : cfg.layers) {
    if (code == 7) {
      auto* platePhys = new GeoPhysVol(leadLog);
      world->add(new GeoNameTag(("Lead_" + std::to_string(iLead)).c_str()));
      world->add(new GeoTransform(GeoTrf::Translate3D(0, 0, zCursor + 0.5*leadZ)));
      world->add(platePhys);
      zCursor += leadZ;
      ++iLead;
    }
    else if (code == 1) {
      const double zCenter = zCursor + 0.5*scintZ;
      BarLayer::place(world, wideHLog, 60.0, 36, zCenter/mm, "WidePVT_H", iWideH, BarAxis::AlongX);
      zCursor += scintZ; ++iWideH;
    }
    else if (code == 2) {
      const double zCenter = zCursor + 0.5*scintZ;
      BarLayer::place(world, wideVLog, 60.0, 36, zCenter/mm, "WidePVT_V", iWideV, BarAxis::AlongY);
      zCursor += scintZ; ++iWideV;
    }
    else if (code == 3) {
      const double zCenter = zCursor + 0.5*scintZ;
      BarLayer::place(world, thinHLog, 10.0, 216, zCenter/mm, "ThinPS_H", iThinH, BarAxis::AlongX);
      zCursor += scintZ; ++iThinH;
    }
    else if (code == 4) {
      const double zCenter = zCursor + 0.5*scintZ;
      BarLayer::place(world, thinVLog, 10.0, 216, zCenter/mm, "ThinPS_V", iThinV, BarAxis::AlongY);
      zCursor += scintZ; ++iThinV;
    }
    else if (code == 5) {
      const double zCenter = zCursor + 0.5*hplZ;
      Fibre_HPLayer::build(world,
                           MM.air(),
                           MM.polystyrene(),
                           zCenter/mm,
                           iHPL,
                           cfg.plate_xy_mm,
                           cfg.hpl_thickness_mm,
                           cfg.fiber_diameter_mm);
      zCursor += hplZ;
      ++iHPL;
    }
    else if (code == 8) {
      // airgap: no volume needed; just advance z
      zCursor += airGapZ;
      ++iGap;
    }
  }
}
