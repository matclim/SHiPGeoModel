#pragma once
#include "CalorimeterConfig.h"

class GeoVPhysVol;
class MaterialManager;

class CalorimeterBuilder {
public:
  static void buildStack(GeoVPhysVol* world, MaterialManager& MM, const CalorimeterConfig& cfg, int mx=1, int my=1);
  static double totalThickness_mm(const CalorimeterConfig& cfg);
};
