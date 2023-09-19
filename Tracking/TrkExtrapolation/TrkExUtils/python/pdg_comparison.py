# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Short script to compare ATLAS material radiation/ionization losses with PDG tables

import matplotlib.pyplot as plt

plt.style.use("tableau-colorblind10")

import ROOT
import ctypes

from AthenaCommon.SystemOfUnits import millimeter, centimeter

ROOT.gInterpreter.ProcessLine('#include "TrkExUtils/MaterialInteraction.h"')
ROOT.gSystem.Load("libTrkExUtils.so")

# Values for silicon from
# https://pdg.lbl.gov/2022/AtomicNuclearProperties/HTML/silicon_Si.html

# Momentum MeV
p = [
    92.85,
    100.3,
    107.4,
    114.3,
    121.0,
    127.6,
    140.3,
    152.7,
    164.7,
    176.4,
    199.4,
    221.8,
    254.6,
    286.8,
    339.6,
    363.3,
    391.7,
    443.2,
    494.5,
    545.5,
    596.4,
    647.1,
    697.7,
    798.7,
    899.5,
    1000.0,
    1101.0,
    1301.0,
    1502.0,
    1803.0,
    2103.0,
    2604.0,
    3104.0,
    3604.0,
    4104.0,
    4604.0,
    5105.0,
    5605.0,
    6105.0,
    7105.0,
    8105.0,
    9105.0,
    10110.0,
    12110.0,
    14110.0,
    17110.0,
    20110.0,
    25110.0,
    30110.0,
    35110.0,
    40110.0,
    45110.0,
    50110.0,
    55110.0,
    60110.0,
    70110.0,
    80110.0,
    90110.0,
    100100.0,
    120100.0,
    140100.0,
    170100.0,
    200100.0,
    250100.0,
    300100.0,
    350100.0,
    400100.0,
    450100.0,
    500100.0,
    550100.0,
    581600.0,
    600100.0,
    700100.0,
    800100.0,
    900100.0,
    1000000.0,
    1200000.0,
    1400000.0,
    1700000.0,
    2000000.0,
    2500000.0,
    3000000.0,
    3500000.0,
    4000000.0,
    4500000.0,
    5000000.0,
    5500000.0,
    6000000.0,
    7000000.0,
    8000000.0,
    9000000.0,
    10000000.0,
]

# PDG values for the radiation energy loss [MeV cm^2/g]
pdg_rad = [
    6.158e-05,
    6.377e-05,
    6.596e-05,
    6.839e-05,
    7.161e-05,
    7.486e-05,
    8.145e-05,
    8.815e-05,
    9.496e-05,
    0.0001019,
    0.000116,
    0.0001304,
    0.0001528,
    0.0001757,
    0.0002153,
    0.0002563,
    0.0002563,
    0.0002985,
    0.0003418,
    0.0003861,
    0.0004314,
    0.0004774,
    0.0005242,
    0.00062,
    0.0007182,
    0.0008186,
    0.0009391,
    0.001234,
    0.001547,
    0.002045,
    0.002575,
    0.003524,
    0.004531,
    0.005588,
    0.006687,
    0.007823,
    0.008992,
    0.01019,
    0.01141,
    0.01392,
    0.01652,
    0.0192,
    0.02194,
    0.02764,
    0.03356,
    0.04275,
    0.05227,
    0.06896,
    0.08631,
    0.1042,
    0.1226,
    0.1414,
    0.1605,
    0.1798,
    0.1994,
    0.2393,
    0.2801,
    0.3217,
    0.364,
    0.4491,
    0.5361,
    0.6696,
    0.806,
    1.034,
    1.267,
    1.505,
    1.745,
    1.989,
    2.235,
    2.478,
    2.632,
    2.723,
    3.216,
    3.715,
    4.218,
    4.726,
    5.735,
    6.754,
    8.298,
    9.857,
    12.44,
    15.04,
    17.66,
    20.3,
    22.95,
    25.61,
    28.25,
    30.9,
    36.21,
    41.55,
    46.9,
    52.27,
]
# PDG values for the mean energy loss [MeV cm^2/g]
pdg_mean_ion = [
    2.796,
    2.608,
    2.461,
    2.345,
    2.25,
    2.172,
    2.052,
    1.965,
    1.899,
    1.849,
    1.781,
    1.737,
    1.699,
    1.678,
    1.665,
    1.664,
    1.665,
    1.672,
    1.681,
    1.692,
    1.703,
    1.714,
    1.726,
    1.747,
    1.767,
    1.786,
    1.803,
    1.834,
    1.86,
    1.894,
    1.922,
    1.96,
    1.991,
    2.017,
    2.038,
    2.057,
    2.074,
    2.089,
    2.102,
    2.126,
    2.145,
    2.162,
    2.177,
    2.203,
    2.224,
    2.249,
    2.27,
    2.297,
    2.319,
    2.336,
    2.352,
    2.365,
    2.377,
    2.387,
    2.396,
    2.413,
    2.427,
    2.44,
    2.451,
    2.47,
    2.485,
    2.505,
    2.522,
    2.545,
    2.563,
    2.579,
    2.593,
    2.605,
    2.616,
    2.625,
    2.631,
    2.634,
    2.65,
    2.664,
    2.676,
    2.687,
    2.706,
    2.723,
    2.743,
    2.76,
    2.784,
    2.804,
    2.821,
    2.836,
    2.849,
    2.86,
    2.871,
    2.881,
    2.898,
    2.913,
    2.926,
    2.938,
]

