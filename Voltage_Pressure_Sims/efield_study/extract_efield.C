#include <iostream>
#include <fstream>
#include <cmath>

#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ComponentComsol.hh"

using namespace Garfield;

/*
Extract the electric field and electric potential as a function of z for the COMSOL model. This helps diagnose
whether the electric field / potential is as expected.
*/

int main() {
  // Load COMSOL model
  ComponentComsol pumaModel;
  pumaModel.Initialise(
      "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/mesh.mphtxt",
      "/home/macosta/ella_work/PUMA_Tests/Simulations/dielectric_py.txt",
      "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/potential_1900_new.txt", "mm");

  // Extract electric field along z-axis
  const double zMin = 0.0; // cm
  const double zMax = 6.0; // cm
  const int nPoints = 500;

  std::ofstream efieldFile("Efield_vs_z_outside.txt");
  efieldFile << "# z [cm]\tEz [V/cm]\n";
  
  // Offset from center of PUMA
  double x = 0.6; // [cm]
  double y = 0.0; // [cm]

  for (int i = 0; i <= nPoints; ++i) {
    double z = zMin + i * (zMax - zMin) / nPoints;
    std::array<double, 3> efield = pumaModel.ElectricField(x, y, z);
    double ez = efield[2];
    efieldFile << z << "\t" << ez << "\n";
  }

  efieldFile.close();
  std::cout << "Electric field profile saved to Efield_vs_z.txt\n";

  std::ofstream potentialFile("Potential_vs_z_outside.txt");
  potentialFile << "# z [cm]\tV [V]\n";

for (int i = 0; i <= nPoints; ++i) {
  double z = zMin + i * (zMax - zMin) / nPoints;
  double potential = pumaModel.ElectricPotential(x, y, z);
  potentialFile << z << "\t" << potential << "\n";
}

potentialFile.close();

  return 0;
}