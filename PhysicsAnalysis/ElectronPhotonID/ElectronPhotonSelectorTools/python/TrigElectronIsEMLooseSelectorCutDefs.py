# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

##===============================================================================
## Name:        TrigEgammaElectronIsEMCutDefs_loose.py
##
## Author:      Ryan Mackenzie White
## Created:     June 2014
##
## Description: Loose trigger electron cut definitions for 2014 new tunes. 
## Optimized by S. Kahn            
##===============================================================================

import cppyy
try :
    cppyy.loadDictionary('ElectronPhotonSelectorToolsDict')
except :
    pass

from ROOT import egammaPID

# Import a needed helper
from PATCore.HelperUtils import *

# Define GeV
GeV = 1000.0

def TrigElectronIsEMLooseSelectorConfigDC14(theTool) :
    '''
    This is for the Loose isEM definitions for the Trigger.
    '''
 
    theTool = GetTool(theTool)
    
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150329/ElectronIsEMLooseSelectorCutDefs.conf"


def TrigElectronIsEMLooseSelectorConfigDC14OLD(theTool) :
    '''
    This is for the Loose isEM definitions for the Trigger.
    '''

    theTool = GetTool(theTool)

    # range of eta bins for e-ID
    theTool.CutBinEta = [0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]

    # range of ET bins for e-ID
    theTool.CutBinET = [5.0*GeV, 10.0*GeV, 15.0*GeV, 20.0*GeV, 30.0*GeV, 40.0*GeV, 50.0*GeV, 60.0*GeV, 70.0*GeV, 80.0*GeV]
    # cut on fraction of energy deposited in 1st sampling
    theTool.CutF1 = [0.005]
    # cut on hadronic energy

    #CM 16-March-2011: update 
    theTool.CutHadLeakage  = [0.12800, 0.15600, 0.15200, 0.11600, 0.15600, 0.04275, 0.13200, 0.15200, 0.15600, 0.14000,# < 5
            0.12800, 0.15600, 0.15200, 0.11600, 0.15600, 0.04275, 0.13200, 0.15200, 0.15600, 0.14000, # 5-10
            0.03225, 0.03225, 0.03075, 0.03575, 0.02575, 0.04275, 0.04325, 0.04525, 0.04325, 0.03675,  # 10-15
            0.02925, 0.02925, 0.02775, 0.03175, 0.02375, 0.03875, 0.04025, 0.03425, 0.03825, 0.02975,  # 15-20
            0.03825, 0.03825, 0.03675, 0.03975, 0.03375, 0.01975, 0.04125, 0.04125, 0.04125, 0.03375,  # 20-30 Optimized by S. Kahn Aug. 2014
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
            0.8623, 0.8623, 0.8623, 0.8373, 0.8673, 0.750, 0.8523, 0.8823, 0.8724, 0.7424, # 20-30 Optimized by S. Kahn Aug. 2014
            0.9075, 0.9075, 0.8975, 0.8875, 0.8875, 0.790, 0.8925, 0.9075, 0.8975, 0.8075, # 30-40
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 40-50
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 50-60
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 60-70
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625, # 70-80
            0.9175, 0.9175, 0.9125, 0.9075, 0.9025, 0.790, 0.8975, 0.9125, 0.9025, 0.8625 # >80          
            ]

    # cut on ratio e233/e237
    theTool.CutRphi37 = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # < 5
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 5 - 10
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 10 - 15
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 10 - 15
            0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.75, 0.75, 0.75, # 20-30 Optimized by S. Kahn Aug. 2014
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 30 - 40
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 40 - 50
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 50 - 60
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 60 - 70
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # 70 - 80
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0  # > 80
            ]

    # cut on shower width in 2nd sampling
    theTool.CutWeta2c = [0.017, 0.016, 0.018, 0.016, 0.019, 0.031, 0.017, 0.016, 0.0155, 0.0155,   # < 5
            0.017, 0.016, 0.018, 0.016, 0.019, 0.031, 0.017, 0.016, 0.0155, 0.0155, # 5-10
            0.014 ,0.014 ,0.015 ,0.016 ,0.017 ,0.025 ,0.017 ,0.015 ,0.015 ,0.0145, #10-15
            0.0135 ,0.0135 ,0.0145 ,0.016 ,0.017 ,0.025 ,0.017 ,0.015 ,0.015 ,0.0145, #15-20
            0.0121, 0.0121, 0.0131, 0.0131, 0.0141, 0.025, 0.0141, 0.0136, 0.0137, 0.0141, #20-30 Optimized by S. Kahn Aug. 2014
            0.012 ,0.012 ,0.013 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135, #30-40
            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135, #40-50
            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135,# 50-60
            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135,# 60-70
            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135,# 70-80
            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.0135 # 80< 
            ]


    # cut on total width in 1st sampling
    theTool.CutWtot  = [ 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999.,  # < 5 
            9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999.,  # 5-10
            3.20, 3.20, 3.20, 3.85, 3.85, 9999., 3.80, 3.00, 2.00, 9999.,  # 10-15 
            3.00, 3.00, 3.00, 3.75, 3.75, 9999., 3.80, 3.00, 2.00, 9999.,  # 15-20 
            3.25, 3.25, 3.40, 3.75, 3.95, 9999., 4.05, 2.90, 2.20, 9999.,  # 20-30 Optimized by S. Kahn Aug. 2014
            2.80, 2.80, 2.80, 3.30, 3.40, 9999., 3.70, 3.00, 1.70, 9999.,  # 30-40 
            2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 40-50 
            2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 50-60 
            2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 60-70 
            2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 70-80 
            2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.   # >80 
            ]

    # cut on (Emax - Emax2)/(Emax + Emax2) in 1st sampling
    theTool.CutDEmaxs1  = [0.640, 0.600, 0.560, 0.36, 0.24, -9999., 0.320, 0.640, 0.760, -9999.  , # < 5 
            0.640, 0.600, 0.560, 0.36, 0.24, -9999., 0.320, 0.640, 0.760, -9999.  , # 5-10 
            0.790 ,0.790 ,0.750 ,0.590 ,0.530, -9999 ,0.600 ,0.790 ,0.840, -9999.,  #10-15
            0.790 ,0.790 ,0.790 ,0.600 ,0.550, -9999 ,0.600 ,0.790 ,0.850, -9999.,  #15-20
            0.785 ,0.785 ,0.785 ,0.750 ,0.750, -9999 ,0.750 , 0.85 ,0.875, -9999 ,  #20-30 Optimized by S. Kahn Aug. 2014
            0.800 ,0.800 ,0.825 ,0.720 ,0.690, -9999 ,0.780 ,0.810 ,0.880, -9999 ,  #30-40
            0.800 ,0.800 ,0.825 ,0.730 ,0.690, -9999 ,0.790 ,0.810 ,0.880, -9999 ,  #40-50
            0.800 ,0.800 ,0.825 ,0.730 ,0.690 ,-9999. ,0.790 ,0.810 ,0.880 ,-9999., # 50-60 
            0.800 ,0.800 ,0.825 ,0.730 ,0.690 ,-9999. ,0.790 ,0.810 ,0.880 ,-9999., # 60-70
            0.800 ,0.800 ,0.825 ,0.730 ,0.690 ,-9999. ,0.790 ,0.810 ,0.880 ,-9999., # 70-80
            0.800 ,0.800 ,0.825 ,0.730 ,0.690 ,-9999. ,0.790 ,0.810 ,0.880 ,-9999.  # 80< 
            ] 
                                   
    ### cutf3###
    #Cut on fraction of energy to use 3rd sampling
    theTool.CutF3 = [0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # < 5
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 5-10
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 10-15
            0.0215, 0.0215, 0.0175, 0.0175, 0.0235, 9999., 0.0175, 0.0285, 0.0268, 0.0383,  # 15-20
            0.0260, 0.0260, 0.0260, 0.0260, 0.0260, 9999., 0.0260, 0.0500, 0.0500, 0.0500,  # 20-30 Optimized by S. Kahn Aug. 2014
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
    # theTool.CutClusterIsolation = []

    # cut on Track quality cut

    # cut on pixel-layer hits
    theTool.CutPi  = [2, 2, 2, 2, 2, 2, 2, 2, 2, 2] #Optimized by S. Kahn Aug. 2014

    # cut on precision hits
    theTool.CutSi  = [8, 8, 8, 8, 8, 8, 8, 8, 8, 8] #Optimized by S. Kahn Aug. 2014

    # cut on transverse impact parameter
    theTool.CutA0  = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0] #Optimized by S. Kahn Aug. 2014

    # cut on transverse impact parameter for tight selection   	 	 
    theTool.CutA0Tight  = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

    # cut on delta eta
    theTool.CutDeltaEta = [0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025,#20-30 Optimized by S. Kahn Aug. 2014
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
            0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015
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
    theTool.CutBL  = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]

    # cut on min delta phi
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
    theTool.CutTRTRatio  = [0.05, 0.08, 0.075, 0.09, 0.105, 0.11] #20-30 Optimized by S. Kahn Aug. 2014
    # cuts on TRT ratio with Outliers for 90% efficiency
    theTool.CutTRTRatio90  = [0.10, 0.10, 0.125, 0.13, 0.13, 0.13]
     
