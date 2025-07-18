import csv
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict
from matplotlib.ticker import ScalarFormatter

"""Makes drift speed vs voltage plots for different pressures. We add a colorbar for the pressure values."""

if __name__ == "__main__":
    # Data containers
    pressure_data_old = defaultdict(lambda: {"voltages": [], "speeds": [], "errors": []})
    pressure_data_new = defaultdict(lambda: {"voltages": [], "speeds": [], "errors": []})

    # Read the CSV
    with open("../../data/drift_speed_results_gas_table_argon.csv", newline='') as csvfile: # !!!
        reader = csv.DictReader(csvfile)
        for row in reader:
            V = float(row["Voltage[V]"])
            P = float(row["Pressure[Torr]"])
            S = float(row["MeanDriftSpeed[cm/us]"])
            E = float(row["StdDev[cm/us]"])
            pressure_data_old[P]["voltages"].append(V)
            pressure_data_old[P]["speeds"].append(S)
            pressure_data_old[P]["errors"].append(E)

    # Read the CSV
    with open("../../drift_speed_results_gas_table_argon_new_simulation.csv", newline='') as csvfile: # !!!
        reader = csv.DictReader(csvfile)
        for row in reader:
            V = float(row["Voltage[V]"])
            P = float(row["Pressure[Torr]"])
            S = float(row["MeanDriftSpeed[cm/us]"])
            E = float(row["StdDev[cm/us]"])
            pressure_data_old[P]["voltages"].append(V)
            pressure_data_old[P]["speeds"].append(S)
            pressure_data_old[P]["errors"].append(E)

    # Set up the plot
    fig, ax = plt.subplots(figsize=(8, 6))

    # Set up scientific notation in y-axis
    ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
    ax.ticklabel_format(axis='y', style='sci', scilimits=(0, 0))

    # Get sorted list of pressures for consistent color mapping
    pressures = sorted(pressure_data_old.keys())
    norm = plt.Normalize(min(pressures), max(pressures))
    cmap = plt.cm.rainbow  # color map ; other good option is "plasma"

    # Plot each dataset
    for pressure in pressures:
        data = pressure_data_old[pressure]
        voltages = np.array(data["voltages"])
        speeds = np.array(data["speeds"]) * 1e6
        errors = np.array(data["errors"]) * 1e6

        color = cmap(norm(pressure))
        ax.errorbar(voltages, speeds, yerr=errors, fmt='o-', color=color,
                    label=f"{pressure:.1f} Torr", capsize=3)

    # Labels and title
    ax.set_title("Argon Simulation") # !!!
    ax.set_xlabel("Voltage [V]")
    ax.set_ylabel("Drift Speed [cm/s]")

    # Add a color bar
    sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])  # only needed for colorbar
    cbar = plt.colorbar(sm, ax=ax)
    cbar.set_label("Pressure [Torr]")

    # Can add a legend to get explicit pressure labels
    # ax.legend(loc="best", fontsize=8)

    # Save or show the plot
    #plt.xlim(475, 650)
    #plt.ylim(2e5,7e5)
    plt.tight_layout()
    plt.savefig("vdr_voltage_argon_new_sim.png", dpi=300) # !!!