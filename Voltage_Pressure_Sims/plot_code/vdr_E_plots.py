import csv
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
from matplotlib.cm import ScalarMappable
from matplotlib.ticker import ScalarFormatter
from collections import defaultdict

def HV_to_E(HV):
    '''
    Using the values I currently have in efield.xlsx, calculate the 
    electric field in drift region 3 based on a given HV.
    - The distance d needs to be checked

    :param int HV: The high voltage set between anode and cathode.
    '''

    RTot = 1209.7 #MOhms
    current = HV/RTot #uA

    d = 0.735 #cm
    Vdrop_drift3 = 196*current #V
    E_drift3 = Vdrop_drift3/d #V/cm
    return E_drift3

if __name__ == "__main__":
    pressure_data = defaultdict(lambda: {"voltages": [], "speeds": [], "errors": []})

    with open("../data/drift_speed_results_gas_table_xenon.csv", newline='') as csvfile: #!!!
        reader = csv.DictReader(csvfile)
        for row in reader:
            V = float(row["Voltage[V]"])
            P = float(row["Pressure[Torr]"])
            S = float(row["MeanDriftSpeed[cm/us]"])
            E = float(row["StdDev[cm/us]"])
            pressure_data[P]["voltages"].append(V)
            pressure_data[P]["speeds"].append(S)
            pressure_data[P]["errors"].append(E)

    fig, ax = plt.subplots(figsize=(8, 6))
    ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
    ax.ticklabel_format(axis='y', style='sci', scilimits=(0, 0))

    # d = 42.13e-1  # m
    k = 1.380649e-23  # J/K
    T = 293.15  # K

    all_E_over_N = []
    all_speeds = []
    all_errors = []
    all_fields = []

    for pressure in pressure_data:
        P_Pa = pressure * 133.322  # Convert to Pascals
        N = (P_Pa / (k * T))*10**(-6)  # cm^-3

        data = pressure_data[pressure]
        voltages = np.array(data["voltages"])
        efields = np.array([HV_to_E(voltage) for voltage in voltages])
        e_over_N = efields / N  # V*cm^2

        speeds = np.array(data["speeds"]) * 1e6  # cm/us → cm/s
        errors = np.array(data["errors"]) * 1e6

        all_E_over_N.extend(e_over_N)
        all_speeds.extend(speeds)
        all_errors.extend(errors)
        all_fields.extend(efields)

    all_E_over_N = np.array(all_E_over_N)
    all_speeds = np.array(all_speeds)
    all_errors = np.array(all_errors)
    all_fields = np.array(all_fields)

    # Normalize color based on E-field
    cmap = plt.cm.plasma
    norm = Normalize(vmin=np.min(all_fields), vmax=np.max(all_fields))
    sm = ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])

    scatter = ax.scatter(all_E_over_N, all_speeds, c=all_fields, cmap=cmap, norm=norm)
    ax.errorbar(all_E_over_N, all_speeds, yerr=all_errors, fmt='none', ecolor='gray', alpha=0.5, capsize=2)

    ax.set_xlabel(r"$E/N$ [V$\cdot$cm$^2$]")
    ax.set_ylabel("Drift Speed [cm/s]")
    ax.set_title("Xenon Simulation") # !!!

    cbar = fig.colorbar(sm, ax=ax)
    cbar.set_label("Electric Field [V/cm]")

    plt.tight_layout()
    plt.savefig("vdr_EN_xenon.png", dpi=300) # !!!