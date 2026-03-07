#include "CaloSysDigitiser.hh"

CaloSysDigitiser::CaloSysDigitiser(){}

CaloSysDigitiser::~CaloSysDigitiser(){}

int CaloSysDigitiser::GeneratePEDCnoise() {
    double u = rng.Uniform(); // Uniform random number in [0, 1)
    double F = 0.9; // F(0)
    int k = 0;
    while (u > F) {
        k++;
        F += (9.0 / 10.0) * (1.0 / TMath::Power(10, k));
    }
    return k;
}

void CaloSysDigitiser::HexantUnpacker(ULong64_t entry){
  hexantX= static_cast<int>(v_hexant->at(entry)/10);
  hexantY= static_cast<int>(v_hexant->at(entry)%10);
}


// ---------------------------------------------------------------------------
// Grid-based coordinate snapping
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Two grid builders with different anchoring strategies:
//
//  BuildFineGrid  — centres at (n + 0.5)*pitch  (e.g. 30, 90, 150 mm for 60 mm bars)
//                   Bar edges lie on exact multiples of pitch; the centre of bar n
//                   is therefore at (n + 0.5)*pitch.  Used for the position-sensitive
//                   (fine) axis of each bar type.
//
//  BuildCoarseGrid — centres at n*pitch  (e.g. -2160, 0, +2160 mm for 2160 mm hexants)
//                   One centre per hexant, sitting at the hexant midpoint.  Used for
//                   the along-bar (coarse) axis where only hexant identity matters.
// ---------------------------------------------------------------------------

std::vector<double> CaloSysDigitiser::BuildFineGrid(double global_min,
                                                      double global_max,
                                                      double pitch) const
{
    std::vector<double> grid;
    // First centre: (n+0.5)*pitch for the lowest n that places it inside the range
    double first = std::floor(global_min / pitch) * pitch + 0.5 * pitch;
    for (double c = first; c <= global_max + 0.5 * pitch; c += pitch)
        grid.push_back(c);
    return grid;
}

std::vector<double> CaloSysDigitiser::BuildCoarseGrid(double global_min,
                                                        double global_max,
                                                        double pitch) const
{
    std::vector<double> grid;
    // First centre: n*pitch (integer multiples) — hexant centres at 0, ±2160, etc.
    double first = std::ceil(global_min / pitch) * pitch;
    for (double c = first; c <= global_max + 0.5 * pitch; c += pitch)
        grid.push_back(c);
    return grid;
}

// BuildGrid is kept as an alias for BuildFineGrid for backward compatibility.
std::vector<double> CaloSysDigitiser::BuildGrid(double global_min,
                                                 double global_max,
                                                 double pitch) const
{
    return BuildFineGrid(global_min, global_max, pitch);
}

// Snap a global coordinate to the nearest bin centre using binary search.
double CaloSysDigitiser::SnapToGrid(double coord,
                                     const std::vector<double>& grid) const
{
    if (grid.empty()) return coord;
    // lower_bound finds first element >= coord
    auto it = std::lower_bound(grid.begin(), grid.end(), coord);
    if (it == grid.end())   return grid.back();
    if (it == grid.begin()) return grid.front();
    // Choose the closer of the two neighbours
    auto prev = std::prev(it);
    return (coord - *prev) <= (*it - coord) ? *prev : *it;
}

