import ROOT

"""
Looking at vdr vs distance to understand why the speed distribution is bimodal.
"""

if __name__ == "__main__":
    # Getting graph
    file = ROOT.TFile.Open("vdr_distance_P1498_V1000.root")
    graph = file.Get("vdr_vs_distance")

     # Create a canvas
    canvas = ROOT.TCanvas("c1", "Drift Velocity vs Distance", 800, 600)
    canvas.SetGrid()  # Add grid for better readability
    
    # Set graph properties
    #graph.SetTitle("Drift Velocity vs Distance;Distance [cm];Drift Velocity [cm/#mus]")
    graph.SetMarkerStyle(20)  # Filled circles
    graph.SetMarkerSize(1.0)
    graph.SetMarkerColor(ROOT.kBlue)
    graph.SetLineColor(ROOT.kBlue)
    graph.SetLineWidth(2)
    
    # Draw the graph
    graph.Draw("ALP")  # A = draw axes, L = line, P = points
    
    # Update the canvas to show the plot
    canvas.Update()
    
    # Optional: Save the plot
    canvas.SaveAs("vdr_vs_distance.png")
    
    # Keep the plot window open
    input("Press Enter to close...")
    
    # Close the file
    file.Close()