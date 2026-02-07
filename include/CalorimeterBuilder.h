#pragma once
#include "CalorimeterConfig.h"

class GeoVPhysVol;
class MaterialManager;

class CalorimeterBuilder {
public:
  static void buildStack(GeoVPhysVol* world, MaterialManager& MM, const CalorimeterConfig& cfg);
};
