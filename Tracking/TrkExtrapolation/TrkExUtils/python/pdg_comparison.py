# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Short script to compare ATLAS material radiation/ionization losses with PDG tables

import matplotlib.pyplot as plt

import numpy as np

import ROOT
import urllib.request
import re
import ctypes

ROOT.gInterpreter.ProcessLine('#include "TrkExUtils/MaterialInteraction.h"')
ROOT.gSystem.Load('libTrkExUtils.so')

cm = 10
mm = 1
X0 = 21.82  #Radiation length
L0 =  108.4 #Nuclear interaction length, not used here
A =  28.0855
Z = 14
rho = 2.329 #given in g/cm^3

#Values for silicon from https://pdg.lbl.gov/2022/AtomicNuclearProperties/HTML/silicon_Si.html
#PDG values are in [MeV cm^2/g]
#ATLAS returns [MeV/mm]
#So multiply ATLAS values by cm/rho to match PDG 

conv = (cm/mm)/rho
mat = ROOT.Trk.Material(X0*conv, L0*conv, A, Z, rho/cm**3)
particle = ROOT.Trk.muon
sigma = ctypes.c_double()
kazL = ctypes.c_double()

pdg_silicon_url = 'https://pdg.lbl.gov/2023/AtomicNuclearProperties/MUE/muE_silicon_Si.txt'
pdg_silicon = urllib.request.urlretrieve(pdg_silicon_url)

fp = open(pdg_silicon[0])
#Regex to match the lines with numbers from the PDG file
starts_with_spaces_then_numbers = re.compile(r"^  [0-9]")

#x-axis: p
x_p = []
y_pdg_ion = []
y_atlas_ion = []
y_atlas_pdg_ion = []
y_pdg_rad = []
y_atlas_rad = []

for line in fp:
  if re.match(starts_with_spaces_then_numbers,line):
    numbers = line.split()
    p = float(numbers[1])
    ion = float(numbers[2])
    rad = float(numbers[6])
    atlas_ion = -ROOT.Trk.MaterialInteraction.dEdl_ionization(p, mat, particle, sigma, kazL)*conv
    atlas_pdg_ion = -ROOT.Trk.MaterialInteraction.PDG_energyLoss_ionization(p, mat, particle, sigma, kazL, 1.0)
    atlas_rad = -ROOT.Trk.MaterialInteraction.dEdl_radiation(p, mat, particle, sigma)*conv
    x_p.append(p)
    y_pdg_ion.append(ion)
    y_atlas_ion.append(atlas_ion)
    y_atlas_pdg_ion.append(atlas_pdg_ion)
    y_pdg_rad.append(rad)
    y_atlas_rad.append(atlas_rad)
    
    print("p: ", p)
    print("PDG ionization: ", ion)
    print("dEdl_ionization: ", atlas_ion)
    print("PDG_energyLoss_ionization: ", atlas_pdg_ion)
    print("PDG radiation: ", rad)
    print("dEdl_radiation: ", atlas_rad)
    

plt.plot(x_p, y_pdg_rad, label="PDG rad")
plt.plot(x_p, y_atlas_rad, label="ATLAS rad")
plt.xlabel ('p [MeV]')
plt.ylabel ('-<dE/dx> [MeV cm^2/g]')
plt.legend()
plt.savefig('dEdx_rad_vs_p.png')
plt.yscale('log')
plt.savefig('dEdx_rad_vs_p_log.png')
plt.clf()

plt.plot(x_p, y_pdg_ion, label="PDG ion")
plt.plot(x_p, y_atlas_ion, label="ATLAS ion")
plt.plot(x_p, y_atlas_pdg_ion, label="ATLAS 'PDG' ion")
plt.ylabel ('-<dE/dx> [MeV cm^2/g]')
plt.xlabel ('p [MeV]')
plt.legend()
plt.savefig('dEdx_ion_vs_p_all.png')
plt.yscale('log')
plt.savefig('dEdx_ion_vs_p_all_log.png')
plt.clf()

plt.plot(x_p, y_pdg_ion, label="PDG ion")
plt.plot(x_p, y_atlas_ion, label="ATLAS ion")
plt.ylabel ('-<dE/dx> [MeV cm^2/g]')
plt.xlabel ('p [MeV]')
plt.legend()
plt.savefig('dEdx_ion_vs_p.png')
plt.yscale('log')
plt.savefig('dEdx_ion_vs_p_log.png')
plt.clf()

plt.plot(x_p, y_pdg_ion, label="PDG ion")
plt.plot(x_p, y_atlas_ion, label="ATLAS ion")
plt.plot(x_p, y_atlas_pdg_ion, label="ATLAS 'PDG' ion")
plt.ylabel ('-<dE/dx> [MeV cm^2/g]')
plt.xlabel ('p [MeV]')
plt.legend()
plt.savefig('dEdx_ion_vs_p_all.png')
plt.xscale('log')
plt.xlim(100,1e6)
plt.ylim(0,5)
plt.savefig('dEdx_ion_vs_p_all_logx_100MeV_1TeV.png')
