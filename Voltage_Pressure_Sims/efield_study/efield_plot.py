import numpy as np
import matplotlib.pyplot as plt

"""
Plots the electric field vs z for the center of PUMA.
"""
if __name__ == "__main__":
    # Load the data
    z, efield = np.loadtxt("Efield_vs_z_avg_HV1900.txt", unpack=True)

    # Plot
    plt.figure(figsize=(8, 6))
    plt.plot(z, efield, marker='o', linestyle='-', color='red', markersize=4)

    # Region definitions
    region_edges = [0.307, 0.47, 1.361,2.096, 2.831, 3.566, 4.357, 4.520]  # Define your region boundaries here
    
    region_names = ["Anode-LowGrid", "LowGrid-R4", "R4-R3", "R3-R2", "R2-R1", "R1-UppGrid", "UppGrid-Cathode"]
    region_colors = ["#e7bcca", "#bfe1f9", "#c8f5cc", "#f5e1c2", "#e7bdee", "#d2bbf6", "#f6b9b2"]

    region_patches = []

    for i in range(len(region_edges) - 1):
        patch = plt.axvspan(region_edges[i], region_edges[i+1],
                            color=region_colors[i], alpha=0.5) # label=region_names[i]
        region_patches.append(patch)

    # Labels and title with increased font size
    plt.title("Average Electric Field vs Z (1900V HV)", fontsize=18)
    plt.xlabel("Z [cm]", fontsize=16)
    plt.ylabel("Electric Field [V/cm]", fontsize=16)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.grid(True)

    # Combine lines and regions in legend
    line_handles, line_labels = plt.gca().get_legend_handles_labels()
    #plt.legend(handles=line_handles + region_patches, fontsize=11, loc='upper right', frameon=True)
    #plt.legend()

    plt.tight_layout()
    #plt.xlim(0.2, 4.7)
    plt.ylim(-500,1500)
    plt.savefig("avg_efield_vs_z.png", dpi=300)