// Called once from Initialise() to build the four permanent grids.
// Detector layout:
//   X : nhexants_X=2 hexants  →  total width  = 2 * size_bar_L
//   Y : nhexants_Y=3 hexants  →  total height = 3 * size_bar_L
// Global coordinates are centred on 0, so the ranges are:
//   X : [ -nhexants_X/2 * size_bar_L ,  nhexants_X/2 * size_bar_L ]
//   Y : [ -nhexants_Y/2 * size_bar_L ,  nhexants_Y/2 * size_bar_L ]
void CaloSysDigitiser::BuildCoordinateGrids()
{
    const double x_min = -(nhexants_X / 2.0) * size_bar_L;
    const double x_max =  (nhexants_X / 2.0) * size_bar_L;
    const double y_min = -(nhexants_Y / 2.0) * size_bar_L;
    const double y_max =  (nhexants_Y / 2.0) * size_bar_L;

    // Fine grid  : bar width (60 mm wide / 10 mm thin) — resolves position
    // Coarse grid: bar length (2160 mm = one full hexant) — resolves only
    //              which hexant column/row the bar occupies
    // Fine grids: bar centres at (n+0.5)*pitch — edges on integer multiples of pitch
    grid_wide_fine_x  = BuildFineGrid(x_min, x_max, size_bar_W);      //   60 mm pitch in X
    grid_wide_fine_y  = BuildFineGrid(y_min, y_max, size_bar_W);      //   60 mm pitch in Y
    grid_thin_fine_x  = BuildFineGrid(x_min, x_max, size_thin_bar_W); //   10 mm pitch in X
    grid_thin_fine_y  = BuildFineGrid(y_min, y_max, size_thin_bar_W); //   10 mm pitch in Y

    // Coarse grids: hexant centres at n*pitch (0, ±2160 mm) — one centre per hexant
    grid_wide_coarse_x  = BuildCoarseGrid(x_min, x_max, size_bar_L);  // 2160 mm pitch in X
    grid_wide_coarse_y  = BuildCoarseGrid(y_min, y_max, size_bar_L);  // 2160 mm pitch in Y
    grid_thin_coarse_x  = BuildCoarseGrid(x_min, x_max, size_bar_L);  // 2160 mm pitch in X
    grid_thin_coarse_y  = BuildCoarseGrid(y_min, y_max, size_bar_L);  // 2160 mm pitch in Y
}

void CaloSysDigitiser::SetFile(std::string file_location_str){
  ff_data = TFile::Open(file_location_str.c_str(),"READ"); 
  ff_data->GetObject("calo_events",ft_data);
}

//Order:
//1. Initialise, get files, TTrees, Branches...
//2. Loop over event
//3. Get energy deposition -> convert into a prompt photon count
//4. Prompt photon count is converted into fibre-captured photon count
//5. Fibre-captured photon count is converted into attenuated photon count
//6. Fibre-captured photon count is converted into escaping photon count
//7. Escaping photon count is converted into PE
//8. PE is converted into ADC

