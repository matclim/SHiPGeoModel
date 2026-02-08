#pragma once
#include <cstddef>

#include <vector>

struct CalorimeterConfig {
  std::vector<int> layers; // sequence of codes

  double plate_xy_mm = 2160.0;
  double lead_thickness_mm = 3.0;
  double scint_thickness_mm = 10.0;

  bool center_stack = true;
  double airgap_mm = 1000.0;

  double hpl_thickness_mm = 50.0;
  double fiber_diameter_mm = 1.2;
};
