# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration                                                       

from TrigMonitorBase.TrigGenericMonitoringToolConfig import defineHistogram, TrigGenericMonitoringToolConfig

class TrigMuonEFTagandProbeMonitoring(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="TrigMuonEFTagandProbeMonitoring"):
        super(TrigMuonEFTagandProbeMonitoring, self).__init__(name)
        self.defineTarget( ["Online", "Validation"] ) 

#Profile Histograms for producing efficiency plots online 
#Ordered by Threshold/(Barrel,Endcap)/(Eta,Phi,Pt) - Threshold 1----------------------------------------

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr1_b, EF_Etaphi_Eff_thr1_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 1 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr1_b, EF_Etaphi_Eff_thr1_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 1 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr1_b, EF_Pt_Eff_thr1_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 1 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr1_e, EF_Etaphi_Eff_thr1_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 1 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr1_e, EF_Etaphi_Eff_thr1_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 1 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr1_e, EF_Pt_Eff_thr1_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 1 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]



#Threshold 2 ---------------------------------------------------------------------------------------------

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr2_b, EF_Etaphi_Eff_thr2_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 2 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr2_b, EF_Etaphi_Eff_thr2_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 2 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr2_b, EF_Pt_Eff_thr2_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 2 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr2_e, EF_Etaphi_Eff_thr2_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 2 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr2_e, EF_Etaphi_Eff_thr2_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 2 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr2_e, EF_Pt_Eff_thr2_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 2 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]



#Threshold 3 ---------------------------------------------------------------------------------------------

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr3_b, EF_Etaphi_Eff_thr3_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 3 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr3_b, EF_Etaphi_Eff_thr3_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 3 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr3_b, EF_Pt_Eff_thr3_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 3 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr3_e, EF_Etaphi_Eff_thr3_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 3 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr3_e, EF_Etaphi_Eff_thr3_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 3 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr3_e, EF_Pt_Eff_thr3_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 3 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]



#Threshold 4 ---------------------------------------------------------------------------------------------

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr4_b, EF_Etaphi_Eff_thr4_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 4 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr4_b, EF_Etaphi_Eff_thr4_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 4 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr4_b, EF_Pt_Eff_thr4_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 4 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr4_e, EF_Etaphi_Eff_thr4_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 4 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr4_e, EF_Etaphi_Eff_thr4_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 4 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr4_e, EF_Pt_Eff_thr4_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 4 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]



#Threshold 5 ---------------------------------------------------------------------------------------------


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr5_b, EF_Etaphi_Eff_thr5_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 5 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr5_b, EF_Etaphi_Eff_thr5_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 5 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr5_b, EF_Pt_Eff_thr5_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 5 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr5_e, EF_Etaphi_Eff_thr5_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 5 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr5_e, EF_Etaphi_Eff_thr5_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 5 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr5_e, EF_Pt_Eff_thr5_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 5 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]



#Threshold 6 ---------------------------------------------------------------------------------------------


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr6_b, EF_Etaphi_Eff_thr6_b', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 6 Barrel;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr6_b, EF_Etaphi_Eff_thr6_b', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 6 Barrel;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr6_b, EF_Pt_Eff_thr6_b', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 6 Barrel;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]


        self.Histograms += [ defineHistogram('EF_Eta_Total_thr6_e, EF_Etaphi_Eff_thr6_e', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 6 Endcap;Muon #eta ;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Phi_Total_thr6_e, EF_Etaphi_Eff_thr6_e', type='TProfile', title="L1 Trigger Efficiency by #phi in threshold 6 Endcap;Muon #phi [Rad];Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Pt_Total_thr6_e, EF_Pt_Eff_thr6_e', type='TProfile', title="L1 Trigger Efficiency by P_{T} in threshold 6 Endcap;Muon P_{T} [GeV];Efficiency",
                                             xbins=20, xmin=10, xmax=110) ]





#Additional Profile histograms for combined barrel and endcap eta plots - ordered by threshold

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr1, EF_Eta_Eff_thr1', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 1;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr2, EF_Eta_Eff_thr2', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 2;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr3, EF_Eta_Eff_thr3', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 3;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr4, EF_Eta_Eff_thr4', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 4;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr5, EF_Eta_Eff_thr5', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 5;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr6, EF_Eta_Eff_thr6', type='TProfile', title="L1 Trigger Efficiency by #eta in threshold 6;Muon #eta;Efficiency",
                                             xbins=35, xmin=-3.5, xmax=3.5) ]





#2D Profile Histograms for producing efficiency plots of eta v phi online 
#Ordered by Threshold - Threshold 1----------------------------------------

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr1_b, EF_Phi_Total_thr1_b, EF_Etaphi_Eff_thr1_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 1 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr1_e, EF_Phi_Total_thr1_e, EF_Etaphi_Eff_thr1_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 1 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]


#Threshold 2 ----------------------------------------------------------------------------------------------
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr2_b, EF_Phi_Total_thr2_b, EF_Etaphi_Eff_thr2_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 2 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr2_e, EF_Phi_Total_thr2_e, EF_Etaphi_Eff_thr2_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 2 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]


#Threshold 3 ----------------------------------------------------------------------------------------------
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr3_b, EF_Phi_Total_thr3_b, EF_Etaphi_Eff_thr3_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 3 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr3_e, EF_Phi_Total_thr3_e, EF_Etaphi_Eff_thr3_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 3 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]


#Threshold 4 ----------------------------------------------------------------------------------------------
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr4_b, EF_Phi_Total_thr4_b, EF_Etaphi_Eff_thr4_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 4 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr4_e, EF_Phi_Total_thr4_e, EF_Etaphi_Eff_thr4_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 4 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]


#Threshold 5 ----------------------------------------------------------------------------------------------
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr5_b, EF_Phi_Total_thr5_b, EF_Etaphi_Eff_thr5_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 5 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr5_e, EF_Phi_Total_thr5_e, EF_Etaphi_Eff_thr5_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 5 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]


#Threshold 6 ----------------------------------------------------------------------------------------------
        self.Histograms += [ defineHistogram('EF_Eta_Total_thr6_b, EF_Phi_Total_thr6_b, EF_Etaphi_Eff_thr6_b', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 6 Barrel;Muon #eta ;Muon #phi",
                                             xbins=11, xmin=-1.1, xmax=1.1, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

        self.Histograms += [ defineHistogram('EF_Eta_Total_thr6_e, EF_Phi_Total_thr6_e, EF_Etaphi_Eff_thr6_e', type='TProfile2D', title="L1 Trigger Efficiency in #eta and #phi Threshold 6 Endcap;Muon #eta ;Muon #phi",
                                             xbins=27, xmin=-2.7, xmax=2.7, ybins=8, ymin=-3.4, ymax=3.4, zmin=-0.1,zmax=1.1) ]

