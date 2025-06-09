import ROOT
import array
import pandas as pd
import matplotlib.pyplot as plt
import csv
from collections import defaultdict

"""Makes drift speed vs voltage plots for different pressures."""

if __name__ == "__main__":
    # Data containers
    pressure_data = defaultdict(lambda: {"voltages": [], "speeds": [], "errors": []})

    # Read the CSV
    with open("drift_speed_stuff/pressures_voltages.csv", newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            V = float(row["Voltages [V]"])
            P = float(row["Pressures [Tor]"])
            S = float(row["Drift Speed [cm/s]"])
            E = float(row["SD [cm/s]"])
            pressure_data[P]["voltages"].append(V)
            pressure_data[P]["speeds"].append(S)
            pressure_data[P]["errors"].append(E)

    # Set up canvas
    c = ROOT.TCanvas("c", "Drift Speed vs Voltage", 800, 600)
    mg = ROOT.TMultiGraph()

    # Get pressure range for color mapping
    pressures = list(pressure_data.keys())
    min_pressure = min(pressures)
    max_pressure = max(pressures)

    # Create more distinct colors manually
    distinct_colors = [ROOT.kRed, ROOT.kBlue, ROOT.kGreen+2, ROOT.kMagenta, 
                    ROOT.kOrange+7, ROOT.kCyan+2, ROOT.kYellow+2, ROOT.kViolet,
                    ROOT.kSpring-6, ROOT.kTeal+2, ROOT.kPink-9, ROOT.kAzure+3]

    # Function to get color from pressure value with better distinction
    def get_color_from_pressure(pressure, pressures_list):
        # Find index of this pressure in sorted list
        sorted_pressures = sorted(pressures_list)
        pressure_index = sorted_pressures.index(pressure)
        # Use distinct colors, cycling if we have more pressures than colors
        return distinct_colors[pressure_index % len(distinct_colors)]

    # Also create a smooth palette for the color bar display
    ROOT.gStyle.SetPalette(ROOT.kRainBow)

    # Plot each pressure as before, but with colors from palette
    for pressure, data in pressure_data.items():
        n = len(data["voltages"])
        g = ROOT.TGraphErrors(n)

        for i in range(n):
            g.SetPoint(i, data["voltages"][i], data["speeds"][i])
            g.SetPointError(i, 0, data["errors"][i])  # X error = 0, Y error = SD

        # Get color based on pressure value
        color = get_color_from_pressure(pressure, pressures)
        g.SetMarkerStyle(20)
        g.SetMarkerColor(color)
        g.SetLineColor(color)
        g.SetTitle(f"{pressure:.1f} Torr")

        mg.Add(g)

    # Draw the multigraph first
    mg.Draw("APL")
    mg.SetTitle(";Voltage [V];Drift Speed [cm/s]")

    # Update canvas to establish coordinate system
    c.Update()

    # Create a histogram for the color bar that maps our actual pressure values
    n_pressures = len(pressures)
    h_colorbar = ROOT.TH2F("colorbar", "", 1, 0, 1, n_pressures, min_pressure, max_pressure)

    # Fill with pressure values
    for i, pressure in enumerate(sorted(pressures)):
        h_colorbar.SetBinContent(1, i+1, pressure)

    # Draw the color bar in the right margin
    # Save current pad
    c.cd()

    # Create a sub-pad for the color bar
    pad_colorbar = ROOT.TPad("colorbar", "colorbar", 0.85, 0.1, 0.95, 0.9)
    pad_colorbar.Draw()
    pad_colorbar.cd()

    # Draw the color bar
    h_colorbar.Draw("COLZ")
    h_colorbar.GetYaxis().SetTitle("Pressure [Torr]")
    h_colorbar.GetYaxis().SetTitleSize(0.05)
    h_colorbar.GetYaxis().SetLabelSize(0.04)
    h_colorbar.GetXaxis().SetLabelSize(0)  # Hide x-axis labels
    h_colorbar.GetZaxis().SetLabelSize(0)  # Hide z-axis labels

    # Go back to main pad
    c.cd()

    c.Update()
    c.SaveAs("drift_speed_vs_voltage_colorbar.png")  # or .pdf or .root