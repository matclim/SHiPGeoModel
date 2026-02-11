#pragma once

class GeoVPhysVol;
class GeoLogVol;
class GeoMaterial;

class Fibre_HPLayer {
public:
  // Builds one HP layer centered at zCenter_mm in the mother volume.
  // Fibres assumed to run along +Y.
static void build(GeoVPhysVol* mother,
                  GeoMaterial* aluminumMat,
                  GeoMaterial* fiberMat,
                  double zCenter_mm,
                  int layerIndex,
                  double casingXY_mm,
                  double casingZ_mm,
                  double fiberDiam_mm,
                  bool fibresAlongY = true);

};
