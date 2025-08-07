import csv
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
from matplotlib.cm import ScalarMappable
from matplotlib.ticker import ScalarFormatter
from collections import defaultdict

"""
We look at points that are strange in the E/N plots and try to figure out why they lie flat.
"""

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

def E_to_HV(E):
    """
    Using E-field values get back the HV that was used.

    :param E: The electric field in V/cm
    """
    d = 0.735 # cm
    RTot = 1209.7 #MOhms

    curr = (E*d)/196
    HV = curr*RTot
    return HV



if __name__ == "__main__":
    pressure_data = defaultdict(lambda: {"voltages": [], "speeds": [], "errors": []})

    with open("../data/drift_speed_results_gas_table_argon.csv", newline='') as csvfile: #!!!
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
    all_E_over_P = [] # V / cm Torr
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
        e_over_P = efields / pressure

        speeds = np.array(data["speeds"]) * 1e6  # cm/us → cm/s
        errors = np.array(data["errors"]) * 1e6

        all_E_over_P.extend(e_over_P)
        all_E_over_N.extend(e_over_N)
        all_speeds.extend(speeds)
        all_errors.extend(errors)
        all_fields.extend(efields)

    all_E_over_P = np.array(all_E_over_P)
    all_E_over_N = np.array(all_E_over_N)
    all_speeds = np.array(all_speeds)
    all_errors = np.array(all_errors)
    all_fields = np.array(all_fields)

    # Normalize color based on E-field
    cmap = plt.cm.plasma
    norm = Normalize(vmin=np.min(all_fields), vmax=np.max(all_fields))
    sm = ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])

    # Set your E-field threshold
    E_threshold = 150  # V/cm

    # Finding the indices of points that are below the threshold
    under_indices = []

    for i in range(len(all_fields)):
        if all_fields[i] <= E_threshold:
            under_indices.append(i)

    weird_pressures = []
    for index in under_indices:
        value = all_E_over_N[index]
        E_field = all_fields[index]
        V = E_to_HV(E_field)
        N = E_field/value
        P = N*k*T*(10)**(6) # in Pa
        P_torr = P * 0.00750062
        speed = all_speeds[index]
        weird_pressures.append((P_torr, V, speed ))

    print(f"Weird values (P, V, speed): {weird_pressures}")
    #print(len(weird_pressures)/8)


    # Create masks for high and low E-field points
    high_E_mask = all_fields >= E_threshold
    low_E_mask = all_fields < E_threshold

    # Plot high E-field points with color mapping
    scatter1 = ax.scatter(all_E_over_P[high_E_mask], all_speeds[high_E_mask], 
                        c=all_fields[high_E_mask], cmap=cmap, norm=norm, label='High E-field')

    # Plot low E-field points with fixed color
    scatter2 = ax.scatter(all_E_over_P[low_E_mask], all_speeds[low_E_mask], 
                        c='red', marker='s', label=f'E < {E_threshold} V/cm')  # Fixed red color, square markers

    # Add error bars for both
    ax.errorbar(all_E_over_P[high_E_mask], all_speeds[high_E_mask], yerr=all_errors[high_E_mask], 
            fmt='none', ecolor='gray', alpha=0.5, capsize=2)
    ax.errorbar(all_E_over_P[low_E_mask], all_speeds[low_E_mask], yerr=all_errors[low_E_mask], 
            fmt='none', ecolor='red', alpha=0.3, capsize=2)

    ax.legend()

    ax.set_xlabel(r"$E/P$ [V$/ cm\cdot$Torr]")
    ax.set_ylabel("Drift Speed [cm/s]")
    ax.set_title("Argon Simulation") # !!!

    cbar = fig.colorbar(sm, ax=ax)
    cbar.set_label("Electric Field [V/cm]")

    plt.tight_layout()
    plt.savefig("test_other_format.png", dpi=300) # !!!