# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

##===============================================================================
## Name:        TrigEgammaElectronIsEMCutDefs_medium1.py
##
## Author:      Ryan Mackenzie White
## Created:     June 2013
##
## Description: Medium1 trigger electron cut definitions for 2012. 
##              Migrated from TrigEgammaElectronCutIDTool_medium1.py LowLumi_2012
##===============================================================================

import PyCintex
PyCintex.loadDictionary('ElectronPhotonSelectorToolsDict')
from ROOT import egammaPID

# Import a needed helper
from PATCore.HelperUtils import *

# Define GeV
GeV = 1000.0
def TrigEgammaElectronIsEMSelectorConfig2011_medium1(theTool) :
    
    '''
    This is for the Medium++ isEM definitions for the Trigger.
    '''
    
    theTool = GetTool(theTool)

    theTool.PIDName = egammaPID.IsEMMedium
 

    # range of eta bins for e-ID
    theTool.CutBinEta = [0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
        
    # range of ET bins for e-ID
    theTool.CutBinET = [5.0*GeV, 10.0*GeV, 15.0*GeV, 20.0*GeV, 30.0*GeV, 40.0*GeV, 50.0*GeV, 60.0*GeV, 70.0*GeV, 80.0*GeV]
    # cut on fraction of energy deposited in 1st sampling
    theTool.CutF1 = [0.005]
    # cut on hadronic energy
    theTool.CutHadLeakage  = [ 0.031,  0.031, 0.021,  0.021,  0.019,  0.028,  0.065,  0.065,  0.046,  0.034, # < 5
                            0.018,  0.018, 0.016,  0.015,  0.016,  0.028,  0.053,  0.038,  0.028,  0.025, # 5-10
                            0.015,  0.015, 0.0150, 0.015,  0.013,  0.033,  0.030,  0.028,  0.015,  0.015, # 10-15 
                            0.0125, 0.0125,0.0120, 0.0120, 0.0115, 0.029,  0.027,  0.020,  0.011,  0.011, # 15-20 
                            0.010,  0.010, 0.010,  0.010,  0.010,  0.010,  0.020,  0.015,  0.008,  0.008, # 20-30
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, # 30-40
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, #40-50
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, # 50-60
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, # 60-70
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, # 70-80
                            0.008,  0.008, 0.008,  0.008,  0.008,  0.010,  0.015,  0.015,  0.008,  0.008, # 80<
                            ]
    # cut on ratio e237/e277
    theTool.CutReta37  = [ 0.700, 0.700, 0.700, 0.700, 0.700, 0.690, 0.848, 0.876, 0.870, 0.888,  # < 5
                        0.700, 0.700, 0.700, 0.700, 0.700, 0.715, 0.860, 0.880, 0.880, 0.880, # 5-10
                        0.895, 0.895, 0.890, 0.885, 0.885, 0.740, 0.870, 0.885, 0.890, 0.900, # 10-15 
                        0.915, 0.915, 0.915, 0.915, 0.910, 0.740, 0.895, 0.900, 0.900, 0.910, # 15-20 
                        0.930, 0.930, 0.930, 0.925, 0.925, 0.750, 0.915, 0.915, 0.910, 0.910, # 20-30
                        0.930, 0.930, 0.930, 0.925, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, # 30-40
                        0.930, 0.930, 0.930, 0.925, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, # 40-50
                        0.930, 0.930, 0.930, 0.930, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, # 50-60
                        0.930, 0.930, 0.930, 0.930, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, # 60-70
                        0.930, 0.930, 0.930, 0.930, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, # 70-80
                        0.930, 0.930, 0.930, 0.930, 0.925, 0.790, 0.915, 0.920, 0.910, 0.910, #80<
                        ]
    # cut on shower width in 2nd sampling
    theTool.CutWeta2c  = [ 0.014, 0.014, 0.014, 0.014, 0.014, 0.028, 0.017, 0.014, 0.014, 0.014, #< 5
                        0.013, 0.013, 0.014, 0.014, 0.014, 0.026, 0.017, 0.014, 0.014, 0.014, # 5-10
                        0.013, 0.013, 0.013, 0.013, 0.013, 0.025, 0.014, 0.014, 0.013, 0.013, # 10-15 
                        0.012, 0.012, 0.013, 0.013, 0.013, 0.025, 0.014, 0.014, 0.013, 0.013, # 15-20 
                        0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.014, 0.013, 0.0125,0.0125,# 20-30
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.013, 0.0125,0.0125,# 30-40
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.013, 0.0125,0.0125,# 40-50
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.012, 0.0125,0.0125,# 50-60
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.012, 0.0125,0.0125,# 60-70
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.012, 0.0125,0.0125,# 70-80
                        0.011, 0.011, 0.012, 0.012, 0.012, 0.025, 0.013, 0.012, 0.0125,0.0125,# 80<
                        ]
    # cut on total width in 1st sampling
    theTool.CutWtot  = [3.48, 3.48, 3.78, 3.96, 4.20, 9999., 4.02, 2.70, 1.86,  9999.,  #  5    GeV
                      3.18, 3.18, 3.54, 3.90, 4.02, 9999., 3.96, 2.70, 1.80,  9999.,  # 5-10
                      2.85, 2.85, 3.20, 3.40, 3.60, 9999., 3.70, 2.40, 1.72,  9999.,  # 10-15 
                      2.76, 2.76, 2.92, 3.30, 3.45, 9999., 3.67, 2.40, 1.72,  9999.,  # 15-20 
                      2.50, 2.50, 2.70, 3.14, 3.23, 9999., 3.58, 2.32, 1.59,  9999.,  # 20-30
                      2.45, 2.45, 2.70, 2.98, 3.17, 9999., 3.52, 2.25, 1.58,  9999.,  # 30-40
                      2.27, 2.27, 2.61, 2.90, 3.17, 9999., 3.36, 2.25, 1.55,  9999.,  # 40-50
                      2.27, 2.27, 2.61, 2.90, 3.17, 9999., 3.36, 2.25, 1.55,  9999.,  # 50-60
                      2.27, 2.27, 2.61, 2.90, 3.17, 9999., 3.36, 2.25, 1.55,  9999.,  # 60-70
                      2.27, 2.27, 2.61, 2.90, 3.17, 9999., 3.36, 2.25, 1.55,  9999.,  # 70-80
                      2.27, 2.27, 2.61, 2.90, 3.17, 9999., 3.36, 2.25, 1.55,  9999.,  # 80<
                      ]
    # cut on (Emax - Emax2)/(Emax + Emax2) in 1st sampling
    theTool.CutDEmaxs1 = [ 0.390, 0.390, 0.200,  0.070, 0.060, -9999., 0.070, 0.430, 0.750, -9999.,  # < 5
                        0.610, 0.601, 0.320,  0.101, 0.130, -9999., 0.120, 0.510, 0.620, -9999.,  # 5-10
                        0.800, 0.800, 0.790,  0.660, 0.650, -9999., 0.650, 0.860, 0.850, -9999.,  # 10-15 
                        0.830, 0.830, 0.825,  0.730, 0.670, -9999., 0.750, 0.900, 0.890, -9999.,  # 15-20 
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 20-30
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 30-40
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 40-50
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 50-60
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 60-70
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 70-80
                        0.835, 0.835, 0.835,  0.730, 0.700, -9999., 0.800, 0.900, 0.910, -9999.,  # 80<
                        ]
    # cut on Delta Emax2 in 1st sampling 
    theTool.CutDeltaEmax2  = []
            
    # cut on Emax2 - Emin in 1st sampling 
    theTool.CutDeltaE  = []
            
    # cut on width in 1st sampling
    theTool.CutWeta1c  = []
            
    # cut on Fside in 1st sampling
    theTool.CutFracm = []
            
    # cut on Energy in cone of 0.20
    # not used by tool
    # theTool.CutClusterIsolation = []
            
    # cut on Track quality cut
            
    # cut on pixel-layer hits
    theTool.CutPi  = [1, 1, 1, 1, 1, 1, 1, 1, 2, 2]
            
    # cut on precision hits
    theTool.CutSi  = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
            
    # cut on transverse impact parameter
    theTool.CutA0  = [5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0]
            
    # cut on transverse impact parameter for tight selection   	 	 
    theTool.CutA0Tight  = [5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0]
            
    # cut on delta eta
    theTool.CutDeltaEta = [0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                        0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005
                        ]
    # cut on delta eta for tight selection
    theTool.CutDeltaEtaTight = [0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
                            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005
                            ]

    theTool.useTRTOutliers  = True
    theTool.useBLOutliers  = True
    theTool.usePIXOutliers  = True
    theTool.useSCTOutliers  = True  
            
    # cut on b-layer hits
    theTool.CutBL  = [1, 1, 1, 1, 1, 1, 1, 1, 0, 0]

    # cut max on delta phi
    theTool.CutmaxDeltaPhi  = [ 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
                            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015
                            ]
            
    # cut min on deltaphi
    theTool.CutminDeltaPhi  = [ -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
                            ]

    # cut min on E/P
    theTool.CutminEp  = [0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
                        0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
                        0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90,
                        0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90,
                        0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
                        0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
                        0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
                        0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
                        0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
                        0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
                        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                        ]
    # cut max on E/P
    theTool.CutmaxEp  = [2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0, 3.0, 3.0,
                        2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0, 3.0, 3.0,
                        2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.0, 4.0,
                        2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.0, 4.0,
                        2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.5, 4.5,
                        3.0, 3.0, 3.0, 3.0, 3.0, 3.5, 3.5, 4.0, 4.5, 4.5,
                        3.0, 3.0, 3.0, 3.0, 3.0, 3.5, 4.0, 5.0, 5.0, 5.0,
                        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                        10., 10., 10., 10., 10., 10., 10., 10., 10., 10.
                        ]
            
    # cuts on TRT
    # range of eta bins for e-ID for TRT
    theTool.CutBinEta_TRT = [0.1, 0.625, 1.07, 1.304, 1.752, 2.0]
    # cuts on Number of TRT hits with Outliers
    theTool.CutNumTRT  = [-15.,-15., -15., -15., -15., -15.]
    # cuts on TRT ratio with Outliers
    theTool.CutTRTRatio  = [0.05, 0.05, 0.05, 0.06, 0.08, 0.08]
    # cuts on TRT ratio with Outliers for 90% efficiency
    theTool.CutTRTRatio90  = [0.10, 0.10, 0.125, 0.13, 0.13, 0.13]


def TrigEgammaElectronIsEMSelectorConfig2012_medium1(theTool) :
    '''
    This is for the Medium++ isEM definitions for the Trigger.
    '''
    
    theTool = GetTool(theTool)

    theTool.PIDName = egammaPID.IsEMMedium
 
    theTool.CutBinEta = [0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
        
    # range of ET bins for e-ID
    theTool.CutBinET = [5.0*GeV, 10.0*GeV, 15.0*GeV, 20.0*GeV, 30.0*GeV, 40.0*GeV, 50.0*GeV, 60.0*GeV, 70.0*GeV, 80.0*GeV]
    # cut on fraction of energy deposited in 1st sampling
    theTool.CutF1 = [0.005]
    # cut on hadronic energy


    theTool.CutHadLeakage  = [0.12800, 0.15600, 0.15200, 0.11600, 0.15600, 0.04275, 0.13200, 0.15200, 0.15600, 0.14000,# < 5
            0.12800, 0.15600, 0.15200, 0.11600, 0.15600, 0.04275, 0.13200, 0.15200, 0.15600, 0.14000, # 5-10
            0.03225, 0.03225, 0.03075, 0.03575, 0.02575, 0.04275, 0.04325, 0.04525, 0.04325, 0.03675,  # 10-15
            0.02925, 0.02925, 0.02775, 0.03175, 0.02375, 0.03875, 0.04025, 0.03425, 0.03825, 0.02975,  # 15-20
            0.02425, 0.02425, 0.02275, 0.02575, 0.01975, 0.01975, 0.02725, 0.02725, 0.02725, 0.01975,  # 20-30
            0.02275, 0.02275, 0.02125, 0.01975, 0.01825, 0.01825, 0.02425, 0.02575, 0.02425, 0.01675,  # 30-40
            0.01825, 0.01825, 0.01975, 0.01525, 0.01675, 0.01675, 0.02125, 0.02275, 0.01975, 0.01675,  # 40-50
            0.01825, 0.01825, 0.01975, 0.01525, 0.01675, 0.01675, 0.02125, 0.02275, 0.01975, 0.01675,  # 50-60
            0.01825, 0.01825, 0.01975, 0.01525, 0.01675, 0.01675, 0.02125, 0.02275, 0.01975, 0.01675,  # 60-70
            0.01825, 0.01825, 0.01975, 0.01525, 0.01675, 0.01675, 0.02125, 0.02275, 0.01975, 0.01675,  # 70-80
            0.01825, 0.01825, 0.01975, 0.01525, 0.01675, 0.01675, 0.02125, 0.02275, 0.01975, 0.01675 # >80
            ]


    # cut on ratio e237/e277
    theTool.CutReta37 = [0.6800, 0.5600, 0.6000, 0.6800, 0.7200, 0.440, 0.7600, 0.7200, 0.7600, 0.7475,  # < 5
            0.6800, 0.5600, 0.6000, 0.6800, 0.7200, 0.440, 0.7600, 0.7200, 0.7600, 0.7475,  # 5-10
            0.8475, 0.8475, 0.8425, 0.8175, 0.8475, 0.740, 0.8275, 0.8675, 0.8675, 0.7475, # 10-15
            0.8675, 0.8675, 0.8675, 0.8475, 0.8725, 0.740, 0.8525, 0.8775, 0.8775, 0.7575, # 15-20
            0.8825, 0.8825, 0.8825, 0.8575, 0.8875, 0.750, 0.8725, 0.9025, 0.8875, 0.7575, # 20-30
            0.9075, 0.9075, 0.8975, 0.8875, 0.8875, 0.790, 0.8925, 0.9075, 0.8975, 0.8075, # 30-40
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 40-50
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 50-60
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 60-70
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 70-80
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625 # >80          
            ]
            
            

    # cut on shower width in 2nd sampling
    theTool.CutWeta2c = [0.017, 0.016, 0.018, 0.016, 0.019, 0.031, 0.017, 0.016, 0.0155, 0.0155,# < 5
            0.017, 0.016, 0.018, 0.016, 0.019, 0.031, 0.017, 0.016, 0.0155, 0.0155, # 5-10
            0.013, 0.013, 0.013, 0.013, 0.013, 0.025, 0.014, 0.014, 0.0135, 0.014,    #10-15
            0.012, 0.012, 0.013, 0.013, 0.013, 0.025, 0.014, 0.014, 0.0135, 0.014,    #15-20
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 20-30
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 30-40
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 40-50
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 50-60
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 60-70
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135, # 70-80
            0.011, 0.011, 0.012, 0.012, 0.013, 0.025, 0.013, 0.0125, 0.013, 0.0135 # 80< 
            ]



    # cut on total width in 1st sampling
    theTool.CutWtot  =  [ 3.48, 3.48, 3.78, 3.96, 4.20, 9999., 4.02, 2.70, 1.86, 9999.,    # < 5    GeV
            3.18, 3.18, 3.54, 3.90, 4.02, 9999., 3.96, 2.70, 1.80, 9999.,    # 5-10   
            2.85, 2.85, 3.20, 3.40, 3.60, 9999., 3.70, 2.40, 1.72, 9999.,    # 10-15
            2.76, 2.76, 2.92, 3.30, 3.45, 9999., 3.67, 2.40, 1.72, 9999.,    # 15-20
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 20-30
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 30-40
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 40-50
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 50-60
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 60-70
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.,    # 70-80
            2.50, 2.50, 2.65, 3.00, 3.20, 9999., 3.30, 2.15, 1.49, 9999.     # >80
            ]

    # cut on (Emax - Emax2)/(Emax + Emax2) in 1st sampling
    theTool.CutDEmaxs1 = [0.640, 0.600, 0.560, 0.36, 0.24, -9999., 0.320, 0.640, 0.760, -9999., # < 5 
            0.640, 0.600, 0.560, 0.36, 0.24, -9999., 0.320, 0.640, 0.760, -9999., # 5-10
            0.8, 0.8, 0.79, 0.61, 0.6, -9999.0, 0.65, 0.86, 0.85, -9999.0,   #10-15
            0.83, 0.83, 0.825, 0.7, 0.65, -9999.0, 0.75, 0.9, 0.89, -9999.0, #15-20
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0, # 20-30
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0, # 30-40
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0, # 40-50
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0, # 50-60
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0,  # 60-70
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0, # 70-80
            0.835, 0.835, 0.835, 0.8, 0.8, -9999.0, 0.8, 0.9, 0.91, -9999.0  # 80<
            ]


    ### cutf3###
    #Cut on fraction of energy to use 3rd sampling
    theTool.CutF3 = [0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # < 5
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 5-10
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 10-15
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 15-20
            #0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 20-30
            #0.0255, 0.0255, 0.0195, 0.0215, 0.0245, 9999., 0.0205, 0.0285, 0.0268, 0.0383,  # 30-40
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,   # 20-30
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,   # 30-40
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,  # 40-50
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,  # 50-60
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,  # 60-70
            0.0265, 0.0265, 0.0195, 0.0215, 0.0245, 9999., 0.0255, 0.0285, 0.0293, 0.0383,  # 70-80
            9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999.            #  >80
            ]

                  
    # cut on Delta Emax2 in 1st sampling 
    theTool.CutDeltaEmax2  = []
            
    # cut on Emax2 - Emin in 1st sampling 
    theTool.CutDeltaE  = []
            
    # cut on width in 1st sampling
    theTool.CutWeta1c  = []
            
    # cut on Fside in 1st sampling
    theTool.CutFracm = []
            
    # cut on Energy in cone of 0.20
    # not used removed from tool
    #theTool.CutClusterIsolation += []
            
    # cut on Track quality cut
            
    # cut on pixel-layer hits
    theTool.CutPi  = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
            
    # cut on precision hits
    theTool.CutSi  = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
            
    # cut on transverse impact parameter
    theTool.CutA0  = [5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0]
            
    # cut on transverse impact parameter for tight selection   	 	 
    theTool.CutA0Tight  = [5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0]
            
         
    # cut on delta eta
    theTool.CutDeltaEta = [0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005
            ]
    # cut on delta eta for tight selection
    theTool.CutDeltaEtaTight = [0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005,
            0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005, 0.005
            ]

    theTool.useTRTOutliers  = True
    theTool.useBLOutliers  = True
    theTool.usePIXOutliers  = True
    theTool.useSCTOutliers  = True  
            
    # cut on b-layer hits
    theTool.CutBL  = [1, 1, 1, 1, 1, 1, 1, 1, 1, 0]

    # cut max on delta phi
    theTool.CutmaxDeltaPhi  = [ 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015
            ]
            
    # cut min on deltaphi
    theTool.CutminDeltaPhi  = [ -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
            -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04 
            ]
            
    # cut min on E/P
    theTool.CutminEp  = [0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
            0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
            0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90,
            0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90,
            0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
            0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
            0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
            0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
            0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
            0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70, 0.70,
            0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
            ]
    # cut max on E/P
    theTool.CutmaxEp  = [2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0, 3.0, 3.0,
            2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0, 3.0, 3.0,
            2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.0, 4.0,
            2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.0, 4.0,
            2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.5, 3.5, 4.5, 4.5,
            3.0, 3.0, 3.0, 3.0, 3.0, 3.5, 3.5, 4.0, 4.5, 4.5,
            3.0, 3.0, 3.0, 3.0, 3.0, 3.5, 4.0, 5.0, 5.0, 5.0,
            5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
            5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
            5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
            10., 10., 10., 10., 10., 10., 10., 10., 10., 10.
            ]

    # cuts on TRT
    # range of eta bins for e-ID for TRT
    theTool.CutBinEta_TRT = [0.1, 0.625, 1.07, 1.304, 1.752, 2.0]
    # cuts on Number of TRT hits with Outliers
    theTool.CutNumTRT  = [-15.,-15., -15., -15., -15., -15.]
    # cuts on TRT ratio with Outliers
    theTool.CutTRTRatio  = [0.05, 0.08, 0.075, 0.09, 0.105, 0.110]
    # cuts on TRT ratio with Outliers for 90% efficiency
    theTool.CutTRTRatio90  = [0.10, 0.10, 0.125, 0.13, 0.13, 0.13]
            
     
