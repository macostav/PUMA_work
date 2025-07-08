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
    mg = ROOT.TMultiGraph() # Contains the graphs for the individual pressures

    colors = [ROOT.kRed, ROOT.kBlue, ROOT.kGreen+3, ROOT.kMagenta, ROOT.kOrange+7, ROOT.kYellow-3, ROOT.kBlack, ROOT.kCyan+3]
    color_index = 0

    legend = ROOT.TLegend(0.7, 0.7, 0.9, 0.9)

    for pressure, data in pressure_data.items():
        n = len(data["voltages"])
        g = ROOT.TGraphErrors(n)

        for i in range(n):
            g.SetPoint(i, data["voltages"][i], data["speeds"][i])
            g.SetPointError(i, 0, data["errors"][i])  # X error = 0, Y error = SD

        g.SetMarkerStyle(20)
        g.SetMarkerColor(colors[color_index % len(colors)])
        g.SetLineColor(colors[color_index % len(colors)])
        g.SetTitle(f"{pressure:.1f} Torr")

        mg.Add(g)
        legend.AddEntry(g, f"{pressure:.1f} Torr", "lp")
        color_index += 1

    # Draw and format
    mg.Draw("APL")
    mg.SetTitle(";Voltage [V];Drift Speed [cm/s]")
    legend.Draw()
    c.Update()
    c.SaveAs("drift_speed_vs_voltage.png")  # or .pdf or .root
    