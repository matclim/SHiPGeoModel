#include "CaloSysDigitiser.hh"
#include <string>


int main(int argc, const char **argv){

  std::string file_name;

  if(argc < 2) file_name= "./calosim_out_10000MeV.root";
  else file_name = argv[1];

  TFile *f_in = TFile::Open(file_name.c_str(),"READ");
  CaloSysDigitiser digitiser(f_in);
  digitiser.Initialise();
  digitiser.Loop();

}
