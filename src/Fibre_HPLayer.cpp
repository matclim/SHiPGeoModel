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

using namespace GeoModelKernelUnits;
void Fibre_HPLayer::build(GeoVPhysVol* mother,
                          GeoMaterial* aluminumMat,
                          GeoMaterial* fiberMat,
                          double zCenter_mm,
                          int layerIndex,
                          double casingXY_mm,
                          double casingZ_mm,
                          double fiberDiam_mm,
                          bool fibresAlongY)
{
  // --- casing dimensions ---
  const double casingXY = casingXY_mm * mm;
  const double casingZ  = casingZ_mm  * mm;

  // Build casing (one physvol per layer)
  auto* casingShape = new GeoBox(0.5*casingXY, 0.5*casingXY, 0.5*casingZ);
  auto* casingLog   = new GeoLogVol("HPL_CasingLog", casingShape, aluminumMat);
  
  auto* casingPhys  = new GeoPhysVol(casingLog);

  

  mother->add(new GeoNameTag(("HPL_" + std::to_string(layerIndex)).c_str()));
  mother->add(new GeoTransform(GeoTrf::Translate3D(0, 0, zCenter_mm * mm)));
  mother->add(casingPhys);

  // --- fibre geometry ---
  const double r = 0.5 * fiberDiam_mm * mm;
  const double fiberLen = casingXY; // 2160 mm along Y (your requirement)
  const double halfLen = 0.5 * fiberLen;

  // If fibresAlongY: rotate +90° about X -> Z becomes Y.
  // Else (fibres along X): rotate -90° about Y -> Z becomes X.
  GeoTrf::Transform3D rotAxis = GeoTrf::Transform3D::Identity();
  if (fibresAlongY) {
    rotAxis = GeoTrf::RotateX3D(90.0 * deg);     // Z -> Y
  } else {
    rotAxis = GeoTrf::RotateY3D(-90.0 * deg);    // Z -> X
  }

  // Reuse a single logical volume for all fibres in this layer
  auto* fiberShape = new GeoTube(0.0, r, halfLen);
  auto* fiberLog   = new GeoLogVol("HPL_FiberLog", fiberShape, fiberMat);
  // --- placement: 3 sublayers, 1800 fibres each ---
  const int    nFib = 1800;
  const double pitch = fiberDiam_mm * mm;      // tight stack
  const double x0 = -0.5*(nFib - 1) * pitch;   // centered

  // Sublayer offsets:
  // 0 and 2 aligned, 1 shifted by +0.5*d in X.
  const double dx[3] = {0.0, 0.5*pitch, 0.0};

  // Z separation between sublayers:
  // simplest “stacked” is pitch; this matches “3 sublayers” literally.
  // (If later you want true hex close packing, you’d use pitch*sqrt(3)/2.)
  const double dz = pitch;
  const double zLocal[3] = {-dz, 0.0, +dz};

  for (int s = 0; s < 3; ++s) {
    for (int i = 0; i < nFib; ++i) {
      const double x = x0 + i * pitch + dx[s];
      const double z = zLocal[s];

      auto* fiberPhys = new GeoPhysVol(fiberLog);

      // Optional: NameTags are heavy, but very useful for debugging.
      // If gmex gets sluggish, remove NameTags for fibers.
      casingPhys->add(new GeoNameTag(
        ("HPL" + std::to_string(layerIndex) + "_S" + std::to_string(s) + "_F" + std::to_string(i)).c_str()
      ));
      // If fibres run along Y, we pack them in X (x varies).
      // If fibres run along X, we pack them in Y (y varies).
      const double xPos = fibresAlongY ? x : 0.0;
      const double yPos = fibresAlongY ? 0.0 : x;  // reuse "x" as the packing coordinate

      casingPhys->add(new GeoTransform(
        GeoTrf::Translate3D(xPos, yPos, z) * rotAxis
      ));
      casingPhys->add(fiberPhys);
    }
  }
}
