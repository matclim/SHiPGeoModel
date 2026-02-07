#include "ConfigReader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

static inline std::string trim(std::string s) {
  auto notSpace = [](unsigned char c){ return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

CalorimeterConfig readConfigFile(const std::string& path)
{
  CalorimeterConfig cfg;

  std::ifstream in(path);
  if (!in) throw std::runtime_error("Could not open config file: " + path);

  std::string line;
  while (std::getline(in, line)) {
    // strip comments
    auto hash = line.find('#');
    if (hash != std::string::npos) line = line.substr(0, hash);
    line = trim(line);
    if (line.empty()) continue;

    auto eq = line.find('=');
    if (eq == std::string::npos) continue;

    auto key = trim(line.substr(0, eq));
    auto val = trim(line.substr(eq + 1));

    if      (key == "n_lead") cfg.n_lead = std::stoi(val);
    else if (key == "n_pvt")  cfg.n_pvt  = std::stoi(val);

    else if (key == "plate_xy_mm")        cfg.plate_xy_mm = std::stod(val);
    else if (key == "lead_thickness_mm")  cfg.lead_thickness_mm = std::stod(val);
    else if (key == "pvt_thickness_mm")   cfg.pvt_thickness_mm  = std::stod(val);

    else if (key == "center_stack") {
      std::string v = val;
      std::transform(v.begin(), v.end(), v.begin(), ::tolower);
      cfg.center_stack = (v=="1" || v=="true" || v=="yes" || v=="on");
    }
  }

  // basic sanity
  if (cfg.n_lead < 0 || cfg.n_pvt < 0) throw std::runtime_error("n_lead/n_pvt must be >= 0");
  return cfg;
}
