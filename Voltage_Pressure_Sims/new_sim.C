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
#include <Garfield/AvalancheMicroscopic.hh>

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

void run_simulation(double pres, int volt, ComponentComsol *pumaModel, const std::string& csvFileName)
{
  const double pressure = pres; // [Torr]

  // Setup gas
  MediumMagboltz gas;
  gas.SetTemperature(293.15);
  gas.SetPressure(pressure);
  gas.SetComposition("Xe", 100.); // !!! Can change this
  gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Xe+_P32_Xe.txt");
  //gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Ar+_Ar.txt");

  std::string gasFileName = "gas_tables/xenon_" + std::to_string(int(pressure)) + "Torr.gas"; // !!!
    
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
pumaModel->SetGas(&gas);

// Sensor setup
Sensor sensor;
sensor.AddComponent(pumaModel);
sensor.SetArea(-3, -3, -15, 3, 3, 5); // [cm]
std::cout << "Sensor Initialized \n";

// DriftLineRKF still available if you want to visualize field lines
DriftLineRKF drift;
drift.SetSensor(&sensor);

// Histogram for drift speeds
TH1D* hSpeed = new TH1D("hSpeed", "Drift Speed;Speed [cm/#mu s];Counts", 100, 0, 10);

// Run the simulation
int nElectronsTarget = 100; // !!! try 10,000
int nElectronsSimulated = 0;

std::cout << "Before loop \n";

while (nElectronsSimulated < nElectronsTarget) {
  // Generate random photoelectron position in a circle on the top surface

  std::cout << "In loop \n";
  auto [x0, y0] = randInCircle();
  double z0 = 4.32; // starting near the cathode (in cm)
  double t0 = 0.0;

  std::cout << "Before drifting \n";

  drift.DriftElectron(x0, y0, z0, t0);

  std::cout << "Electron drifted \n";

  double x1, y1, z1, t1;
  int status;
  drift.GetEndPoint(x1, y1, z1, t1, status);
  

  if (status == 0 && t1 > t0) {  // status 0 = success
    
    double driftLength = sqrt((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) + (z1 - z0)*(z1 - z0));
    double dt = t1 - t0;         // ns

    double vDrift = driftLength / dt * 1e3; // cm/μs
    hSpeed->Fill(vDrift);
    nElectronsSimulated++;
  }
}


// Avalanche mc or avalanche microscopic

  double mean_drift_speed = hSpeed->GetMean();
  double sigma_drift_speed = hSpeed->GetStdDev();
  std::cout << "Mean drift speed: " << mean_drift_speed << " cm/μs\n";
  std::cout << "Standard deviation: " << sigma_drift_speed << " cm/μs\n";

  // Append results to csvFile
  std::ofstream csvFile(csvFileName, std::ios::app);
  csvFile << volt << "," << pressure << "," << mean_drift_speed << "," << sigma_drift_speed << "\n";
  csvFile.close();

  delete hSpeed;
}

void safe_run_simulation(double pressure, int voltage, ComponentComsol* model, const std::string& csvFileName) {
/*
This function runs simulation. But if for whatever reason we get stuck at a particular pressure or volume for a long time,
we move on instead of staying stuck.
*/

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        run_simulation(pressure, voltage, model, csvFileName);
        exit(0);
    } else if (pid > 0) {
        // Parent process: wait with timeout
        int status;
        int timeout_seconds = 1800;  // Adjust this
        int waited = 0;

        while (waitpid(pid, &status, WNOHANG) == 0 && waited < timeout_seconds) {
            sleep(1);
            waited++;
        }

        if (waited >= timeout_seconds) {
            std::cerr << "Timeout for pressure " << pressure << ", killing process\n";
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);  // Clean up
        }
    } else {
        std::cerr << "Fork failed\n";
    }
}

int main()
{
  std::string csvFileName = "drift_speed_results_gas_table_xenon_new_simulation.csv"; // !!!

  // Create or clear file, and write header only once
  if (!std::filesystem::exists(csvFileName)) {
    std::ofstream csvFile(csvFileName);
    csvFile << "Voltage[V],Pressure[Torr],MeanDriftSpeed[cm/us],StdDev[cm/us]\n";
    csvFile.close();
  }

  std::vector<int> voltages = {/*200,225,250,300, 350, 400, 500, 600, 700, 800, 850, 900, 1000,
    1100, 1200, 1300, 1400, 1500, 1600, 1603, 1700, 1800,*/ 1900};
  
  std::vector<double> pressures = {158.0272814,305.83624583,/*497.8134186,703.14866857,897.54054586,*/1000.95292865/*, 1003.96149327*/, 
    1005.96709465, 1106.13661436/*, 1304.82907969, 1498.69503398*/};

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
        safe_run_simulation(pressure, voltage, &pumaModel, csvFileName);
      }
    }
      
  return 0;
}