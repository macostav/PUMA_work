#include <cstdlib>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>

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

std::pair<double, double> randInCircle()
{
  /*
  Generates random coordinates for inside of a circle.

  :returns: a pair of coordinates
  */
  double radiusCathode = 0.5; //[cm]
  double radiusElectrons = radiusCathode / 3.0;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist_angle(0, 2 * M_PI);
  std::uniform_real_distribution<double> dist_radius(0, 1);

  double theta = dist_angle(gen);
  double r = radiusElectrons * std::sqrt(dist_radius(gen));

  double x = r * std::cos(theta);
  double y = r * std::sin(theta);

  return {x, y};
}

void run_simulation(double pres, int volt, ComponentComsol *pumaModel)
{
  const double pressure = pres; // [Torr]

  // Setup gas
  MediumMagboltz gas;
  gas.SetTemperature(293.15);
  gas.SetPressure(pressure);
  gas.SetComposition("Ar", 100.);
  gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Ar+_Ar.txt");

  if (!gas.LoadGasFile("/home/macosta/ella_work/PUMA_Tests/Simulations/argon_table.gas"))
  {
    gas.GenerateGasTable(5, false);
    gas.WriteGasFile("argon_table.gas");
  }
  gas.Initialise(false);
  std::cout << "Gas Initialized \n";

  // Attach gas to pre-loaded model
  pumaModel->SetGas(&gas);

  // Sensor setup
  Sensor sensor;
  sensor.AddComponent(pumaModel);
  sensor.SetArea(-3, -3, -15, 3, 3, 5); // [cm]
  std::cout << "Sensor Initialized \n";

  // TrackHeed setup
  TrackHeed trackHeed;
  trackHeed.SetParticle("e-");
  trackHeed.SetSensor(&sensor);
  std::cout << "TrackHeed Initialized \n";

  DriftLineRKF drift;
  drift.SetSensor(&sensor);

  // Histogram for e- speed
  TH1F *hSpeed = new TH1F("hSpeed", "Electron Drift Speeds;Speed [cm/microsecond];Counts", 100, 0, 1);

  // Drift loop
  int nRuns = 200;
  for (int i = 0; i < nRuns; i++)
  {
    auto [x0, y0] = randInCircle();
    double z0 = 4.32;

    double energy = 1e6;
    trackHeed.NewTrack(x0, y0, z0, 0, 0, 1, energy);

    double xc, yc, zc, tc;
    int nc, nsec;
    double ec, esec;

    while (trackHeed.GetCluster(xc, yc, zc, tc, nc, nsec, ec, esec))
    {
      for (int i = 0; i < nc; ++i)
      {
        drift.DriftElectron(xc, yc, zc, tc);

        double x1, y1, z1, t1;
        int status2;
        drift.GetEndPoint(x1, y1, z1, t1, status2);

        double dx = x1 - xc;
        double dy = y1 - yc;
        double dz = z1 - zc;
        double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        double time = t1 - tc;

        if (time > 0)
        {
          double speed = distance / time;
          hSpeed->Fill(speed * 1e3); // cm/μs
        }
      }
    }
  }

  double mean_drift_speed = hSpeed->GetMean();
  double sigma_drift_speed = hSpeed->GetStdDev();
  std::cout << "Mean drift speed: " << mean_drift_speed << " cm/μs\n";
  std::cout << "Standard deviation: " << sigma_drift_speed << " cm/μs\n";

  // Save results
  std::ostringstream rootFileName, statsFileName;
  rootFileName << "drift_speeds_" << volt << "V_" << (int)pressure << "Torr.root";
  statsFileName << "drift_speed_stats_" << volt << "V_" << (int)pressure << "Torr.txt";

  TFile outFile(rootFileName.str().c_str(), "RECREATE");
  hSpeed->Write();
  outFile.Close();

  std::ofstream statsFile(statsFileName.str());
  statsFile << "Voltage [V]: " << volt << "\n";
  statsFile << "Pressure [Torr]: " << pressure << "\n";
  statsFile << "Mean drift speed [cm/us]: " << mean_drift_speed << "\n";
  statsFile << "Standard deviation [cm/us]: " << sigma_drift_speed << "\n";
  statsFile.close();

  delete hSpeed;
}

int main()
{
  std::vector<int> voltages = {200};
  std::vector<double> pressures = {158.0272814, 305.83624583, 497.8134186, 703.14866857, 897.54054586, 1000.95292865, 1003.96149327,
                                   1005.96709465, 1106.13661436, 1304.82907969, 1498.69503398};

    for (int voltage: voltages) {
    // Load model just once (depends only on voltage)
    std::ostringstream potFile;
    potFile << "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/potential_" << voltage << ".txt";

    ComponentComsol pumaModel;
    pumaModel.Initialise(
      "/home/macosta/PUMA/miguel_work/Voltage_Pressure_Sims/Comsol_Files/mesh.mphtxt",
      "/home/macosta/ella_work/PUMA_Tests/Simulations/dielectric_py.txt",
      potFile.str(), "mm");

      std::cout << "Model Initialized \n";
      for (double pressure : pressures) {
        run_simulation(pressure, voltage, &pumaModel);
      }
    }
      

  return 0;
}