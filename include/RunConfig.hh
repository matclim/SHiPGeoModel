#pragma once
#include <string>
#include <array>

struct RunConfig {
  int n_events = 0;

  std::string particle = "e-";
  double energy_MeV = 1000.0;

  std::array<double,3> position_mm  {0.0, 0.0, -2500.0};
  std::array<double,3> direction    {0.0, 0.0, 1.0};
  double sigma_xy_mm = 0.0;
  bool getBool(const std::string& key, bool def=false) const;
  std::string getString(const std::string& key, const std::string& def="") const;


  std::string macro = "";
  long seed = 0;

  bool visualize = false;
  std::string vis_macro = "../vis.mac";

};

RunConfig readRunConfigFile(const std::string& path);
