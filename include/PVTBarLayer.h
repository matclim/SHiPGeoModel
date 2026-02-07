#pragma once

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"

class PVTBarLayer {
public:
    // Build 36 bars on top of a 2160x2160 mm plate
    static void build(GeoVPhysVol* mother, GeoLogVol* barLog, double zCenter_mm, int layerIndex);
};
