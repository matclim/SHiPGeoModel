void testADCs_digi(){

  ROOT::RDataFrame df("digi_calo_events","./build/digi_calosim_out_10000MeV_e-.root");

  auto df1 = df.Define("ADCsum",[](
        const std::vector<double> &v_digi_widebar_ADC_SiPM_L,const std::vector<double> &v_digi_widebar_ADC_SiPM_R,
        const std::vector<bool> &v_digi_widebar_gain_switch_L,const std::vector<bool> &v_digi_widebar_gain_switch_R,
        const std::vector<double> &v_digi_thinbar_ADC_SiPM_L,const std::vector<double> &v_digi_thinbar_ADC_SiPM_R,
        const std::vector<bool> &v_digi_thinbar_gain_switch_L,const std::vector<bool> &v_digi_thinbar_gain_switch_R,
        const std::vector<double> &v_digi_sharp_ADC_SiPM,const std::vector<bool> &v_digi_sharp_gain_switch){
      
      double hg_pedestal = 50;
      double lg_pedestal = 3;

      double ADCsum = 0;
      for(size_t ent = 0; ent < v_digi_widebar_ADC_SiPM_L.size();ent++){
        if(v_digi_widebar_gain_switch_L.at(ent)==0) ADCsum+= v_digi_widebar_ADC_SiPM_L.at(ent) -hg_pedestal;
        else ADCsum+= (v_digi_widebar_ADC_SiPM_L.at(ent)-lg_pedestal)*200./7.;
      }
      for(size_t ent = 0; ent < v_digi_widebar_ADC_SiPM_R.size();ent++){
        if(v_digi_widebar_gain_switch_R.at(ent)==0) ADCsum+= v_digi_widebar_ADC_SiPM_R.at(ent) -hg_pedestal;
        else ADCsum+= (v_digi_widebar_ADC_SiPM_R.at(ent)-lg_pedestal)*200./7.;
      }
      for(size_t ent = 0; ent < v_digi_thinbar_ADC_SiPM_L.size();ent++){
        if(v_digi_thinbar_gain_switch_L.at(ent)==0) ADCsum+= v_digi_thinbar_ADC_SiPM_L.at(ent) -hg_pedestal;
        else ADCsum+= (v_digi_thinbar_ADC_SiPM_L.at(ent)-lg_pedestal)*200./7.;
      }
      for(size_t ent = 0; ent < v_digi_thinbar_ADC_SiPM_R.size();ent++){
        if(v_digi_thinbar_gain_switch_R.at(ent)==0) ADCsum+= v_digi_thinbar_ADC_SiPM_R.at(ent) -hg_pedestal;
        else ADCsum+= (v_digi_thinbar_ADC_SiPM_R.at(ent)-lg_pedestal)*200./7.;
      }
      for(size_t ent = 0; ent < v_digi_sharp_ADC_SiPM.size();ent++){
        if(v_digi_sharp_gain_switch.at(ent)==0) ADCsum+= v_digi_sharp_ADC_SiPM.at(ent) -hg_pedestal;
        else ADCsum+= (v_digi_sharp_ADC_SiPM.at(ent)-lg_pedestal)*200./7.;
      }
      return ADCsum;
  },
        {"v_digi_widebar_ADC_SiPM_L","v_digi_widebar_ADC_SiPM_R","v_digi_widebar_gain_switch_L","v_digi_widebar_gain_switch_R","v_digi_thinbar_ADC_SiPM_L","v_digi_thinbar_ADC_SiPM_R","v_digi_thinbar_gain_switch_L","v_digi_thinbar_gain_switch_R","v_digi_sharp_ADC_SiPM","v_digi_sharp_gain_switch"}
      );

   auto h1 = df1.Histo1D({"ADC_sum","ADC_sum;ADC counts;#",100,0,1000000},"ADCsum");

   std::unique_ptr c1 = std::make_unique<TCanvas>("c1","c1",800,600);


    h1->Draw();
    c1->SaveAs("ADCtest.pdf");
}