void CaloSysDigitiser::Initialise(){

  ft_data->SetBranchAddress("edep",&v_edep);
  ft_data->SetBranchAddress("x_global",&v_x_global);
  ft_data->SetBranchAddress("y_global",&v_y_global);
  ft_data->SetBranchAddress("z_global",&v_z_global);
  ft_data->SetBranchAddress("x_local",&v_x_local);
  ft_data->SetBranchAddress("y_local",&v_y_local);
  ft_data->SetBranchAddress("z_local",&v_z_local);
  ft_data->SetBranchAddress("type",&v_type);
  ft_data->SetBranchAddress("section",&v_section);
  ft_data->SetBranchAddress("layer",&v_layer);
  ft_data->SetBranchAddress("vol",&v_vol);
  ft_data->SetBranchAddress("hcal",&v_hcal);
  ft_data->SetBranchAddress("hpl_subsection",&v_hpl_subsection);
  ft_data->SetBranchAddress("hexant",&v_hexant);

  ff_out = TFile::Open((static_cast<std::string>("digi_")+static_cast<std::string>(ff_data->GetName())).c_str(),"RECREATE");

  ft_out = new TTree("digi_calo_events","Digitised SHiP calorimeter events");
  ft_out->Branch("v_digi_widebar_ADC_SiPM_L",&v_digi_widebar_ADC_SiPM_L);
  ft_out->Branch("v_digi_widebar_ADC_SiPM_R",&v_digi_widebar_ADC_SiPM_R);
  ft_out->Branch("v_digi_thinbar_ADC_SiPM_L",&v_digi_thinbar_ADC_SiPM_L);
  ft_out->Branch("v_digi_thinbar_ADC_SiPM_R",&v_digi_thinbar_ADC_SiPM_R);
  ft_out->Branch("v_digi_sharp_ADC_SiPM",&v_digi_sharp_ADC_SiPM);
  ft_out->Branch("v_digi_wide_orientation",&v_digi_wide_orientation);
  ft_out->Branch("v_digi_thin_orientation",&v_digi_thin_orientation);
  ft_out->Branch("v_digi_sharp_orientation",&v_digi_sharp_orientation);
  ft_out->Branch("v_digi_sharplayer",    &v_digi_sharplayer);
  ft_out->Branch("v_digi_sharp_sensorID",&v_digi_sharp_sensorID);
  ft_out->Branch("v_digi_widebar_gain_switch_L",&v_digi_widebar_gain_switch_L);
  ft_out->Branch("v_digi_widebar_gain_switch_R",&v_digi_widebar_gain_switch_R);
  ft_out->Branch("v_digi_thinbar_gain_switch_L",&v_digi_thinbar_gain_switch_L);
  ft_out->Branch("v_digi_thinbar_gain_switch_R",&v_digi_thinbar_gain_switch_R);
  ft_out->Branch("v_digi_sharp_gain_switch",&v_digi_sharp_gain_switch);
  ft_out->Branch("v_digi_widebar_H",&v_digi_widebar_H);
  ft_out->Branch("v_digi_widebar_V",&v_digi_widebar_V);
  ft_out->Branch("v_digi_thinbar_H",&v_digi_thinbar_H);
  ft_out->Branch("v_digi_thinbar_V",&v_digi_thinbar_V);
  ft_out->Branch("v_digi_widelayer", &v_digi_widelayer);
  ft_out->Branch("v_digi_thinlayer", &v_digi_thinlayer);
  ft_out->Branch("v_digi_wide_hexant",&v_digi_wide_hexant);
  ft_out->Branch("v_digi_thin_hexant",&v_digi_thin_hexant);
  ft_out->Branch("v_digi_sharp_hexant",&v_digi_sharp_hexant);
  ft_out->Branch("v_digi_wide_x",&v_digi_wide_x);
  ft_out->Branch("v_digi_wide_y",&v_digi_wide_y);
  ft_out->Branch("v_digi_wide_z",&v_digi_wide_z);
  ft_out->Branch("v_digi_thin_x",&v_digi_thin_x);
  ft_out->Branch("v_digi_thin_y",&v_digi_thin_y);
  ft_out->Branch("v_digi_thin_z",&v_digi_thin_z);
  ft_out->Branch("v_digi_sharp_x",&v_digi_sharp_x);
  ft_out->Branch("v_digi_sharp_y",&v_digi_sharp_y);
  ft_out->Branch("v_digi_sharp_z",&v_digi_sharp_z);
  ft_out->Branch("v_digi_wide_hcal",  &v_digi_wide_hcal);
  ft_out->Branch("v_digi_thin_hcal",  &v_digi_thin_hcal);
  ft_out->Branch("v_digi_sharp_hcal", &v_digi_sharp_hcal);

  // Build the global coordinate grids once so SnapToGrid() can reuse them.
  BuildCoordinateGrids();
}


void CaloSysDigitiser::ConvertWideBarPromptPhotonCount(double edep){
  n_prompt_photons = edep * cf_wb_edep_to_prompt_photons;
}

void CaloSysDigitiser::ConvertWideBarCapturedPhotonCount(){
  n_captured_photons = n_prompt_photons * cf_wb_prompt_photons_to_captured_photons;
}

void CaloSysDigitiser::ConvertWideBarAttenuatedPhotonCount(double xylocal){
  n_atten_photons = n_captured_photons * TMath::Exp(-xylocal/atten_length_bar_fibre);
}

