# PUMA_work

This repository contains simulations and data analysis done for the PUMA purity monitor.

## Voltage_Pressure_Sims:
Garfield++ simulations of electron drift. This directory contains other subdirectories:
- Comsol_Files: data extracted from the COMSOL simulation. This is then used for the G++ sims.
- drift_sim: simulation of electron drift through the PUMA volume.
- efield_study: investigating the electric field extracted from the COMSOL simulation.
- gas_tables: gas tables used in the G++ simulations as well as the code needed to generate them.
- speed_distribution_analysis: analysing the distribution of electron drift speeds for debugging.

## Grid_Transparency_Analysis:
Analyzing data taken at TRIUMF's lab with little PUMA to determine the grid transparency of PUMA.