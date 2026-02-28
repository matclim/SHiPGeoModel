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
  double fiber_core_diameter_mm = -1.0; // default: if <0 use fiber_diameter_mm
  
  std::vector<int> layers2;
  double iron_thickness_mm = 170.0;

  double gap_ecal_hcal_mm = 0;

  int module_nx = 1;
  int module_ny = 1;
  
  double module_pitch_x_mm = 0.0; // if 0 -> use plate_xy_mm
  double module_pitch_y_mm = 0.0; // if 0 -> use plate_xy_mm

  double tol_x_mm = 10;
  double tol_y_mm = 10;
  double tol_z_mm = 10;
};
