#include <iostream>
#include <fstream>
#include <cmath>

#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ComponentComsol.hh"

using namespace Garfield;

int main() {
  const double pressure = 305.83624583; // Torr

  // Load COMSOL model
  ComponentComsol pumaModel;
  pumaModel.Initialise(
      "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/mesh.mphtxt",
      "/home/macosta/ella_work/PUMA_Tests/Simulations/dielectric_py.txt",
      "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/potential_1600.txt", "mm");

  // Setup gas
  MediumMagboltz gas;
  gas.SetTemperature(293.15);
  gas.SetPressure(pressure);
  gas.SetComposition("Ar", 100.);
  gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Ar+_Ar.txt");

  std::string gasFileName = "gas_tables/argon_" + std::to_string(int(pressure)) + "Torr.gas";
    
    if (!gas.LoadGasFile(gasFileName)) {
        std::cout << "Generating new gas table for " << pressure << " Torr...\n";
        gas.GenerateGasTable(5, false);
        gas.WriteGasFile(gasFileName);
    } else {
        std::cout << "Loaded existing gas table for " << pressure << " Torr\n";
    }
  gas.Initialise(false);
  std::cout << "Gas Initialized \n";

  // Attach gas to pre-loaded model
  pumaModel.SetGas(&gas);

  // Extract electric field along z-axis
  const double zMin = 0.0; // cm
  const double zMax = 6.0;   // cm
  const int nPoints = 200;

  std::ofstream efieldFile("Efield_vs_z.txt");
  efieldFile << "# z [cm]\tEz [V/cm]\n";

  for (int i = 0; i <= nPoints; ++i) {
    double z = zMin + i * (zMax - zMin) / nPoints;
    std::array<double, 3> efield = pumaModel.ElectricField(0.0, 0.0, z);
    double ez = efield[2];
    efieldFile << z << "\t" << ez << "\n";
  }

  efieldFile.close();
  std::cout << "Electric field profile saved to Efield_vs_z.txt\n";

  return 0;
}