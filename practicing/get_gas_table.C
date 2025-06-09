#include <cstdlib>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
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

// We get the gas table with this script

using namespace Garfield;

int main(int argc, char* argv[]) {
    // Set up gas
    MediumMagboltz gas;
    gas.SetComposition("ar", 93., "co2", 7.);
    gas.SetPressure(760.);
    gas.SetTemperature(273.);
    gas.SetFieldGrid(100., 100000., 20, true);
    gas.GenerateGasTable(10);
    gas.WriteGasFile("ar_93_co2_7.gas");

    return 0;
}