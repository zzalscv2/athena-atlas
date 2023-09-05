#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigMETMonitoringAlgorithm.py
@author K. Hamano
@author G. Gallardo
@date 2019-05-13
@brief MET Trigger python configuration for Run 3 AthenaMonitoring package

'''

def TrigMETMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigMETAthMonitorCfg')

    ### STEP 2 ###
    # Adding an algorithm to the helper. Here, we will use the example
    # algorithm in the AthenaMonitoring package. Just pass the type to the
    # helper. Then, the helper will instantiate an instance and set up the
    # base class configuration following the inputFlags. The returned object
    # is the algorithm.
    #The added algorithm must exist as a .h file

    from AthenaConfiguration.ComponentFactory import CompFactory
    TrigMETMonAlg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonAlg')

    # You can actually make multiple instances of the same algorithm and give
    # them different configurations
    TrigMETMonChain1Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain1Alg')
    TrigMETMonChain2Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain2Alg')
    TrigMETMonChain3Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain3Alg')
    TrigMETMonChain4Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain4Alg')
    TrigMETMonChain5Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain5Alg')
    TrigMETMonChain6Alg = helper.addAlgorithm(CompFactory.TrigMETMonitorAlgorithm,'TrigMETMonChain6Alg')

    # # If for some really obscure reason you need to instantiate an algorithm
    # # yourself, the AddAlgorithm method will still configure the base
    # # properties and add the algorithm to the monitoring sequence.
    # helper.AddAlgorithm(myExistingAlg)


    ### check Run2 or Run3 MT
    mt_chains = True
    if ( inputFlags.Trigger.EDMVersion < 3 ) :
      mt_chains = False


    ### STEP 3 ###
    # Edit properties of a algorithm
    # some generic property
    # expertTrigMETMonAlg.RandomHist = True
    # to enable a trigger filter, for example:
    # TrigMETMonAlg.TriggerChain = 'HLT_xe30_cell_L1XE10'
    # without filters, all events are processed.
    TrigMETMonChain1 = 'HLT_xe80_cell_xe115_tcpufit_L1XE50'
    TrigMETMonChain2 = 'HLT_xe65_cell_xe90_pfopufit_L1XE50'
    TrigMETMonChain3 = 'HLT_xe65_cell_xe105_nn_L1XE50'
    TrigMETMonChain4 = 'HLT_xe80_cell_xe115_tcpufit_L1XE55'
    TrigMETMonChain5 = 'HLT_xe65_cell_xe90_pfopufit_L1XE55'
    TrigMETMonChain6 = 'HLT_xe65_cell_xe105_nn_L1XE55'
    if mt_chains:
      TrigMETMonChain1Alg.TriggerChain = TrigMETMonChain1
      TrigMETMonChain2Alg.TriggerChain = TrigMETMonChain2
      TrigMETMonChain3Alg.TriggerChain = TrigMETMonChain3
      TrigMETMonChain4Alg.TriggerChain = TrigMETMonChain4
      TrigMETMonChain5Alg.TriggerChain = TrigMETMonChain5
      TrigMETMonChain6Alg.TriggerChain = TrigMETMonChain6
    else:
      TrigMETMonChain1Alg.TriggerChain = 'HLT_xe110_pufit_xe65_L1XE50'
      TrigMETMonChain2Alg.TriggerChain = 'HLT_xe110_pufit_xe65_L1XE50'
      TrigMETMonChain3Alg.TriggerChain = 'HLT_xe110_pufit_xe65_L1XE50'


    ### monitorig group
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess=getHLTMonitoringAccess(inputFlags)
    metChains=moniAccess.monitoredChains(signatures="metMon",monLevels=["shifter"])
    metChainsVal=moniAccess.monitoredChains(signatures="metMon",monLevels=["val"])
    metChainsT0=moniAccess.monitoredChains(signatures="metMon",monLevels=["t0"])

    ### container name selection
    if mt_chains: # these are temporary, needs to be changed
      TrigMETMonAlg.hlt_electron_key = 'HLT_egamma_Electrons'
      TrigMETMonAlg.hlt_muon_key = 'HLT_MuonsCB_RoI'
      TrigMETMonAlg.offline_met_key = 'MET_Reference_AntiKt4EMTopo' #this used to be 'MET_EMTopo'
      TrigMETMonAlg.hlt_pfsum_key = 'HLT_MET_pfsum'
      TrigMETMonAlg.l1_jFexMet_key = 'L1_jFexMETRoI'
      TrigMETMonAlg.l1_jFexSumEt_key = 'L1_jFexSumETRoI'
      TrigMETMonAlg.l1_gFexJwojScalar_key = 'L1_gScalarEJwoj'
      TrigMETMonAlg.l1_gFexJwojMETComponents_key = 'L1_gMETComponentsJwoj'
      TrigMETMonAlg.l1_gFexJwojMHTComponents_key = 'L1_gMHTComponentsJwoj'
      TrigMETMonAlg.l1_gFexJwojMSTComponents_key = 'L1_gMSTComponentsJwoj'
      TrigMETMonAlg.l1_gFexNCMETScalar_key = 'L1_gScalarENoiseCut'
      TrigMETMonAlg.l1_gFexNCMETComponents_key = 'L1_gMETComponentsNoiseCut'
      TrigMETMonAlg.l1_gFexRhoMETScalar_key = 'L1_gScalarERms'
      TrigMETMonAlg.l1_gFexRhoMETComponents_key = 'L1_gMETComponentsRms'
    else:
      TrigMETMonAlg.hlt_electron_key = 'HLT_xAOD__ElectronContainer_egamma_Electrons'
      TrigMETMonAlg.hlt_muon_key = 'HLT_xAOD__MuonContainer_MuonEFInfo'
      TrigMETMonAlg.offline_met_key = 'MET_Reference_AntiKt4LCTopo'
      TrigMETMonAlg.hlt_cell_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET'
      TrigMETMonAlg.hlt_mht_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht'
      TrigMETMonAlg.hlt_tc_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_tc_em_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_tcpufit_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PUC'
      TrigMETMonAlg.hlt_trkmht_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_pfsum_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_pfsum_vssk_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_pfsum_cssk_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_pfopufit_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_cvfpufit_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_mhtpufit_pf_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'
      TrigMETMonAlg.hlt_mhtpufit_em_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl'

    ### chain name selection
    L1Chains = ["L1_XE50",
                "L1_XE55",
                "L1_jXE100",
                "L1_gXENC100",
                "L1_gXERHO100",
                "L1_gXEJWOJ100"]
    HLTChains = []
    HLTChainsVal = []
    HLTChainsT0 = []
    if mt_chains == 0:
      L1Chains = ["L1_XE50"]
      HLTChains = ["HLT_xe70_mht",
                   "HLT_xe90_mht_L1XE50",
                   "HLT_xe100_mht_L1XE50",
                   "HLT_xe110_mht_L1XE50",
                   "HLT_xe90_pufit_L1XE50",
                   "HLT_xe100_pufit_L1XE50",
                   "HLT_xe100_pufit_L1XE55",
                   "HLT_xe110_pufit_L1XE50",
                   "HLT_xe110_pufit_L1XE55",
                   "HLT_xe110_pufit_xe65_L1XE50",
                   "HLT_xe110_pufit_xe65_L1XE60",
                   "HLT_xe110_pufit_xe70_L1XE50"]
    ## set chains from mon group
    if len(metChains) > 0:
        HLTChains = metChains
    if len(metChainsVal) > 0:
        HLTChainsVal = metChainsVal
    if len(metChainsT0) > 0:
        HLTChainsT0 = metChainsT0
    
    ## Setting primary electron and muon chains for signalEl/signalMu/Expert plot selection
    HLTChainEl = ["HLT_e26_lhtight_ivarloose_L1.*","HLT_e28_lhtight_ivarloose_L1.*"]
    HLTChainMu = ["HLT_mu24_ivarmedium_L1.*"]
    ## pass chains to TrigMETMonAlg
    TrigMETMonAlg.L1Chains = L1Chains
    TrigMETMonAlg.HLTChains = HLTChains
    TrigMETMonAlg.HLTChainsVal = HLTChainsVal
    TrigMETMonAlg.HLTChainsT0 = HLTChainsT0
    TrigMETMonAlg.HLTChainEl = HLTChainEl
    TrigMETMonAlg.HLTChainMu = HLTChainMu

    ### algorithm selection
    algsL1 = ["roi"] 
    algsL1Fex = ["jFex",
                 "gFexJwoj",
                 "gFexNC",
                 "gFexRho"]
    algsHLT = ["cell", 
               "tcpufit", 
               "pfopufit",
               "met_nn"]
    algsHLTChain1 = ["cell",
                     "tcpufit"]
    algsHLTChain2 = ["cell",
                     "pfopufit"]
    algsHLTChain3 = ["cell",
                     "met_nn"]
    algsHLTChain4 = ["cell",
                     "tcpufit"]
    algsHLTChain5 = ["cell",
                     "pfopufit"]
    algsHLTChain6 = ["cell",
                     "met_nn"]
    algsHLTPreSel = ["cell", 
               "tcpufit", 
               "tcpufit_sig30", 
               "pfsum_cssk", 
               "pfsum_vssk", 
               "pfopufit", 
               "pfopufit_sig30", 
               "mhtpufit_pf", 
               "mhtpufit_em",
               "met_nn"] 
    algsHLT2d = ["cell", 
                 "tcpufit", 
                 "pfopufit"]
    algsHLTExpert = ["mht",
                     "tc_em",
                     "cvfpufit",
                     "pfsum",
                     "trkmht",
                     "mhtpufit_pf", 
                     "mhtpufit_em",
                     "tcpufit_sig30", 
                     "pfopufit_sig30", 
                     "pfsum_cssk", 
                     "pfsum_vssk"]
    algsMET2d_tcpufit = ["pfopufit", 
                         "pfsum_cssk",
                         "mhtpufit_pf"]

    ## pass algorithmss to TrigMETMonAlg
    TrigMETMonAlg.algsL1 = algsL1
    TrigMETMonAlg.algsHLT = algsHLT
    TrigMETMonAlg.algsHLTPreSel = algsHLTPreSel
    TrigMETMonAlg.algsHLT2d = algsHLT2d
    TrigMETMonAlg.algsHLTExpert = algsHLTExpert
    TrigMETMonAlg.algsMET2d_tcpufit = algsMET2d_tcpufit
    TrigMETMonChain1Alg.algsHLT = algsHLTChain1
    TrigMETMonChain2Alg.algsHLT = algsHLTChain2
    TrigMETMonChain3Alg.algsHLT = algsHLTChain3
    TrigMETMonChain4Alg.algsHLT = algsHLTChain4
    TrigMETMonChain5Alg.algsHLT = algsHLTChain5
    TrigMETMonChain6Alg.algsHLT = algsHLTChain6

    ### cell component and status bit
    comp_names = ["PreSamplB", "EMB1", "EMB2", "EMB3", # LAr barrel
                  "PreSamplE", "EME1", "EME2", "EME3", # LAr EM endcap
                  "HEC0",      "HEC1", "HEC2", "HEC3", # Hadronic end cap cal.
                  "TileBar0", "TileBar1", "TileBar2",  # Tile barrel
                  "TileGap1", "TileGap2", "TileGap3",  # Tile gap (ITC & scint)
                  "TileExt0", "TileExt1", "TileExt2",  # Tile extended barrel
                  "FCalEM",   "FCalHad2", "FCalHad3"]  # Forward cal endcap
                  #"Muons" ]                            # Muons
    bit_names = [
             "Processing",         # bit  0
             "ErrBSconv",          # bit  1
             "ErrMuon",            # bit  2
             "ErrFEB",             # bit  3
             "Skipped",            # bit  4
             "CompBigMEtSEtRatio", # bit  5
             "BadCompEnergy",      # bit  6
             "BadEnergyRatio",     # bit  7
             "spare",              # bit  8
             "BadCellQuality",     # bit  9
             "BadCellEnergy",      # bit 10
             "BadCellTime",        # bit 11
             "NoMuonTrack",        # bit 12
             "spare",              # bit 13
             "Processed",          # bit 14
             "CompError",          # bit 15
             "EMB_A_Missing",      # bit 16
             "EMB_C_Missing",      # bit 17
             "EME_A_Missing",      # bit 18
             "EME_C_Missing",      # bit 19
             "HEC_A_Missing",      # bit 20
             "HEC_C_Missing",      # bit 21
             "FCAL_A_Missing",     # bit 22
             "FCAL_C_Missing",     # bit 23
             "TileB_A_Missing",    # bit 24
             "TileB_C_Missing",    # bit 25
             "TileE_A_Missing",    # bit 26
             "TileE_C_Missing",    # bit 27
             "BadEMfraction",      # bit 28
             "GlobBigMEtSEtRatio", # bit 29
             "ObjInCrack",         # bit 30
             "GlobError"           # bit 31
             ]
    ## pass algorithmss to TrigMETMonAlg
    TrigMETMonAlg.compNames = comp_names
    TrigMETMonAlg.bitNames = bit_names

    electronPtCut = 30.0
    muonPtCut = 30.0
    electronEtaCut = 2.5
    muonEtaCut = 2.5
    LArNoiseBurstVetoAlgs = ["pfopufit",
                     "cell"]
    signalLepAlgs = ["pfopufit",
                     "cell",
                     "tcpufit"]
   
    TrigMETMonAlg.electronPtCut = electronPtCut
    TrigMETMonAlg.electronEtaCut = electronEtaCut
    TrigMETMonAlg.muonPtCut = muonPtCut
    TrigMETMonAlg.muonEtaCut = muonEtaCut
    TrigMETMonAlg.signalLepAlgs = signalLepAlgs
    TrigMETMonAlg.LArNoiseBurstVetoAlgs = LArNoiseBurstVetoAlgs

    ### STEP 4 ###
    # Add some tools. N.B. Do not use your own trigger decion tool. Use the
    # standard one that is included with AthMonitorAlgorithm.

    # # Add a tool that doesn't have its own configuration function. In
    # # this example, no accumulator is returned, so no merge is necessary.
    # from MyDomainPackage.MyDomainPackageConf import MyDomainTool
    # expertTrigMETMonAlg.MyDomainTool = MyDomainTool()

    # Add a generic monitoring tool (a "group" in old language). The returned
    # object here is the standard GenericMonitoringTool.
    metGroup = helper.addGroup(TrigMETMonAlg,'TrigMETMonitor','HLT/METMon/')

    # Add a GMT for the other example monitor algorithm
    metChain1Group = helper.addGroup(TrigMETMonChain1Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain1))
    metChain2Group = helper.addGroup(TrigMETMonChain2Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain2))
    metChain3Group = helper.addGroup(TrigMETMonChain3Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain3))
    metChain4Group = helper.addGroup(TrigMETMonChain4Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain4))
    metChain5Group = helper.addGroup(TrigMETMonChain5Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain5))
    metChain6Group = helper.addGroup(TrigMETMonChain6Alg,'TrigMETMonitor','HLT/METMon/{}'.format(TrigMETMonChain6))

    ### STEP 5 ###
    # Configure histograms
    #NB! The histograms defined here must match the ones in the cxx file exactly
    #
    # Binning
    et_bins=205 #Et
    et_min=-13.5
    et_max=401.5
    et_bins_log=20 #Et_log
    et_min_log=-1.875
    et_max_log=4.125
    ec_bins=199 #Ex, Ey, Ez
    ec_min=-298.5
    ec_max=298.5
    ec_bins_log=27 #Ex_log, Ey_log, Ez_log
    ec_min_log=-4.125
    ec_max_log=4.125
    sumet_bins=305 #sumEt
    sumet_min=-27.0
    sumet_max=4203.0
    sumet_bins_log=20 #sumEt_log
    sumet_min_log=-1.875
    sumet_max_log=4.125
    ## These bin settings are for future use.
    ## commented to avoid compiler warning.
    #sume_bins=153 #sumE
    #sume_min=-27.0
    #sume_max=24003.0
    #sume_bins_log=40 #sumE_log
    #sume_min_log=-1.875
    #sume_max_log=6.125
    phi_bins=64 # phi, used to be 100
    phi_bins_2d=32 # phi, used to be 24
    phi_min=-3.1416
    phi_max=3.1416
    eta_bins_2d=24# eta
    eta_min=-4.8
    eta_max=4.8
    eff_bins=42 # efficiency
    eff_min=-13.5
    eff_max=401.5
    # Histograms
    ## offline MET
    metGroup.defineHistogram('offline_Ex',title='Offline Missing E_{x};E_{x} [GeV];Events',
                             path='Expert/Offline',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('offline_Ey',title='Offline Missing E_{y};E_{y} [GeV];Events',
                             path='Expert/Offline',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('offline_Et',title='Offline Missing E_{T};E_{T} [GeV];Events',
                             path='Expert/Offline',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('offline_sumEt',title='Offline sumE_{T};sumE_{T} [GeV];Events',
                             path='Expert/Offline',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('offline_NoMu_Ex',title='Offline (No Mu) Missing E_{x};E_{x} [GeV];Events',
                             path='Expert/Offline',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('offline_NoMu_Ey',title='Offline (No Mu) Missing E_{y};E_{y} [GeV];Events',
                             path='Expert/Offline',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('offline_NoMu_Et',title='Offline (No Mu) Missing E_{T};E_{T} [GeV];Events',
                             path='Expert/Offline',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('offline_NoMu_sumEt',title='Offline (No Mu) sumE_{T};sumE_{T} [GeV];Events',
                             path='Expert/Offline',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    ## HLT electron and muon
    metGroup.defineHistogram('hlt_el_mult',title='HLT Electron Multiplicity;Number of electrons;Events',
                           path='Expert/ElMu',xbins=10,xmin=0,xmax=10)
    metGroup.defineHistogram('hlt_el_pt',title='HLT Electron p_{T};p_{T} [GeV];Events',
                           path='Expert/ElMu',xbins=100,xmin=0,xmax=100)
    metGroup.defineHistogram('hlt_mu_mult',title='HLT Muon Multiplicity;Number of muons;Events',
                           path='Expert/ElMu',xbins=10,xmin=0,xmax=10)
    metGroup.defineHistogram('hlt_mu_pt',title='HLT Muon p_{T};p_{T} [GeV];Events',
                           path='Expert/ElMu',xbins=100,xmin=0,xmax=100)
    ## Topoclusters
    metGroup.defineHistogram('hlt_topoclusters_mult',title='HLT Topoclusters Multiplicity;Number of Clusters;Events',
                           path='Expert/Topoclusters',xbins=120,xmin=0,xmax=1200)
    metGroup.defineHistogram('hlt_topoclusters_pt',title='HLT Topoclusters p_{T};p_{T} [GeV];Events',
                           path='Expert/Topoclusters',xbins=50,xmin=0,xmax=20)
    ## Tracks 
    metGroup.defineHistogram('hlt_tracks_mult',title='HLT Tracks Multiplicity;Number of Tracks;Events',
                           path='Expert/Tracks',xbins=120,xmin=0,xmax=1200)
    metGroup.defineHistogram('hlt_tracks_pt',title='HLT Tracks p_{T};p_{T} [GeV];Events',
                           path='Expert/Tracks',xbins=50,xmin=0,xmax=20)
    metGroup.defineHistogram('hlt_tracks_leading_pt',title='HLT Tracks Leading p_{T};p_{T} [GeV];Events',
                           path='Expert/Tracks',xbins=50,xmin=0,xmax=20)
    metGroup.defineHistogram('hlt_tracks_vec_sumPt',title='HLT Tracks Vector Sum p_{T};p_{T} [GeV];Events',
                           path='Expert/Tracks',xbins=100,xmin=0,xmax=100)
    metGroup.defineHistogram('hlt_tracks_sca_sumPt',title='HLT Tracks Scalar Sum p_{T};p_{T} [GeV];Events',
                           path='Expert/Tracks',xbins=100,xmin=0,xmax=1000)
    metGroup.defineHistogram('hlt_tracks_eta,hlt_tracks_phi;hlt_tracks_eta_phi', 
                           type='TH2F', 
                           title='HLT Tracks #eta - #phi (p_{T} > 3 GeV);#eta;#phi',
                           path='Expert/Tracks',
                           xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    ## Vertices
    metGroup.defineHistogram('hlt_vertex_mult',title='HLT Vertex Multiplicity;Number of Vertexs;Events',
                           path='Expert/Vertex',xbins=55,xmin=-5,xmax=50)
    metGroup.defineHistogram('hlt_vertex_z',title='HLT Vertex Z;Vertex Z [mm];Events',
                           path='Expert/Vertex',xbins=100,xmin=-200,xmax=200)
    metGroup.defineHistogram('hlt_vertex_z_diff',title='(HLT-Offline) Vertex Z Diff;Vertex Z [mm];Events',
                           path='Expert/Vertex',xbins=100,xmin=-200,xmax=200)
    metGroup.defineHistogram('hlt_vertex_mult_mu,act_IPBC;hlt_vertex_mult_mu',
                             type='TProfile',
                             title='Average Vertex Mult. per IPBC;Actual IPBC;Average Vertex Mult.',
                             path='Expert/Vertex',
                             xbins=55, xmin=-5, xmax=55)
    
    ## L1
    for alg in algsL1:
      metGroup.defineHistogram('L1_{}_Ex'.format(alg),
                             title='L1_{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_Ex_log'.format(alg),
                             title='L1_{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_Ey'.format(alg),
                             title='L1_{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_Ey_log'.format(alg),
                             title='L1_{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_Et'.format(alg),
                             title='L1_{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('L1_{}_Et_log'.format(alg),
                             title='L1_{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('L1_{}_sumEt'.format(alg),
                             title='L1_{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('L1_{}_sumEt_log'.format(alg),
                             title='L1_{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('L1_{}_phi'.format(alg),
                             title='L1_{} #phi;#phi;Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## L1 jFex gFex 
    for alg in algsL1Fex:
      metGroup.defineHistogram('L1_{}_Ex'.format(alg),
                             title='L1_{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_HT_Ex'.format(alg),
                             title='L1_{} HT Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_ST_Ex'.format(alg),
                             title='L1_{} ST Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_Ex_log'.format(alg),
                             title='L1_{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_HT_Ex_log'.format(alg),
                             title='L1_{} HT Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_ST_Ex_log'.format(alg),
                             title='L1_{} ST Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_Ey'.format(alg),
                             title='L1_{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_HT_Ey'.format(alg),
                             title='L1_{} HT Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_ST_Ey'.format(alg),
                             title='L1_{} ST Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('L1_{}_Ey_log'.format(alg),
                             title='L1_{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_HT_Ey_log'.format(alg),
                             title='L1_{} HT Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_ST_Ey_log'.format(alg),
                             title='L1_{} ST Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('L1_{}_Et'.format(alg),
                             title='L1_{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('L1_{}_Et_log'.format(alg),
                             title='L1_{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('L1_{}_sumEt'.format(alg),
                             title='L1_{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('L1_{}_sumEt_log'.format(alg),
                             title='L1_{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('L1_{}_phi'.format(alg),
                             title='L1_{} #phi;#phi;Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('L1_{}_ST_phi'.format(alg),
                             title='L1_{} ST #phi;#phi;Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('L1_{}_HT_phi'.format(alg),
                             title='L1_{} HT #phi;#phi;Events'.format(alg),
                             path='Shifter/L1_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT
    for alg in algsHLT:
      metGroup.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg), 
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT Preselection
    for alg in algsHLTPreSel:
      metGroup.defineHistogram('{}_presel_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/preSel',
                             xbins=et_bins,xmin=et_min,xmax=et_max)
    ## MET with LAr Noiseburst Veto applied
    for alg in LArNoiseBurstVetoAlgs:
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('{}_LArNoiseBurstVeto_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_LArNoiseBurstVeto_phi;{0}_phi_etweight'.format(alg), 
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_LArNoiseBurstVeto_eta,{0}_LArNoiseBurstVeto_phi;{0}_LArNoiseBurstVeto_eta_phi'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi;#eta;#phi'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
      metGroup.defineHistogram('{0}_LArNoiseBurstVeto_eta,{0}_LArNoiseBurstVeto_phi;{0}_LArNoiseBurstVeto_eta_phi_etweight'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi (etweighted);#eta;#phi'.format(alg),
                             weight='{}_LArNoiseBurstVeto_Et'.format(alg),
                             path='Shifter/LArNoiseBurstVetoed/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    # for alg in signalLepAlgs:
    for alg in signalLepAlgs:
      metGroup.defineHistogram('{}_SigEl_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_SigEl_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_SigEl_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_SigEl_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_SigEl_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('{}_SigEl_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('{}_SigEl_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('{}_SigEl_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('{}_SigEl_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_SigEl_phi;{0}_SigEl_phi_etweight'.format(alg), 
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_SigEl_Et'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_SigEl_eta,{0}_SigEl_phi;{0}_SigEl_eta_phi'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi;#eta;#phi'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
      metGroup.defineHistogram('{0}_SigEl_eta,{0}_SigEl_phi;{0}_SigEl_eta_phi_etweight'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi (etweighted);#eta;#phi'.format(alg),
                             weight='{}_SigEl_Et'.format(alg),
                             path='Shifter/SignalEl/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
      metGroup.defineHistogram('{}_SigMu_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_SigMu_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_SigMu_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_SigMu_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metGroup.defineHistogram('{}_SigMu_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('{}_SigMu_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metGroup.defineHistogram('{}_SigMu_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metGroup.defineHistogram('{}_SigMu_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metGroup.defineHistogram('{}_SigMu_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_SigMu_phi;{0}_SigMu_phi_etweight'.format(alg), 
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_SigMu_Et'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metGroup.defineHistogram('{0}_SigMu_eta,{0}_SigMu_phi;{0}_SigMu_eta_phi'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi;#eta;#phi'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
      metGroup.defineHistogram('{0}_SigMu_eta,{0}_SigMu_phi;{0}_SigMu_eta_phi_etweight'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi (etweighted);#eta;#phi'.format(alg),
                             weight='{}_SigMu_Et'.format(alg),
                             path='Shifter/SignalMu/{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)

    ## HLT 2d eta-phi histos
    for alg in algsHLT2d:
      metGroup.defineHistogram('{0}_eta,{0}_phi;{0}_eta_phi'.format(alg), 
                             type='TH2F', 
                             title='{} #eta - #phi;#eta;#phi'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
      metGroup.defineHistogram('{a}_eta,{a}_phi;{a}_eta_phi_etweight'.format(a=alg), 
                             type='TH2F', 
                             title='{} #eta - #phi (etweighted);#eta;#phi'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='Shifter/HLT_{}'.format(alg),
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)

    ## L1 efficiency
    for chain in L1Chains:
       metGroup.defineHistogram('offline_NoMu_Et_eff,pass_{};{}_eff'.format(chain,chain),
                                type='TProfile',
                                title='{} efficiency;offline E_{{T}} [GeV];Efficiency'.format(chain),
                                path='Shifter/eff',
                                xbins=eff_bins, xmin=eff_min, xmax=eff_max)
    ## HLT efficiency
    for chain in HLTChains:
       metGroup.defineHistogram('offline_NoMu_Et_eff,pass_{};{}_eff'.format(chain,chain),
                                type='TProfile',
                                title='{} efficiency;offline E_{{T}} [GeV];Efficiency'.format(chain),
                                path='Shifter/eff',
                                xbins=eff_bins, xmin=eff_min, xmax=eff_max)
    ## HLT efficiency for expert
    for chain in HLTChainsVal:
       metGroup.defineHistogram('offline_NoMu_Et_eff,pass_{};{}_eff'.format(chain,chain),
                                type='TProfile',
                                title='{} efficiency;offline E_{{T}} [GeV];Efficiency'.format(chain),
                                path='Expert/eff',
                                xbins=eff_bins, xmin=eff_min, xmax=eff_max)
    for chain in HLTChainsT0:
       metGroup.defineHistogram('offline_NoMu_Et_eff,pass_{};{}_eff'.format(chain,chain),
                                type='TProfile',
                                title='{} efficiency;offline E_{{T}} [GeV];Efficiency'.format(chain),
                                path='Expert/eff',
                                xbins=eff_bins, xmin=eff_min, xmax=eff_max)
    
    ## pileup
    metGroup.defineHistogram('act_IPBC', type='TH1F',title='Actual IPBC;Actual IPBC;Events',
                             path='Shifter/Component',xbins=100,xmin=0,xmax=100)
    ## HLT cell component
    metGroup.defineHistogram('HLT_MET_status',type='TH1F',title='HLT MET Status;;',
                             weight='MET_status',
                             path='Shifter/Component',
                             xbins=len(bit_names),xmin=-0.5,xmax=31.5, xlabels=bit_names)	  
    metGroup.defineHistogram('HLT_MET_component,component_Et;compN_compEt', 
                             type='TH2F', 
                              title='HLT Missing E_{T} VS component;;Missing E_{T} [GeV]',
                             path='Shifter/Component',
                             xbins=len(comp_names),xmin=-0.5,xmax=24.5,ybins=et_bins,ymin=et_min,ymax=et_max,
                             xlabels=comp_names)
    metGroup.defineHistogram('component,component_status;compN_HLT_MET_status',
                             type='TH2F', 
                             title='HLT MET Status VS component;;',
                             weight='component_status_weight',
                             path='Shifter/Component',
                             xbins=len(comp_names),xmin=-0.5,xmax=24.5,ybins=len(comp_names),ymin=-0.5,ymax=31.5,
                             xlabels=comp_names, ylabels=bit_names)
    ## HLT Expert
    for alg in algsHLTExpert:
      metGroup.defineHistogram('{}_Ex'.format(alg),
                               title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                               path='Expert/HLT_{}'.format(alg),
                               xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_Ey'.format(alg),
                               title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                               path='Expert/HLT_{}'.format(alg),
                               xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metGroup.defineHistogram('{}_Et'.format(alg), 
                               title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                               path='Expert/HLT_{}'.format(alg),
                               xbins=et_bins,xmin=et_min,xmax=et_max)
      metGroup.defineHistogram('{}_sumEt'.format(alg),
                               title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                               path='Expert/HLT_{}'.format(alg),
                               xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    #2D MET tcpufit vs pfopufit, pfsum_cssk, trkmht
    for alg in algsMET2d_tcpufit:
      metGroup.defineHistogram('{}_2D_Et,tcpufit_2D_Et;hlt_tcpufit_Et_{}_Et'.format(alg, alg), 
                               type='TH2F', 
                               title='HLT tcpufit Missing E_{{T}} vs. HLT {} Missing Et;{} E_{{T}} [GeV];tcpufit E_{{T}} [GeV]'.format(alg, alg),
                               path='Expert/HLT_MET2D',
                               xbins=40,xmin=et_min,xmax=et_max,ybins=40,ymin=et_min,ymax=et_max) 
    ## Chain1 specific
    for alg in algsHLTChain1:
      metChain1Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain1Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain1Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain1Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain1Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain1Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain1Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain1Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain1Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain1Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg), 
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## Chain2 specific
    for alg in algsHLTChain2:
      metChain2Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain2Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain2Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain2Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain2Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain2Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain2Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain2Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain2Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain2Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg),
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## Chain3 specific
    for alg in algsHLTChain3:
      metChain3Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain3Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain3Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain3Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain3Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain3Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain3Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain3Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain3Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain3Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg),
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)

    ## Chain4 specific
    for alg in algsHLTChain4:
      metChain4Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain4Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain4Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain4Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain4Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain4Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain4Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain4Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain4Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain4Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg),
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)

    ## Chain5 specific
    for alg in algsHLTChain5:
      metChain5Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain5Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain5Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain5Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain5Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain5Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain5Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain5Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain5Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain5Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg),
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)

    ## Chain6 specific
    for alg in algsHLTChain6:
      metChain6Group.defineHistogram('{}_Ex'.format(alg),
                             title='{} Missing E_{{x}};E_{{x}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain6Group.defineHistogram('{}_Ex_log'.format(alg),
                             title='{} Missing E_{{x}} log;sgn(E_{{x}}) log(E_{{x}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain6Group.defineHistogram('{}_Ey'.format(alg),
                             title='{} Missing E_{{y}};E_{{y}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins,xmin=ec_min,xmax=ec_max)
      metChain6Group.defineHistogram('{}_Ey_log'.format(alg),
                             title='{} Missing E_{{y}} log;sgn(E_{{y}}) log(E_{{y}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
      metChain6Group.defineHistogram('{}_Et'.format(alg),
                             title='{} Missing E_{{T}};E_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins,xmin=et_min,xmax=et_max)
      metChain6Group.defineHistogram('{}_Et_log'.format(alg),
                             title='{} Missing E_{{T}} log;log(E_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
      metChain6Group.defineHistogram('{}_sumEt'.format(alg),
                             title='{} sumE_{{T}};sumE_{{T}} [GeV];Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
      metChain6Group.defineHistogram('{}_sumEt_log'.format(alg),
                             title='{} sumE_{{T}} log;log(sumE_{{T}}/GeV);Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
      metChain6Group.defineHistogram('{}_phi'.format(alg),
                             title='{} #phi;#phi;Events'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)
      metChain6Group.defineHistogram('{0}_phi;{0}_phi_etweight'.format(alg),
                             title='{} #phi (etweighted);#phi;Et weighted events'.format(alg),
                             weight='{}_Et'.format(alg),
                             path='HLT_{}'.format(alg),
                             xbins=phi_bins,xmin=phi_min,xmax=phi_max)


    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can
    # just return directly (and not create "result" above)
    return helper.result()

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/'
    file = 'data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1'
    flags = initConfigFlags()
    flags.Input.Files = [nightly+file]
    flags.Input.isMC = True
    flags.Output.HISTFileName = 'TrigMETMonitorOutput.root'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    trigMETMonitorAcc = TrigMETMonConfig(flags)
    cfg.merge(trigMETMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigMETMonitorAcc.getEventAlgo('TrigMETMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    cfg.run() #use cfg.run(20) to only run on first 20 events
