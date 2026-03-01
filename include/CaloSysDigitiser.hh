#pragma once
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"

#include <RtypesCore.h>
#include <cstdint>
#include <memory>
#include <iostream>
#include <cmath>

class CaloSysDigitiser{

  public:
    CaloSysDigitiser();
    CaloSysDigitiser(TFile* f_data){ff_data = f_data; ff_data->GetObject(static_cast<std::string>("calo_events").c_str(),ft_data);};
    ~CaloSysDigitiser();
   
    void Initialise();
    void SetFile(std::string file_location_str);
    void Loop();
  

  private:

    TRandom3 rng;

    void DigitiseEntry(ULong64_t entry);
    void GetLayer(int layer, int type, bool hcal, double z_global);
    int GeneratePEDCnoise();
    void ConvertWideBarPromptPhotonCount(double edep);
    void ConvertWideBarCapturedPhotonCount();
    void ConvertWideBarAttenuatedPhotonCount(double xylocal);
    void ConvertWideBarEscapingPhotonCount();
    void ConvertWideBarPE();
    double ConvertADCCountWide(double n_PE_wnoise, bool &LG);
    double SmearWideBarsWidth(double xy) const;
    double SmearWideBarsLength(double xy) const;
    double SmearThinBarsWidth(double xy) const;
    double SmearThinBarsLength(double xy) const;
    void ConvertThinBarPromptPhotonCount(double edep);
    void ConvertThinBarCapturedPhotonCount();
    void ConvertThinBarAttenuatedPhotonCount(double xylocal);
    void ConvertThinBarEscapingPhotonCount();
    void ConvertThinBarPE();
    double ConvertADCCountThin(double n_PE_wnoise, bool &LG);
    void HexantUnpacker(ULong64_t entry);
    double AddGeneralNoise(double PEcount);
    double ConvertADCCountSHARP(double n_PE_wnoise, bool &LG);
    void ConvertFibrePromptPhotonCount(double edep);
    void ConvertFibreAttenuatedPhotonCount(double xylocal);
    void ConvertFibreEscapingPhotonCount();
    void ConvertFibrePE();
    double CircleAreaLeftOfLine(double d, double r) const;
    double FibreSensorOverlapFraction(double fibre_x, double sensor_x) const;
    double GetFibreCentre(double global_coord, int sublayer) const;
    void MapFibreToSensors(double fibre_along, double fibre_transverse, double z_global, int layer, int type, int hexant);

    double AddSiPMNoise(double PEcount);
    double GetADCCountWide(double edep, double xy_local, bool &LG);
    double GetADCCountThin(double edep, double xy_local, bool &LG);
    double GetADCCountSHARP(double edep, double xy_local, bool &LG);
    void GetADCChannels(int type, double edep, double x_local, double y_local);


    void GetBar_or_Fibre(ULong64_t entry);


    //TTree input data
    TFile* ff_data;
    TTree* ft_data;
    
    std::vector<double>* v_edep = nullptr;
    std::vector<double>* v_x_global = nullptr;
    std::vector<double>* v_y_global = nullptr;
    std::vector<double>* v_z_global = nullptr;
    std::vector<double>* v_x_local = nullptr;
    std::vector<double>* v_y_local = nullptr;
    std::vector<double>* v_z_local = nullptr;
    std::vector<int>*    v_type = nullptr;
    std::vector<int>*    v_section = nullptr;
    std::vector<int>*    v_layer = nullptr;
    std::vector<int>*    v_vol = nullptr;
    std::vector<int>*    v_hcal = nullptr;
    std::vector<int>*    v_hpl_subsection = nullptr;
    std::vector<int>*    v_hexant = nullptr;

    //Process data

    int hexantX, hexantY;

    //TTree output data
    TTree *ft_out;
    TFile *ff_out;

    std::vector<double> v_digi_widebar_ADC_SiPM_L; //left-right/bottom-top
    std::vector<double> v_digi_widebar_ADC_SiPM_R; //left-right/bottom-top
    std::vector<double> v_digi_thinbar_ADC_SiPM_L; //left-right/bottom-top
    std::vector<double> v_digi_thinbar_ADC_SiPM_R; //left-right/bottom-top
    std::vector<double> v_digi_sharp_ADC_SiPM;
    std::vector<bool> v_digi_widebar_gain_switch_L;
    std::vector<bool> v_digi_widebar_gain_switch_R;
    std::vector<bool> v_digi_thinbar_gain_switch_L;
    std::vector<bool> v_digi_thinbar_gain_switch_R;
    std::vector<int> v_digi_wide_orientation;  // 1 = horizontal, 2 = vertical
    std::vector<int> v_digi_thin_orientation;  // 3 = horizontal, 4 = vertical
    std::vector<int> v_digi_sharp_orientation; // 5 = horizontal, 6 = vertical
    std::vector<int> v_digi_sharp_sensorID;
    std::vector<int> v_digi_wide_hexant;
    std::vector<int> v_digi_thin_hexant;
    std::vector<int> v_digi_sharp_hexant;
    std::vector<bool> v_digi_sharp_gain_switch;
    std::vector<int> v_digi_widebar_H;
    std::vector<int> v_digi_widebar_V;
    std::vector<int> v_digi_widelayer;
    std::vector<int> v_digi_thinbar_H;
    std::vector<int> v_digi_thinbar_V;
    std::vector<int> v_digi_thinlayer;
    std::vector<int> v_digi_sharplayer;
    std::vector<double> v_digi_wide_x;
    std::vector<double> v_digi_wide_y;
    std::vector<double> v_digi_wide_z;
    std::vector<double> v_digi_thin_x;
    std::vector<double> v_digi_thin_y;
    std::vector<double> v_digi_thin_z;
    std::vector<double> v_digi_sharp_x;
    std::vector<double> v_digi_sharp_y;
    std::vector<double> v_digi_sharp_z;
    //Detector data
    double size_bar_L = 2160;
    double size_bar_W = 60;
    int nhexants_X = 2;
    int nhexants_Y = 3;
    int nbars_wide_module = 36;
    int nbars_thin_module = 216;
    double size_sharp_SiPM = 2.4;
    double r_sharp_fibre = 0.6;
    //Digitisation data

    double atten_length_bar_fibre = 3500;
    double cf_wb_edep_to_prompt_photons = 12340.; //Amount cut in 2 already as there are 2 sides
    double cf_wb_prompt_photons_to_captured_photons = 0.05;
    double cf_wb_atten_photons_to_escaping_photons = 0.6;
    double cf_wb_PDE = 0.5;
    double cf_wb_n_PE_ADC = 1.0;

    double cf_tb_edep_to_prompt_photons = 7720.;
    double cf_tb_prompt_photons_to_captured_photons = 0.1;
    double cf_tb_atten_photons_to_escaping_photons = 0.6;
    double cf_tb_PDE = 0.4;
    double cf_tb_n_PE_ADC = 1.0 ;

    double cf_fibre_edep_to_prompt_photons = 10000; //Amount cut in 2 already as there are 2 sides
    double cf_fibre_prompt_photons_to_captured_photons = 0.05;
    double cf_fibre_atten_photons_to_escaping_photons = 0.6;
    double cf_fibre_PDE = 0.5;
    double cf_fibre_n_PE_ADC = 1.0;

    double n_prompt_photons;
    double n_captured_photons;
    double n_atten_photons;
    double n_escaping_photons;
    double n_PE;
    double n_PE_wnoise;
    double HG_ADC;
    double hg_pedestal = 50; 
    double lg_pedestal = 3.;
    double LG_factor = 200./7.;

};