void CaloSysDigitiser::ConvertWideBarEscapingPhotonCount(){
  n_escaping_photons = n_atten_photons * cf_wb_atten_photons_to_escaping_photons;
}

void CaloSysDigitiser::ConvertWideBarPE(){
  n_PE = n_escaping_photons * cf_wb_PDE;
}

double CaloSysDigitiser::ConvertADCCountWide(double n_PE_wnoise, bool &LG){
  double HG_ADC = cf_wb_n_PE_ADC * n_PE_wnoise + hg_pedestal;
  if(HG_ADC < 1000) return HG_ADC; 
  else {LG=1; if((lg_pedestal + HG_ADC/LG_factor) < 1023) return lg_pedestal + HG_ADC/LG_factor; else return 1023;}
}


double CaloSysDigitiser::ConvertADCCountThin(double n_PE_wnoise, bool &LG){
  double HG_ADC = cf_tb_n_PE_ADC * n_PE_wnoise + hg_pedestal;
  if(HG_ADC < 1000) return HG_ADC; 
  else {LG=1; if((lg_pedestal + HG_ADC/LG_factor) < 1023) return lg_pedestal + HG_ADC/LG_factor; else return 1023;}
}

double CaloSysDigitiser::ConvertADCCountSHARP(double n_PE_wnoise, bool &LG){
  if(n_PE_wnoise < 0) return hg_pedestal;
  double HG_ADC = cf_fibre_n_PE_ADC * n_PE_wnoise + hg_pedestal;
  if(HG_ADC < 1000) return HG_ADC; 
  else {LG=1; if((lg_pedestal + HG_ADC/LG_factor) < 1023) return lg_pedestal + HG_ADC/LG_factor; else return 1023;}
}

void CaloSysDigitiser::ConvertThinBarPromptPhotonCount(double edep){
  n_prompt_photons = edep * cf_tb_edep_to_prompt_photons;
}

void CaloSysDigitiser::ConvertThinBarCapturedPhotonCount(){
  n_captured_photons = n_prompt_photons * cf_tb_prompt_photons_to_captured_photons;
}

void CaloSysDigitiser::ConvertThinBarAttenuatedPhotonCount(double xylocal){
  n_atten_photons = n_captured_photons * TMath::Exp(-xylocal/atten_length_bar_fibre);
}

void CaloSysDigitiser::ConvertThinBarEscapingPhotonCount(){
  n_escaping_photons = n_atten_photons * cf_tb_atten_photons_to_escaping_photons;
}

void CaloSysDigitiser::ConvertThinBarPE(){
 n_PE = n_escaping_photons * cf_tb_PDE;
}

double CaloSysDigitiser::AddSiPMNoise(double PEcount){
  return rng.Poisson(PEcount) + GeneratePEDCnoise();
}

double CaloSysDigitiser::AddGeneralNoise(double PEcount){
  return rng.Gaus(PEcount,0.5);
}

void CaloSysDigitiser::ConvertFibrePromptPhotonCount(double edep){
  n_prompt_photons = edep * cf_fibre_edep_to_prompt_photons;
}

void CaloSysDigitiser::ConvertFibreAttenuatedPhotonCount(double xylocal){
    n_atten_photons = n_prompt_photons * TMath::Exp(-xylocal / atten_length_bar_fibre);
}


void CaloSysDigitiser::ConvertFibreEscapingPhotonCount(){
  n_escaping_photons = n_atten_photons * cf_fibre_atten_photons_to_escaping_photons;
}

/**
 * Area of a circle of radius r lying to the LEFT of a vertical line
 * at signed offset d from the circle centre (d = line_x - centre_x).
 */
double CaloSysDigitiser::CircleAreaLeftOfLine(double d, double r) const
{
    if (d >=  r) return 0.0;
    if (d <= -r) return M_PI * r * r;
    return r * r * std::acos(d / r) - d * std::sqrt(r * r - d * d);
}

