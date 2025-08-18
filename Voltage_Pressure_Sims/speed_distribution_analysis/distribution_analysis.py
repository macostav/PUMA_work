import ROOT
import numpy as np
import matplotlib.pyplot as plt

"""
Looking at vdr vs distance to understand why the speed distribution is bimodal.
"""

if __name__ == "__main__":
# Getting graph
    voltages = [1000, 1400, 1700, 1900] # [V] 
    colors = ['purple', 'blue', 'green', 'red']  # Define colors for each voltage

    for i, voltage in enumerate(voltages):
        title = "root_files/vdr_distance_P1498_V" + str(voltage) + ".root"
        file = ROOT.TFile.Open(title)
        graph = file.Get("vdr_distance")

        # Get the distances
        n_points = graph.GetN()
    
        # Get pointers arrays
        x_array = graph.GetX()
        y_array = graph.GetY()
        
        # Convert to Python lists
        distances = [np.abs(x_array[i]) for i in range(n_points)]
        distances_rounded = [round(x_array[i], 2) for i in range(n_points)] # Round distances (a lot of distance values are close)
        unique_distances = set(distances_rounded)
        vdr = [y_array[i] for i in range(n_points)]

        plt.scatter(distances, vdr, color=colors[i], label=f"HV={voltage} V")

        # Define arrays for velocities corresponding to short and long distances
        short_vdr = []
        long_vdr = []

        # We want to separarate the drift velocities based on the distance travelled
        for j, distance in enumerate(distances):
            short_d, long_d = 3.85, 4.04 # The two unique distances found above
            if np.abs(np.abs(distance) - short_d) < np.abs(np.abs(distance) - long_d): # if we are closer to short distance
                short_vdr.append(vdr[j])
            else:
                long_vdr.append(vdr[j])

        # Histograms for short and long distances overlaid
        fig_hist2, ax = plt.subplots(figsize=(10, 6))

        ax.hist(short_vdr, bins=100, alpha=0.6, color='blue', label=f'Short distance (≈{short_d} cm)', edgecolor='black')
        #ax.hist(long_vdr, bins=100, alpha=0.6, color='red', label=f'Long distance (≈{long_d} cm)', edgecolor='black')
        ax.set_xlabel('Drift Velocity [cm/μs]')
        ax.set_ylabel('Counts')
        ax.set_title(f'Drift Velocity Distribution in Argon - HV={voltage} V')
        ax.legend()
        ax.grid(True, alpha=0.3)
        plt.savefig(f'velocity_distribution_HV{voltage}V_short_distance.png', dpi=300, bbox_inches='tight')
        plt.close()  # Closes the figure to free memory


        # Close the file
        file.Close()

    plt.title("Drift Velocity vs Distance Travelled in Argon")
    plt.xlabel("Distance [cm]")
    plt.ylabel("Drift Velocity [cm/μs]")
    plt.legend()
    plt.grid(True)
    plt.savefig("vdr_distance.png")