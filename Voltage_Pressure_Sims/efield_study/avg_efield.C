#include <iostream>
#include <fstream>
#include <cmath>

#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ComponentComsol.hh"

using namespace Garfield;

/*
Extract the average electric field in the region close to the upper grid. The average is weighed by the area.
*/

int main()
{
    // Load COMSOL model
    ComponentComsol pumaModel;
    pumaModel.Initialise(
        "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/mesh.mphtxt",
        "/home/macosta/ella_work/PUMA_Tests/Simulations/dielectric_py.txt",
        "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/potential_1900.txt", "mm");

    // Extract electric field along z-axis
    const double zMin = 0.0; // cm
    const double zMax = 6.0; // cm
    const int nPoints = 500;

    // Offset from center of PUMA; x will essentially act as a radial distance from the center
    std::vector<double> x_samples = {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5}; // [cm]
    double y = 0.0;

    std::ofstream efieldFile("Efield_vs_z_avg_HV1900.txt");
    efieldFile << "# z [cm]\tEz [V/cm]\n"; // [cm]

    // Will compute average E-field weighing by annular area
    for (int i = 0; i <= nPoints; ++i)
    {
        double z = zMin + i * (zMax - zMin) / nPoints;

        double weighted_sum = 0.0;
        double weight_total = 0.0;

        // Loop over annuli
        for (size_t j = 0; j < x_samples.size() - 1; ++j)
        {
            double r1 = x_samples[j];
            double r2 = x_samples[j + 1];

            // Midpoint radius for sampling
            double r_mid = 0.5 * (r1 + r2);

            // Area of annulus
            double area = M_PI * (r2 * r2 - r1 * r1);

            // Sample E-field at (r_mid, 0, z)
            std::array<double, 3> efield = pumaModel.ElectricField(r_mid, y, z);
            double ez_sample = efield[2];

            weighted_sum += ez_sample * area;
            weight_total += area;
        }

        double avg_ez = weighted_sum / weight_total;
        efieldFile << z << "\t" << avg_ez << "\n";
    }

    efieldFile.close();
    std::cout << "Electric field profile saved to Efield_vs_z.txt\n";

    return 0;
}