/**
 * Fraction of a fibre cross-section overlapping a sensor strip of width
 * sensor_w centred at sensor_x, for a fibre centred at fibre_x.
 */
double CaloSysDigitiser::FibreSensorOverlapFraction(double fibre_x, double sensor_x) const
{
    const double x_left  = sensor_x - 0.5 * size_sharp_SiPM;
    const double x_right = sensor_x + 0.5 * size_sharp_SiPM;
    double overlap = CircleAreaLeftOfLine(x_left  - fibre_x, r_sharp_fibre)
                   - CircleAreaLeftOfLine(x_right - fibre_x, r_sharp_fibre);
    return overlap / (M_PI * r_sharp_fibre * r_sharp_fibre);
}

double CaloSysDigitiser::GetFibreCentre(double global_coord, int sublayer) const
{
    // Sublayer 1 is staggered by one fibre radius relative to sublayers 0 and 2
    double stagger = (sublayer == 1) ? r_sharp_fibre : 0.0;
    return global_coord + stagger;
}


/**
 * For a fibre at global coordinate fibre_x with n_escaping_photons,
 * distributes photons across overlapping sensors and pushes results
 * into the SHARP ADC vectors.
 *
 * Sensor centres are on a regular size_sharp_sipm grid, aligned to 0.
 */
void CaloSysDigitiser::MapFibreToSensors(double fibre_along, double fibre_transverse,
                       double z_global, int layer, int type, int hexant, bool hcal){

    // Find index of nearest sensor centre in global coordinates
    int centre_idx = static_cast<int>(std::floor(fibre_along / size_sharp_SiPM + 0.5));

    const int search_range = 2;
    double total_fraction = 0.0;
    std::vector<std::pair<double, double>> contributions; // (global sensor centre, fraction)

    for (int i = centre_idx - search_range; i <= centre_idx + search_range; ++i)
    {
        double sensor_centre = i * size_sharp_SiPM; // global position
        double frac = FibreSensorOverlapFraction(fibre_along, sensor_centre);
        if (frac > 0.0)
        {
            contributions.emplace_back(sensor_centre, frac);
            total_fraction += frac;
        }
    }

    // Snap transverse coordinate (along-fibre axis) to the hexant centre.
    // The fibre runs the full length of a hexant, so like the bar coarse axis,
    // only the hexant identity matters — use the coarse grid (pitch = size_bar_L).
    // type 5: fibre runs along X → coarse grid in X; type 6: runs along Y → coarse in Y.
    double global_transverse = (type == 5)
        ? SnapToGrid(fibre_transverse, grid_wide_coarse_y)
        : SnapToGrid(fibre_transverse, grid_wide_coarse_x);

    for (auto& [sensor_global, frac] : contributions){
      // sensor number is the global index relative to the hexant origin
      int sensor_number = static_cast<int>(std::round(sensor_global / size_sharp_SiPM));

      double saved_atten = n_atten_photons;
      n_atten_photons = n_escaping_photons * (frac / total_fraction);
      ConvertFibrePE();
      n_PE_wnoise = AddSiPMNoise(n_PE);
      n_PE_wnoise = AddGeneralNoise(n_PE_wnoise);
      bool LG = 0;
      v_digi_sharp_ADC_SiPM.push_back(ConvertADCCountSHARP(n_PE_wnoise, LG));
      v_digi_sharp_gain_switch.push_back(LG);
      v_digi_sharp_sensorID.push_back(sensor_number);
      v_digi_sharp_hexant.push_back(hexant);
      v_digi_sharp_orientation.push_back(type);
      v_digi_sharplayer.push_back(layer);
      v_digi_sharp_z.push_back(z_global);
      v_digi_sharp_hcal.push_back(hcal);
      if (type == 5) {
          v_digi_sharp_x.push_back(sensor_global);
          v_digi_sharp_y.push_back(global_transverse);
      } else {
          v_digi_sharp_x.push_back(global_transverse);
          v_digi_sharp_y.push_back(sensor_global);
      }
      n_atten_photons = saved_atten;
    }

}

