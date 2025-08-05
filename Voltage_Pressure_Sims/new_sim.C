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
#include <TGraph.h>
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
  gas.SetComposition("Ar", 100.); // !!! Can change this
  //gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Xe+_P32_Xe.txt");
  gas.LoadIonMobility("/home/macosta/ella_work/PUMA_Tests/Simulations/IonMobility_Ar+_Ar.txt");

  std::string gasFileName = "gas_tables/argon_" + std::to_string(int(pressure)) + "Torr.gas"; // !!!
    
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
int nElectronsTarget = 10000; //
int nElectronsSimulated = 0;
int totalAttempts = 0; // will use for looking at geometric grid transparency

while (nElectronsSimulated < nElectronsTarget) {
  // Generate random photoelectron position in a circle on the top surface
  auto [x0, y0] = randInCircle();
  double z0 = 4.47; // [cm] ; note cathode is at 4.493 cm
  double t0 = 0.0;

  drift.DriftElectron(x0, y0, z0, t0);

  double x1, y1, z1, t1;
  int status;
  drift.GetEndPoint(x1, y1, z1, t1, status);
  totalAttempts++;
  nElectronsSimulated++;

  std::cout << "ENDPOINT: (" << x1 << "," << y1 << "," << z1 << ")\n" << std::endl; 
  /*
  if (t1 > t0 && z1 < 0.47) { // check that electron did not get trapped in grid

    double driftLength = sqrt((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) + (z1 - z0)*(z1 - z0));
    double dt = t1 - t0;         // ns

    double vDrift = driftLength / dt * 1e3; // cm/μs
    hSpeed->Fill(vDrift);
    nElectronsSimulated++;
  }*/
}


// Avalanche mc or avalanche microscopic

  double mean_drift_speed = hSpeed->GetMean();
  double sigma_drift_speed = hSpeed->GetStdDev();
  std::cout << "Mean drift speed: " << mean_drift_speed << " cm/μs\n";
  std::cout << "Standard deviation: " << sigma_drift_speed << " cm/μs\n";

  // Append results to csvFile
  std::ofstream csvFile(csvFileName, std::ios::app);
  csvFile << volt << "," << pressure << "," << mean_drift_speed << "," << sigma_drift_speed<< "," << totalAttempts << "\n";
  csvFile.close();

// START OF STUFF !!!
  
// Extract electric field along z-axis and create plot
const double zMin = 3.5; // cm
const double zMax = 4.6;   // cm
const int nPoints = 200;

// Create arrays for TGraph
double* zArray = new double[nPoints+1];
double* ezArray = new double[nPoints+1];

std::ofstream efieldFile("Efield_vs_z.txt");
efieldFile << "# z [cm]\tEz [V/cm]\n";

for (int i = 0; i <= nPoints; ++i) {
  double z = zMin + i * (zMax - zMin) / nPoints;
  std::array<double, 3> efield = pumaModel->ElectricField(0.0, 0.0, z);
  double ez = efield[2];
  efieldFile << z << "\t" << ez << "\n";
  
  // Fill arrays for TGraph
  zArray[i] = z;
  ezArray[i] = ez;
}

efieldFile.close();
std::cout << "Electric field profile saved to Efield_vs_z.txt\n";

// Create TGraph for continuous function plot
TGraph* grEfield = new TGraph(nPoints+1, zArray, ezArray);
grEfield->SetTitle("Electric Field vs Z;Z [cm];E_{z} [V/cm]");
grEfield->SetLineColor(kBlue);
grEfield->SetLineWidth(2);
grEfield->SetMarkerStyle(0); // No markers, just line

// Create canvas and plot
TCanvas* c1 = new TCanvas("c1", "Electric Field Profile", 800, 600);
c1->SetGrid();

grEfield->Draw("AL"); // A = with axes, L = line

// Save as PNG
c1->SaveAs("Efield_vs_z.png");
std::cout << "Electric field plot saved to Efield_vs_z.png\n";

// Clean up
delete[] zArray;
delete[] ezArray;
delete grEfield;
delete c1;

  // END OF STUFF !!!

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
        int timeout_seconds = 2500;  // Adjust this
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
  std::string csvFileName = "drift_speed_results_gas_table_argon_new_simulation.csv"; // !!!

  // Create or clear file, and write header only once
  if (!std::filesystem::exists(csvFileName)) {
    std::ofstream csvFile(csvFileName);
    csvFile << "Voltage[V],Pressure[Torr],MeanDriftSpeed[cm/us],StdDev[cm/us],TotalAttempts\n";
    csvFile.close();
  }

  std::vector<int> voltages = {/*200,225,250,300, 350, 400, 500, 600, 700, 800, 850, 900, 1000,
    1100, 1200, 1300, 1400, 1500, 1600, 1603, 1700, 1800,*/ 1900};
  
  std::vector<double> pressures = {158.0272814,305.83624583, 497.8134186 ,703.14866857,897.54054586,1000.95292865, 1003.96149327, 
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
        safe_run_simulation(pressure, voltage, &pumaModel, csvFileName);
      }
    }
      
  return 0;
}