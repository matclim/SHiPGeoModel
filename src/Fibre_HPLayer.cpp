#include "Fibre_HPLayer.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"

#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace GeoModelKernelUnits;

void Fibre_HPLayer::build(GeoVPhysVol* mother,
                          GeoMaterial* aluminumMat,
                          GeoMaterial* fiberMat,
                          std::string layering,
                          double zCenter_mm,
                          int layerIndex,
                          double casingXY_mm,
                          double casingZ_mm,
                          double fiberDiam_mm,
                          double fiberCoreDiam_mm,
                          bool fibresAlongY,
                          const std::string& nameSuffix)
{
  // --- casing dimensions ---
  const double casingXY = casingXY_mm * mm;
  const double casingZ  = casingZ_mm  * mm;

  // --- build casing ---
  auto* casingShape = new GeoBox(0.5*casingXY, 0.5*casingXY, 0.5*casingZ);
  auto* casingLog   = new GeoLogVol("HPL_CasingLog", casingShape, aluminumMat);
  auto* casingPhys  = new GeoPhysVol(casingLog);

  mother->add(new GeoNameTag((layering + "_HPL_Casing" + nameSuffix).c_str()));
  mother->add(new GeoTransform(GeoTrf::Translate3D(0, 0, zCenter_mm * mm)));
  mother->add(casingPhys);

  // --- fibre geometry ---
  const double rOuter = 0.5 * fiberDiam_mm * mm;
  const double rCore  = 0.5 * fiberCoreDiam_mm * mm;

  if (rCore <= 0.0 || rCore > rOuter) {
    throw std::runtime_error("Fibre_HPLayer: invalid core diameter (core must be >0 and <= outer)");
  }

  // Fibres run along casingXY; GeoTube axis is Z by default
  const double halfLen = 0.5 * casingXY;

  // rotate tube axis to Y or X
  GeoTrf::Transform3D rotAxis = GeoTrf::Transform3D::Identity();
  if (fibresAlongY) rotAxis = GeoTrf::RotateX3D( 90.0 * deg);   // Z -> Y
  else              rotAxis = GeoTrf::RotateY3D(-90.0 * deg);   // Z -> X

  auto* cladShape = new GeoTube(0.0, rOuter, halfLen);
  auto* coreShape = new GeoTube(0.0, rCore,  halfLen);

  // LV names: cladding should NOT contain "Fiber" if you match "FiberCore" for sensitivity later
  auto* cladLog = new GeoLogVol("HPL_CladLog",      cladShape, fiberMat);
  auto* coreLog = new GeoLogVol("HPL_FiberCoreLog", coreShape, fiberMat);

  // --- tiling along packing coordinate ---
  const double pitch = 2.0 * rOuter;      // tight packing (center-to-center)
  const double dxMax = 0.5 * pitch;       // middle sublayer shift

  const double usable = casingXY - 2.0*rOuter - dxMax;
  const int nFib = std::max(1, int(std::floor(usable / pitch)) + 1);

  // center the bundle; include dxMax so shifted layer still fits
  const double x0 = -0.5 * ((nFib - 1) * pitch + dxMax);

  const double dx[3] = {0.0, 0.5*pitch, 0.0};

  // --- sublayer Z placement (keep inside casingZ) ---
  // Put outer sublayers as far as possible without crossing the casing boundary
  const double dz = std::max(0.0, (0.5*casingZ - rOuter));
  const double zLocal[3] = {-dz, 0.0, +dz};

  const std::string orient = fibresAlongY ? "V_L" : "H_L";

  // --- place fibres ---
  for (int s = 0; s < 3; ++s) {
    for (int i = 0; i < nFib; ++i) {
      const double pack = x0 + i * pitch + dx[s];
      const double z    = zLocal[s];

      const std::string baseName =
        layering + "_HPL_" + orient + std::to_string(layerIndex) +
        "_S" + std::to_string(s) +
        "_F" + std::to_string(i) +
        nameSuffix;

      // cladding PV
      auto* cladPhys = new GeoPhysVol(cladLog);

      // core PV inside cladding PV (same axis, no transform)
      cladPhys->add(new GeoNameTag((baseName + "_Core").c_str()));
      cladPhys->add(new GeoPhysVol(coreLog));

      // name the cladding itself too (useful for debugging)
      casingPhys->add(new GeoNameTag((baseName + "_Clad").c_str()));

      const double xPos = fibresAlongY ? pack : 0.0;
      const double yPos = fibresAlongY ? 0.0  : pack;

      casingPhys->add(new GeoTransform(GeoTrf::Translate3D(xPos, yPos, z) * rotAxis));
      casingPhys->add(cladPhys);
    }
  }
}