void CaloSysDigitiser::ConvertFibrePE(){
  n_PE = n_escaping_photons * cf_fibre_PDE;
}



double CaloSysDigitiser::GetADCCountWide(double edep, double xy_local, bool &LG){
    
  ConvertWideBarPromptPhotonCount(edep);
  ConvertWideBarCapturedPhotonCount();
  ConvertWideBarAttenuatedPhotonCount(xy_local);
  ConvertWideBarEscapingPhotonCount();
  ConvertWideBarPE();
  n_PE_wnoise = AddSiPMNoise(n_PE);
  n_PE_wnoise = AddGeneralNoise(n_PE_wnoise);
  return ConvertADCCountWide(n_PE_wnoise, LG);
}

double CaloSysDigitiser::GetADCCountThin(double edep, double xy_local, bool &LG){
    
  ConvertThinBarPromptPhotonCount(edep);
  ConvertThinBarCapturedPhotonCount();
  ConvertThinBarAttenuatedPhotonCount(xy_local);
  ConvertThinBarEscapingPhotonCount();
  ConvertThinBarPE();
  n_PE_wnoise = AddSiPMNoise(n_PE);
  n_PE_wnoise = AddGeneralNoise(n_PE_wnoise);
  return ConvertADCCountThin(n_PE_wnoise,LG);
}

void CaloSysDigitiser::GetADCChannels(int type, double edep, double x_local, double y_local, bool hcal, int hexant){
  
  double x_local_shifted = x_local + 0.5 * size_bar_L;
  double y_local_shifted = y_local + 0.5 * size_bar_L;


  bool LG = 0;
  switch(type){
    case 1: v_digi_widebar_ADC_SiPM_L.push_back(GetADCCountWide(edep, x_local_shifted, LG));
            v_digi_widebar_gain_switch_L.push_back(LG);
            LG = 0;
            v_digi_widebar_ADC_SiPM_R.push_back(GetADCCountWide(edep, size_bar_L - x_local_shifted,LG));
            v_digi_widebar_gain_switch_R.push_back(LG);
            v_digi_wide_orientation.push_back(1);
            v_digi_wide_hcal.push_back(hcal);
            v_digi_wide_hexant.push_back(hexant);
            break;
    case 2: v_digi_widebar_ADC_SiPM_L.push_back(GetADCCountWide(edep, y_local_shifted, LG));
            v_digi_widebar_gain_switch_L.push_back(LG);
            LG = 0;
            v_digi_widebar_ADC_SiPM_R.push_back(GetADCCountWide(edep, size_bar_L - y_local_shifted, LG));
            v_digi_widebar_gain_switch_R.push_back(LG);
            v_digi_wide_orientation.push_back(2);
            v_digi_wide_hcal.push_back(hcal);
            v_digi_wide_hexant.push_back(hexant);
            break;
    case 3: v_digi_thinbar_ADC_SiPM_L.push_back(GetADCCountThin(edep, x_local_shifted, LG));
            v_digi_thinbar_gain_switch_L.push_back(LG);
            LG = 0;
            v_digi_thinbar_ADC_SiPM_R.push_back(GetADCCountThin(edep, size_bar_L - x_local_shifted, LG));
            v_digi_thinbar_gain_switch_R.push_back(LG);
            v_digi_thin_orientation.push_back(3);
            v_digi_thin_hcal.push_back(hcal);
            v_digi_thin_hexant.push_back(hexant);
            break;
    case 4: v_digi_thinbar_ADC_SiPM_L.push_back(GetADCCountThin(edep, y_local_shifted, LG));
            v_digi_thinbar_gain_switch_L.push_back(LG);
            LG = 0;
            v_digi_thinbar_ADC_SiPM_R.push_back(GetADCCountThin(edep, size_bar_L - y_local_shifted, LG));
            v_digi_thinbar_gain_switch_R.push_back(LG);
            v_digi_thin_orientation.push_back(4);
            v_digi_thin_hcal.push_back(hcal);
            v_digi_thin_hexant.push_back(hexant);
            break;
    case 5: break; //Done in another function
    case 6: break; //
    default: std::cout << "Unknown code detected" << std::endl;
  }

}


