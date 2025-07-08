import numpy as np
import matplotlib.pyplot as plt

"""
Plots the electric field vs z for the center of PUMA. This is done with HV 200V.
"""
if __name__ == "__main__":
    # Load the data
    #z_before, efield_before = np.loadtxt("efield_vs_z.txt", unpack=True)
    z_after, efield_after = np.loadtxt("../Efield_vs_z.txt", unpack=True)

    # Plot
    plt.figure(figsize=(8, 6))
    #plt.plot(z_before, efield_before, marker='o', linestyle='-', color='blue', markersize=4, label='Before Fix')
    plt.plot(z_after, efield_after, marker='o', linestyle='-', color='red', markersize=4, label = 'After Fix')

    # Region definitions
    region_edges = [0.307, 0.47, 1.361,2.096, 2.831, 3.566, 4.357, 4.520]  # Define your region boundaries here
    
    region_names = ["Anode-LowGrid", "LowGrid-R4", "R4-R3", "R3-R2", "R2-R1", "R1-UppGrid", "UppGrid-Cathode"]
    region_colors = ["#e7bcca", "#bfe1f9", "#c8f5cc", "#f5e1c2", "#e7bdee", "#d2bbf6", "#f6b9b2"]

    region_patches = []

    for i in range(len(region_edges) - 1):
        patch = plt.axvspan(region_edges[i], region_edges[i+1],
                            color=region_colors[i], alpha=0.5, label=region_names[i])
        region_patches.append(patch)

    # Labels and title with increased font size
    plt.title("Electric Field vs Z (Center of PUMA, 1600V HV)", fontsize=18)
    plt.xlabel("Z [cm]", fontsize=16)
    plt.ylabel("Electric Field [V/cm]", fontsize=16)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.grid(True)

    # Combine lines and regions in legend
    line_handles, line_labels = plt.gca().get_legend_handles_labels()
    #plt.legend(handles=line_handles + region_patches, fontsize=11, loc='upper right', frameon=True)

    plt.tight_layout()
    #plt.ylim(-500, 1000)  # Sets x-axis from 0 to 5
    plt.savefig("efield_vs_z.png", dpi=300)
    plt.show()
