import numpy as np
import matplotlib.pyplot as plt

"""
Plots the electric field vs z for the center of PUMA.
"""
if __name__ == "__main__":
    # Load the data
    z_center, efield_center = np.loadtxt("Efield_vs_z_outside.txt", unpack=True)
    #z_in, efield_in = np.loadtxt("../Potential_vs_z_small_offset.txt", unpack = True)
    #z_out, efield_out = np.loadtxt("../Potential_vs_z_outside.txt", unpack = True)

    # Plot
    plt.figure(figsize=(8, 6))
    plt.plot(z_center, efield_center, marker='o', linestyle='-', color='red', markersize=4, label = 'Center of PUMA')
    #plt.plot(z_in, efield_in, marker = 'o', linestyle = "-", color = 'blue', markersize=4, label = 'Small Offset from Center (x = 3mm)')
    #plt.plot(z_out, efield_out, marker ='o', linestyle = "-", color = "green", markersize=4, label = 'Outside of Cathode (x = 6mm)')

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
    plt.title("Electric Potential vs Z (1600V HV)", fontsize=18)
    plt.xlabel("Z [cm]", fontsize=16)
    plt.ylabel("Electric Potential [V]", fontsize=16)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.grid(True)

    # Combine lines and regions in legend
    line_handles, line_labels = plt.gca().get_legend_handles_labels()
    #plt.legend(handles=line_handles + region_patches, fontsize=11, loc='upper right', frameon=True)
    plt.legend()

    plt.tight_layout()
    #plt.xlim(0.2, 4.7)
    plt.ylim(-500,1000)
    plt.savefig("potential_vs_z.png", dpi=300)
