#include "PVTBarLayer.h"
#include <string>
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"

using namespace GeoModelKernelUnits;

void PVTBarLayer::build(GeoVPhysVol* mother, GeoLogVol* barLog, double zCenter_mm, int layerIndex)
{
  const double barX = 60.0 * mm;
  const int nBars = 36;
  const double x0 = -0.5 * (nBars - 1) * barX;

  for (int i = 0; i < nBars; ++i) {
    const double x = x0 + i * barX;
    auto barPhys = new GeoPhysVol(barLog);
    mother->add(new GeoNameTag(("PVT_L" + std::to_string(layerIndex) + "_B" + std::to_string(i)).c_str()));
    mother->add(new GeoTransform(GeoTrf::Translate3D(x, 0.0, zCenter_mm * mm)));
    mother->add(barPhys);
  }
}
