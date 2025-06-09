#include <TApplication.h>
#include <TCanvas.h>
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ViewMedium.hh"

using namespace Garfield;

int main(int argc, char* argv[]) {
    TApplication app("app", &argc, argv);

    MediumMagboltz gas;
    gas.LoadGasFile("ar_93_co2_7.gas");  // Loads pre-generated data

    ViewMedium mediumView;
    mediumView.SetMedium(&gas);
    mediumView.PlotElectronVelocity('e');

    TCanvas* c = (TCanvas*)gPad->GetCanvas();
    if (c) c->SaveAs("electron_velocity.png");

    return 0;
}