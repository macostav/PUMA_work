#include <cstdlib>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <filesystem>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TFile.h>
#include "Garfield/ComponentComsol.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewCell.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ViewDrift.hh"
#include "Garfield/DriftLineRKF.hh"
#include "Garfield/Random.hh"
#include "Garfield/ComponentGrid.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/ViewMedium.hh"

using namespace Garfield;

int main() {

  std::vector<double> pressures = {158.0272814,305.83624583,497.8134186,703.14866857,897.54054586,1000.95292865, 1003.96149327, 
    1005.96709465, 1106.13661436, 1304.82907969, 1498.69503398};

  
  for (double pressure: pressures){
    // Setup gas
    
  MediumMagboltz gas;
  gas.SetMaxElectronEnergy(50.);     // retry here
  gas.EnableAutoEnergyLimit(false); // use the max electron energy specified above
  gas.SetTemperature(293.15);
  gas.SetPressure(pressure);
  gas.SetComposition("Ar", 100.);
  gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Ar+_Ar.txt");
  
  

  std::string gasFileName = "new_argon_" + std::to_string(int(pressure)) + "Torr.gas";
  std::cout << "Generating new gas table for " << pressure << " Torr...\n";
  gas.GenerateGasTable(10, true);
  gas.WriteGasFile(gasFileName);
  }
}