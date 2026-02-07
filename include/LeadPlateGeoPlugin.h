#pragma once

#include "GeoModelKernel/GeoVGeometryPlugin.h"

class LeadPlateGeoPlugin : public GeoVGeometryPlugin
{
public:
  LeadPlateGeoPlugin() = default;
  ~LeadPlateGeoPlugin() override = default;

  void create(GeoVPhysVol* world, bool publish = false) override;

  LeadPlateGeoPlugin(const LeadPlateGeoPlugin&) = delete;
  LeadPlateGeoPlugin& operator=(const LeadPlateGeoPlugin&) = delete;
};
