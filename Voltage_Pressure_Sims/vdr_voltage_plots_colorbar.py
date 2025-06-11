import csv
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict
from matplotlib.ticker import ScalarFormatter

"""Makes drift speed vs voltage plots for different pressures. We add a colorbar for the pressure values."""

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

    # Set up the plot
    fig, ax = plt.subplots(figsize=(8, 6))

    # Set up scientific notation in y-axis
    ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
    ax.ticklabel_format(axis='y', style='sci', scilimits=(0, 0))

    # Get sorted list of pressures for consistent color mapping
    pressures = sorted(pressure_data.keys())
    norm = plt.Normalize(min(pressures), max(pressures))
    cmap = plt.cm.Reds  # color map ; other good option is "plasma"

    # Plot each dataset
    for pressure in pressures:
        data = pressure_data[pressure]
        voltages = np.array(data["voltages"])
        speeds = np.array(data["speeds"])
        errors = np.array(data["errors"])

        color = cmap(norm(pressure))
        ax.errorbar(voltages, speeds, yerr=errors, fmt='o-', color=color,
                    label=f"{pressure:.1f} Torr", capsize=3)

    # Labels and title
    ax.set_title("")
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
    plt.tight_layout()
    plt.savefig("drift_speed_vs_voltage_matplotlib2.png", dpi=300)