# The ATLAS method return [MeV/mm] for the mean energy loss
# So multiply ATLAS values by cm/rho to match PDG
# The Atlas method returns [MeV] for the path dependent most
# propable energy loss, calculate with for 1cm and divide with rho


# silicon properties
X0 = 21.82  # Radiation length
L0 = 108.4  # Nuclear interaction length, not used here
A = 28.0855
Z = 14
rho = 2.329  # given in g/cm^3


conv = (centimeter / millimeter) / rho
mat = ROOT.Trk.Material(X0 * conv, L0 * conv, A, Z, rho / centimeter**3)
particle = ROOT.Trk.muon
sigma = ctypes.c_double()
kazL = ctypes.c_double()

# lets do the calculations and compare
atlas_mean_ion = []
atlas_rad = []
atlas_mop_ion = []
print("|p      | PDG Mean Ion | ATLAS Mean Ion | ATLAS Mop Ion | PDG rad | ATLAS rad |")
for i in range(len(p)):
    atlas_mean_ion.append(
        -ROOT.Trk.MaterialInteraction.dEdl_ionization(p[i], mat, particle, sigma, kazL)
        * conv
    )
    atlas_mop_ion.append(
        -ROOT.Trk.MaterialInteraction.dE_MPV_ionization(
            p[i], mat, particle, sigma, kazL, centimeter
        )
        / rho
    )
    atlas_rad.append(
        -ROOT.Trk.MaterialInteraction.dEdl_radiation(p[i], mat, particle, sigma) * conv
    )

    print(
        "|{0:10.5} | {1:10.6} | {2:10.6} | {3:10.6} | {4:10.6} | {5:10.6}".format(
            p[i],
            pdg_mean_ion[i],
            atlas_mean_ion[i],
            atlas_mop_ion[i],
            pdg_rad[i],
            atlas_rad[i],
        )
    )

plt.plot(p, pdg_rad, label="PDG rad", marker=".")
plt.plot(p, atlas_rad, label="ATLAS rad", marker="x")
plt.xlabel("p [MeV]")
plt.ylabel("-<dE/dx> [MeV cm^2/g]")
plt.legend()
plt.savefig("dEdx_rad_vs_p.png", dpi=300)
plt.clf()

plt.plot(p, pdg_mean_ion, label="PDG mean ion", marker=".", markersize=4)
plt.plot(p, atlas_mean_ion, label="ATLAS mean ion", marker="x", markersize=4)
plt.plot(p, atlas_mop_ion, label="ATLAS mpv ion", marker="+", markersize=4)
plt.ylabel("-<dE/dx> [MeV cm^2/g]")
plt.xlabel("p [MeV]")
plt.legend()
plt.xscale("log")
plt.ylim(0, 5)
plt.savefig("dEdx_ion_vs_p_all.png", dpi=300)