void CaloSysDigitiser::GetLayer(int layer, int type, bool hcal, double z_global){
  
  switch (type) {
    case 1: v_digi_widelayer.push_back(layer + static_cast<int>(hcal)*5); v_digi_wide_z.push_back(z_global); break;
    case 2: v_digi_widelayer.push_back(layer + static_cast<int>(hcal)*5); v_digi_wide_z.push_back(z_global); break;
    case 3: v_digi_thinlayer.push_back(layer + static_cast<int>(hcal)*5); v_digi_thin_z.push_back(z_global); break;
    case 4: v_digi_thinlayer.push_back(layer + static_cast<int>(hcal)*5); v_digi_thin_z.push_back(z_global); break;
    case 5: break; //Done in another function 
    case 6: break; //Done in another function
  }
}

void CaloSysDigitiser::GetBar_or_Fibre(ULong64_t entry){

  // Unpack hexant FIRST so hexantX/hexantY are valid for this entry.
  HexantUnpacker(entry);

  const double x_hit = v_x_global->at(entry);
  const double y_hit = v_y_global->at(entry);

  switch (v_type->at(entry)) {
    case 1: // Wide horizontal: bar runs along X (coarse), fine segmentation in Y
        v_digi_widebar_H.push_back(v_vol->at(entry));
        v_digi_wide_x.push_back(SnapToGrid(x_hit, grid_wide_coarse_x));  // 2160 mm pitch
        v_digi_wide_y.push_back(SnapToGrid(y_hit, grid_wide_fine_y));    //   60 mm pitch
        break;

    case 2: // Wide vertical: bar runs along Y (coarse), fine segmentation in X
        v_digi_widebar_V.push_back(v_vol->at(entry));
        v_digi_wide_x.push_back(SnapToGrid(x_hit, grid_wide_fine_x));    //   60 mm pitch
        v_digi_wide_y.push_back(SnapToGrid(y_hit, grid_wide_coarse_y));  // 2160 mm pitch
        break;

    case 3: // Thin horizontal: bar runs along X (coarse), fine segmentation in Y
        v_digi_thinbar_H.push_back(v_vol->at(entry));
        v_digi_thin_x.push_back(SnapToGrid(x_hit, grid_thin_coarse_x)); // 2160 mm pitch
        v_digi_thin_y.push_back(SnapToGrid(y_hit, grid_thin_fine_y));   //   10 mm pitch
        break;

    case 4: // Thin vertical: bar runs along Y (coarse), fine segmentation in X
        v_digi_thinbar_V.push_back(v_vol->at(entry));
        v_digi_thin_x.push_back(SnapToGrid(x_hit, grid_thin_fine_x));   //   10 mm pitch
        v_digi_thin_y.push_back(SnapToGrid(y_hit, grid_thin_coarse_y)); // 2160 mm pitch
        break;
    case 5: {
        int sublayer = v_hpl_subsection->at(entry);
        if (sublayer < 0) break;
        double fibre_along      = v_x_global->at(entry);
        double fibre_transverse = GetFibreCentre(v_y_global->at(entry), sublayer);
        double fibre_local      = v_x_local->at(entry) + 0.5 * size_bar_L; // shift to [0, bar_L]
        ConvertFibrePromptPhotonCount(v_edep->at(entry));
        ConvertFibreAttenuatedPhotonCount(fibre_local);
        ConvertFibreEscapingPhotonCount();
        MapFibreToSensors(fibre_along, fibre_transverse,
                  v_z_global->at(entry), v_layer->at(entry) + static_cast<int>(v_hcal->at(entry)) * 5,
                  5, hexantX*10 + hexantY, v_hcal->at(entry));
        break;
    }
    case 6: {
        int sublayer = v_hpl_subsection->at(entry);
        if (sublayer < 0) break;
        double fibre_along      = v_y_global->at(entry);
        double fibre_transverse = GetFibreCentre(v_x_global->at(entry), sublayer);
        double fibre_local      = v_y_local->at(entry) + 0.5 * size_bar_L; // shift to [0, bar_L]
        ConvertFibrePromptPhotonCount(v_edep->at(entry));
        ConvertFibreAttenuatedPhotonCount(fibre_local);
        ConvertFibreEscapingPhotonCount();
        MapFibreToSensors(fibre_along, fibre_transverse,
                  v_z_global->at(entry), v_layer->at(entry) + static_cast<int>(v_hcal->at(entry)) * 5,
                  6, hexantX*10 + hexantY, v_hcal->at(entry));
        break;
    }
  }

}

