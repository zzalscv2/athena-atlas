# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# default configuration of the ElectronIsEMSelectorCutDefs
# This one is used for loose++ menu

# Import a needed helper
from PATCore.HelperUtils import *

# Define GeV
GeV = 1000.0

def ElectronIsEMH4l2011SelectorConfig(theTool) :
    '''
    This is for the Loose++ isEM definitions.
    '''
    
    theTool = GetTool(theTool)

    # the eta ranges
    theTool.CutBinEta += [0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
    
    # range of ET bins for e-ID
    theTool.CutBinET += [5.0*GeV, 10.0*GeV, 15.0*GeV, 20.0*GeV, 30.0*GeV, 40.0*GeV, 50.0*GeV, 60.0*GeV, 70.0*GeV, 80.0*GeV]
    # cut on fraction of energy deposited in 1st sampling
    theTool.CutF1 += [0.005]
    # cut on hadronic energy
    theTool.CutHadLeakage  += [ 0.031, 0.031, 0.021, 0.021, 0.019, 0.028, 0.065, 0.065, 0.046, 0.034, # < 5
                               0.018, 0.018, 0.016, 0.015, 0.016, 0.028, 0.053, 0.038, 0.028, 0.025, # 5-10
                               0.018, 0.018, 0.018, 0.020, 0.016, 0.033, 0.036, 0.033, 0.024, 0.025, # 10-15 
                               0.015, 0.015, 0.015, 0.016, 0.014, 0.029, 0.033, 0.022, 0.019, 0.018, # 15-20 
                               0.012, 0.012, 0.012, 0.012, 0.012, 0.015, 0.030, 0.022, 0.016, 0.016, # 20-30 
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.021, 0.021, 0.015, 0.015, # 30-40 
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.015, 0.015, 0.010, 0.010, # 40-50
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.015, 0.015, 0.010, 0.010, # 50-60
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.015, 0.015, 0.010, 0.010, # 60-70
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.015, 0.015, 0.010, 0.010, # 70-80
                               0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.015, 0.015, 0.010, 0.010, # >80
                               ]


    # cut on ratio e237/e277
    theTool.CutReta37  += [ 0.700, 0.700, 0.700, 0.700, 0.700, 0.690, 0.848, 0.876, 0.870, 0.888, # < 5
                           0.700, 0.700, 0.700, 0.700, 0.700, 0.715, 0.860, 0.880, 0.880, 0.880, # 5-10
                           0.875, 0.875, 0.875, 0.875, 0.875, 0.740, 0.860, 0.875, 0.870, 0.870, # 10-15
                           0.900, 0.900, 0.895, 0.895, 0.890, 0.740, 0.880, 0.900, 0.880, 0.880, # 15-20
                           0.910, 0.910, 0.910, 0.910, 0.910, 0.750, 0.890, 0.900, 0.890, 0.890, # 20-30
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # 30-40
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # 40-50 
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # 50-60 
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # 60-70 
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # 70-80 
                           0.920, 0.920, 0.920, 0.915, 0.915, 0.790, 0.895, 0.915, 0.895, 0.890, # >80 
                           ]
    
    # cut on shower width in 2nd sampling
    theTool.CutWeta2c  +=  [0.014, 0.014, 0.014, 0.014, 0.014, 0.028, 0.017, 0.014, 0.014, 0.014,# <5 
                            0.014, 0.014, 0.014, 0.014, 0.014, 0.026, 0.017, 0.014, 0.014, 0.014,# 5-10
                            0.014, 0.014 ,0.015 ,0.016 ,0.017 ,0.025 ,0.017 ,0.015 ,0.015 ,0.015,#10-15
                            0.013, 0.013 ,0.015 ,0.016 ,0.017 ,0.025 ,0.017 ,0.015 ,0.015 ,0.014,#15-20
                            0.013 ,0.013 ,0.014 ,0.015 ,0.015 ,0.025 ,0.016 ,0.015 ,0.015 ,0.014,#20-30
                            0.012 ,0.012 ,0.013 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013,#30-40
                            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013,#40-50
                            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013,# 50-60
                            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013,# 60-70 
                            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013,# 70-80 
                            0.011 ,0.011 ,0.012 ,0.013 ,0.013 ,0.025 ,0.015 ,0.014 ,0.014 ,0.013]# 80<   

    
    # cut on total width in 1st sampling
    theTool.CutWtot  += [ 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999.,  # < 5 
                         9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999., 9999.,  # 5-10
                         3.20, 3.20, 3.20, 3.85, 3.85, 9999., 3.80, 3.00, 2.00, 9999.,  # 10-15 
                         3.00, 3.00, 3.00, 3.75, 3.75, 9999., 3.80, 3.00, 2.00, 9999.,  # 15-20 
                         2.90, 2.90, 2.90, 3.50, 3.50, 9999., 3.80, 3.00, 2.00, 9999.,  # 20-30 
                         2.80, 2.80, 2.80, 3.30, 3.40, 9999., 3.70, 3.00, 1.70, 9999.,  # 30-40 
                         2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 40-50 
                         2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 50-60 
                         2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 60-70 
                         2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # 70-80 
                         2.80, 2.80, 2.80, 3.20, 3.40, 9999., 3.70, 2.90, 1.60, 9999.,  # >80 
                         ]
    
    # cut on (Emax - Emax2)/(Emax + Emax2) in 1st sampling
    theTool.CutDEmaxs1 += [ 0.390 ,0.390 ,0.200, 0.070 ,0.060, -9999,  0.070, 0.430, 0.750, -9999,  # < 5 
                           0.650 ,0.660 ,0.560 ,0.460 ,0.530 ,-9999,  0.600, 0.680, 0.750, -9999,  # 5-10
                           0.790, 0.790, 0.750, 0.590, 0.530, -9999., 0.600, 0.790, 0.840, -9999.,  # 10-15 
                           0.790, 0.790, 0.790, 0.700, 0.580, -9999., 0.600, 0.790, 0.850, -9999.,  # 15-20 
                           0.800, 0.800, 0.820, 0.720, 0.650, -9999., 0.780, 0.790, 0.850, -9999.,  # 20-30 
                           0.800, 0.800, 0.825, 0.720, 0.690, -9999., 0.780, 0.810, 0.880, -9999.,  # 30-40 
                           0.800, 0.800, 0.825, 0.730, 0.690, -9999., 0.790, 0.810, 0.880, -9999.,  # 40-50 
                           0.800, 0.800, 0.825, 0.730, 0.690, -9999., 0.790, 0.810, 0.880, -9999.,  # 50-60 
                           0.800, 0.800, 0.825, 0.730, 0.690, -9999., 0.790, 0.810, 0.880, -9999.,  # 60-70 
                           0.800, 0.800, 0.825, 0.730, 0.690, -9999., 0.790, 0.810, 0.880, -9999.,  # 70-80 
                           0.800, 0.800, 0.825, 0.730, 0.690, -9999., 0.790, 0.810, 0.880, -9999.,  # >80 
                           ]
    
    # cut on Track quality cut        
    theTool.usePIXOutliers  = True
    theTool.useSCTOutliers  = True  


    # cut on pixel-layer hits
    theTool.CutPi  += [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        
    # cut on precision hits
    theTool.CutSi  += [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
        
    # cut on delta eta
    theTool.CutDeltaEta += [0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
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

###################### END OF USEFULL CUTS ##########################################


    # The following ARE NOT APPLIED FOR THE H4L or the LOOSE MENU ISEM

    # cut on transverse impact parameter
    theTool.CutA0  += [5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0]
    
    # cut on transverse impact parameter for tight selection   	 	 
    theTool.CutA0Tight  += [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

    # cut on delta eta for tight selection
    theTool.CutDeltaEtaTight += [0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
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
    theTool.useTRTOutliers  = True
    theTool.useBLOutliers  = True

        
    # cut on b-layer hits
    theTool.CutBL  += [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
    # cut max on delta phi
    theTool.CutmaxDeltaPhi  += [ 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015,
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
    theTool.CutminDeltaPhi  += [ -0.03, -0.03, -0.03, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,
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
    theTool.CutminEp  += [0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80,
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
    theTool.CutmaxEp  += [2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0, 3.0, 3.0,
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
    theTool.CutBinEta_TRT += [0.1, 0.625, 1.07, 1.304, 1.752, 2.0]
    # cuts on Number of TRT hits with Outliers
    theTool.CutNumTRT  += [-15.,-15., -15., -15., -15., -15.]
    # cuts on TRT ratio with Outliers
    theTool.CutTRTRatio  += [0.04, 0.04, 0.04, 0.05, 0.07, 0.07]
    # cuts on TRT ratio with Outliers for 90% efficiency
    theTool.CutTRTRatio90  += [0.10, 0.10, 0.125, 0.13, 0.13, 0.13]

###################### END OF CUTS ##########################################

