#pragma once
#include <string>
class GeoVPhysVol;
class GeoLogVol;

enum class BarAxis { AlongX, AlongY }; // which axis we replicate bars along

class BarLayer {
public:
  static void place(GeoVPhysVol* mother,
                    GeoLogVol*   barLog,
                    double       pitch_mm,
                    int          nBars,
                    double       zCenter_mm,
                    const char*  tagPrefix,
                    int          layerIndex,
                    BarAxis      axis,
                    const std::string& nameSuffix = ""
                    );
};
