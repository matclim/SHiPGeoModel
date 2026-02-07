#pragma once
#include <cstddef>

struct CalorimeterConfig {
  int n_lead = 40;
  int n_pvt  = 40;

  double plate_xy_mm = 2160.0;
  double lead_thickness_mm = 3.0;
  double pvt_thickness_mm  = 10.0;

  bool center_stack = true; // nice default
};