void CaloSysDigitiser::DigitiseEntry(ULong64_t entry){

  //Enter the required data from the loaded TTree branch vectors using only the entry

  GetADCChannels(v_type->at(entry),v_edep->at(entry),v_x_local->at(entry),v_y_local->at(entry),v_hcal->at(entry),v_hexant->at(entry));
  GetLayer(v_layer->at(entry),v_type->at(entry),v_hcal->at(entry),v_z_global->at(entry));
  GetBar_or_Fibre(entry);
}



void CaloSysDigitiser::Loop(){

  ULong64_t nev = ft_data->GetEntries();
  for(ULong64_t ev=0;ev<nev;ev++){
    ft_data->GetEntry(ev);
    ULong64_t n_vectorentry = v_edep->size();
    for(ULong64_t vectorentry=0;vectorentry<n_vectorentry;vectorentry++){
      DigitiseEntry(vectorentry);
    }
    ft_out->Fill();

    v_digi_widebar_ADC_SiPM_L.clear(); 
    v_digi_widebar_ADC_SiPM_R.clear(); 
    v_digi_thinbar_ADC_SiPM_L.clear();
    v_digi_thinbar_ADC_SiPM_R.clear();
    v_digi_sharp_ADC_SiPM.clear();                                                                
    v_digi_wide_orientation.clear();
    v_digi_thin_orientation.clear();
    v_digi_sharp_orientation.clear();
    v_digi_widebar_gain_switch_L.clear();
    v_digi_widebar_gain_switch_R.clear();
    v_digi_thinbar_gain_switch_L.clear();
    v_digi_thinbar_gain_switch_R.clear();
    v_digi_sharp_gain_switch.clear();
    v_digi_widebar_H.clear();
    v_digi_widebar_V.clear();
    v_digi_widelayer.clear();
    v_digi_thinbar_H.clear();
    v_digi_thinbar_V.clear();
    v_digi_thinlayer.clear();
    v_digi_wide_hexant.clear();
    v_digi_thin_hexant.clear();
    v_digi_sharp_hexant.clear();
    v_digi_wide_x.clear();
    v_digi_wide_y.clear();
    v_digi_wide_z.clear();
    v_digi_thin_x.clear();
    v_digi_thin_y.clear();
    v_digi_thin_z.clear();
    v_digi_sharp_x.clear();
    v_digi_sharp_y.clear();
    v_digi_sharp_z.clear();
    v_digi_sharplayer.clear();
    v_digi_sharp_sensorID.clear();
    v_digi_wide_hcal.clear();
    v_digi_thin_hcal.clear();
    v_digi_sharp_hcal.clear();
  }
  ff_out->WriteTObject(ft_out);
  ff_out->Close();
}
