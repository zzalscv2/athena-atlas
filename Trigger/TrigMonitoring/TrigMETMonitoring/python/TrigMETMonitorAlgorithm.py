#
#  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
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

    # # If for some really obscure reason you need to instantiate an algorithm
    # # yourself, the AddAlgorithm method will still configure the base 
    # # properties and add the algorithm to the monitoring sequence.
    # helper.AddAlgorithm(myExistingAlg)


    ### STEP 3 ###
    # Edit properties of a algorithm
    # some generic property
    # expertTrigMETMonAlg.RandomHist = True
    # to enable a trigger filter, for example:
    # TrigMETMonAlg.TriggerChain = 'HLT_xe30_cell_L1XE10'
    # without filters, all events are processed.
    TrigMETMonChain1Alg.TriggerChain = 'HLT_xe65_cell_L1XE50'
    
    ### check Run2 or Run3 MT
    mt_chains = True
    if ( inputFlags.Trigger.EDMDecodingVersion < 3 ) :
      mt_chains = False

    ### container name selection
    if mt_chains:
      TrigMETMonAlg.hlt_pfsum_key = 'HLT_MET_pfsum'
    else:
      TrigMETMonAlg.l1_jnc_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.l1_jrho_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.l1_gnc_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.l1_grho_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.l1_gjwoj_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.l1_gpufit_key = 'LVL1EnergySumRoI'
      TrigMETMonAlg.hlt_cell_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET'
      TrigMETMonAlg.hlt_mt_key = 'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht'
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
    ### these are default chains ######
      #TrigMETMonAlg.L1Chain02 = 'L1_XE50'
      #TrigMETMonAlg.L1Chain02 = 'L1_jXENC50'
      #TrigMETMonAlg.L1Chain03 = 'L1_jXERHO50'
      #TrigMETMonAlg.L1Chain04 = 'L1_gXENC50'
      #TrigMETMonAlg.L1Chain05 = 'L1_gXERHO50'
      #TrigMETMonAlg.L1Chain06 = 'L1_gXEJWOJ50'
      #TrigMETMonAlg.L1Chain07 = 'L1_gXEPUFIT50'
      #TrigMETMonAlg.HLTChain01 = 'HLT_xe65_cell_L1XE50'
      #TrigMETMonAlg.HLTChain02 = 'HLT_xe100_mht_L1XE50'
      #TrigMETMonAlg.HLTChain03 = 'HLT_xe100_tcpufit_L1XE50'
      #TrigMETMonAlg.HLTChain04 = 'HLT_xe100_trkmht_L1XE50'
      #TrigMETMonAlg.HLTChain05 = 'HLT_xe100_pfsum_L1XE50'
      #TrigMETMonAlg.HLTChain06 = 'HLT_xe100_pfopufit_L1XE50'
      #TrigMETMonAlg.HLTChain07 = 'HLT_xe100_cvfpufit_L1XE50'
      #TrigMETMonAlg.HLTChain08 = 'HLT_xe100_mhtpufit_em_subjesgscIS_L1XE50'
      #TrigMETMonAlg.HLTChain09 = 'HLT_xe100_mhtpufit_pf_subjesgscIS_L1XE50'
      #TrigMETMonAlg.HLTChain10 = 'HLT_xe100_pfsum_cssk_L1XE50'
      #TrigMETMonAlg.HLTChain11 = 'HLT_xe100_pfsum_vssk_L1XE50'
      #TrigMETMonAlg.HLTChain12 = 'HLT_xe65_cell_xe110_tcpuft_L1XE50'
      #TrigMETMonAlg.HLTChain13 = 'HLT_xe100_trkmht_xe85_tcpufit_xe65_cell_L1XE50'
      #TrigMETMonAlg.HLTChain14 = 'HLT_xe95_trkmht_xe90_tcpufit_xe75_cell_L1XE50'
    if mt_chains:
      TrigMETMonAlg.HLTChain02 = 'HLT_xe30_cell_L1XE10'
    else: 
      TrigMETMonAlg.L1Chain02 = 'L1_XE10'
      TrigMETMonAlg.L1Chain03 = 'L1_XE30'
      TrigMETMonAlg.L1Chain04 = 'L1_XE55'
      TrigMETMonAlg.L1Chain05 = 'L1_XE60'
      TrigMETMonAlg.L1Chain06 = 'L1_XE70'
      TrigMETMonAlg.L1Chain07 = 'L1_XE75'
      TrigMETMonAlg.HLTChain01 = 'HLT_xe70_mht_L1XE50'
      TrigMETMonAlg.HLTChain02 = 'HLT_xe90_mht_L1XE50'
      TrigMETMonAlg.HLTChain03 = 'HLT_xe110_mht_L1XE50'
      TrigMETMonAlg.HLTChain04 = 'HLT_xe90_pufit_L1XE50'
      TrigMETMonAlg.HLTChain05 = 'HLT_xe100_pufit_L1XE50'
      TrigMETMonAlg.HLTChain06 = 'HLT_xe110_pufit_L1XE50'
      TrigMETMonAlg.HLTChain07 = 'HLT_xe110_pufit_xe65_L1XE50'
      TrigMETMonAlg.HLTChain08 = 'HLT_xe110_pufit_xe70_L1XE50'


    ### STEP 4 ###
    # Add some tools. N.B. Do not use your own trigger decion tool. Use the
    # standard one that is included with AthMonitorAlgorithm.

    # # First, add a tool that's set up by a different configuration function. 
    # # In this case, CaloNoiseToolCfg returns its own component accumulator, 
    # # which must be merged with the one from this function.

    # # Then, add a tool that doesn't have its own configuration function. In
    # # this example, no accumulator is returned, so no merge is necessary.
    # from MyDomainPackage.MyDomainPackageConf import MyDomainTool
    # expertTrigMETMonAlg.MyDomainTool = MyDomainTool()

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    metGroup = helper.addGroup(TrigMETMonAlg,'TrigMETMonitor','HLT/METMon/')

    # Add a GMT for the other example monitor algorithm
    metChain1Group = helper.addGroup(TrigMETMonChain1Alg,'TrigMETMonitor','HLT/METMon/HLT_xe65_cell_L1XE50/')

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
    sume_bins=153 #sumE
    sume_min=-27.0
    sume_max=24003.0
    sume_bins_log=40 #sumE_log
    sume_min_log=-1.875
    sume_max_log=6.125
    phi_bins=100 # phi
    phi_bins_2d=24 # phi
    phi_min=-3.1416
    phi_max=3.1416
    eta_bins=100 # eta
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
    ## L1 roi
    metGroup.defineHistogram('L1_Ex',title='L1 Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_Ey',title='L1 Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_Et',title='L1 Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_sumEt',title='L1 sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 jnc
    metGroup.defineHistogram('L1_jnc_Ex',title='L1_jnc Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_jnc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_jnc_Ey',title='L1_jnc Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_jnc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_jnc_Et',title='L1_jnc Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_jnc',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_jnc_sumEt',title='L1_jnc sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_jnc',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 jrho
    metGroup.defineHistogram('L1_jrho_Ex',title='L1_jrho Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_jrho',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_jrho_Ey',title='L1_jrho Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_jrho',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_jrho_Et',title='L1_jrho Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_jrho',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_jrho_sumEt',title='L1_jrho sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_jrho',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 gnc
    metGroup.defineHistogram('L1_gnc_Ex',title='L1_gnc Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_gnc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gnc_Ey',title='L1_gnc Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_gnc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gnc_Et',title='L1_gnc Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_gnc',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_gnc_sumEt',title='L1_gnc sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_gnc',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 grho
    metGroup.defineHistogram('L1_grho_Ex',title='L1_grho Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_grho',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_grho_Ey',title='L1_grho Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_grho',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_grho_Et',title='L1_grho Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_grho',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_grho_sumEt',title='L1_grho sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_grho',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 gjwoj
    metGroup.defineHistogram('L1_gjwoj_Ex',title='L1_gjwoj Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_gjwoj',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gjwoj_Ey',title='L1_gjwoj Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_gjwoj',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gjwoj_Et',title='L1_gjwoj Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_gjwoj',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_gjwoj_sumEt',title='L1_gjwoj sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_gjwoj',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## L1 gpufit
    metGroup.defineHistogram('L1_gpufit_Ex',title='L1_gpufit Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/L1_gpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gpufit_Ey',title='L1_gpufit Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/L1_gpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('L1_gpufit_Et',title='L1_gpufit Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/L1_gpufit',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('L1_gpufit_sumEt',title='L1_gpufit sumE_{T};sumE_{T} [GeV];Events',
                             path='Shifter/L1_gpufit',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## HLT cell
    metGroup.defineHistogram('cell_Ex',title='cell Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/cell',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('cell_Ex_log',title='cell Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/cell',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('cell_Ey',title='cell Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/cell',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('cell_Ey_log',title='cell Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/cell',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('cell_Et',title='cell Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/cell',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('cell_Et_log',title='cell Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/cell',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('cell_sumEt',title='cell sumEt;sumEt [GeV];Events',
                             path='Shifter/cell',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('cell_sumEt_log',title='cell sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/cell',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('cell_phi',title='cell #phi;#phi;Events',
                             path='Shifter/cell',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('cell_phi;cell_phi_etweight', title='cell #phi (etweighted);#phi;E_{T} weighted events',
                             weight='cell_Et',
                             path='Shifter/cell',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('cell_eta,cell_phi;cell_eta_phi', type='TH2F', title='cell #eta - #phi;#eta;#phi',
                             path='Shifter/cell',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    metGroup.defineHistogram('cell_eta,cell_phi;cell_eta_phi_etweight', type='TH2F', title='cell #eta - #phi (etweighted);#eta;#phi',
                             weight='cell_Et',
                             path='Shifter/cell',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    ## HLT trkmht
    metGroup.defineHistogram('trkmht_Ex',title='trkmht Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/trkmht',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('trkmht_Ex_log',title='trkmht Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/trkmht',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('trkmht_Ey',title='trkmht Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/trkmht',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('trkmht_Ey_log',title='trkmht Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/trkmht',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('trkmht_Et',title='trkmht Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/trkmht',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('trkmht_Et_log',title='trkmht Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/trkmht',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('trkmht_sumEt',title='trkmht sumEt;sumEt [GeV];Events',
                             path='Shifter/trkmht',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('trkmht_sumEt_log',title='trkmht sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/trkmht',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('trkmht_phi',title='trkmht #phi;#phi;Events',
                             path='Shifter/trkmht',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('trkmht_phi;trkmht_phi_etweight', title='trkmht #phi (etweighted);#phi;E_{T} weighted events',
                             weight='trkmht_Et',
                             path='Shifter/trkmht',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT tcpufit
    metGroup.defineHistogram('tcpufit_Ex',title='tcpufit Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/tcpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tcpufit_Ex_log',title='tcpufit Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/tcpufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('tcpufit_Ey',title='tcpufit Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/tcpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tcpufit_Ey_log',title='tcpufit Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/tcpufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('tcpufit_Ez',title='tcpufit Missing E_{z};E_{z} [GeV];Events',
                             path='Shifter/tcpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tcpufit_Ez_log',title='tcpufit Missing E_{z} log;sgn(E_{z}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/tcpufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('tcpufit_Et',title='tcpufit Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/tcpufit',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('tcpufit_Et_log',title='tcpufit Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/tcpufit',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('tcpufit_sumEt',title='tcpufit sumEt;sumEt [GeV];Events',
                             path='Shifter/tcpufit',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('tcpufit_sumEt_log',title='tcpufit sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/tcpufit',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('tcpufit_sumEt;tcpufit_sumEt_etweight',title='tcpufit sumEt (etweighted);sumEt [GeV];E_{T} weighted events',
                             weight='tcpufit_Et',
                             path='Shifter/tcpufit',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('tcpufit_sumE',title='tcpufit sumE;sumE [GeV];Events',
                             path='Shifter/tcpufit',xbins=sume_bins,xmin=sume_min,xmax=sume_max)
    metGroup.defineHistogram('tcpufit_sumE_log',title='tcpufit sumE log;log_{10}(sumE/GeV);Events',
                             path='Shifter/tcpufit',xbins=sume_bins_log,xmin=sume_min_log,xmax=sume_max_log)
    metGroup.defineHistogram('tcpufit_eta',title='tcpufit #eta;#eta;Events',
                             path='Shifter/tcpufit',xbins=eta_bins,xmin=eta_min,xmax=eta_max)
    metGroup.defineHistogram('tcpufit_phi',title='tcpufit #phi;#phi;Events',
                             path='Shifter/tcpufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('tcpufit_phi;tcpufit_phi_etweight', title='tcpufit #phi (etweighted);#phi;E_{T} weighted events', 
                             weight='tcpufit_Et',
                             path='Shifter/tcpufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('tcpufit_eta,tcpufit_phi;tcpufit_eta_phi', type='TH2F', title='tcpufit #eta - #phi;#eta;#phi',
                             path='Shifter/tcpufit',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    metGroup.defineHistogram('tcpufit_eta,tcpufit_phi;tcpufit_eta_phi_etweight', type='TH2F', title='tcpufit #eta - #phi (etweighted);#eta;#phi',
                             weight='tcpufit_Et',
                             path='Shifter/tcpufit',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    ## Efficiency
    metGroup.defineHistogram('offline_Et,pass_L101;L101_eff', type='TProfile',title='L101 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L102;L102_eff', type='TProfile',title='L102 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L103;L103_eff', type='TProfile',title='L103 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L104;L104_eff', type='TProfile',title='L104 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L105;L105_eff', type='TProfile',title='L105 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L106;L106_eff', type='TProfile',title='L106 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_L107;L107_eff', type='TProfile',title='L107 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT01;HLT01_eff', type='TProfile',title='HLT01 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT02;HLT02_eff', type='TProfile',title='HLT02 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT03;HLT03_eff', type='TProfile',title='HLT03 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT04;HLT04_eff', type='TProfile',title='HLT04 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT05;HLT05_eff', type='TProfile',title='HLT05 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT06;HLT06_eff', type='TProfile',title='HLT06 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT07;HLT07_eff', type='TProfile',title='HLT07 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT08;HLT08_eff', type='TProfile',title='HLT08 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT09;HLT09_eff', type='TProfile',title='HLT09 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT10;HLT10_eff', type='TProfile',title='HLT10 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT11;HLT11_eff', type='TProfile',title='HLT11 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT12;HLT12_eff', type='TProfile',title='HLT12 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT13;HLT13_eff', type='TProfile',title='HLT13 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    metGroup.defineHistogram('offline_Et,pass_HLT14;HLT14_eff', type='TProfile',title='HLT14 efficiency;offline E_{T} [GeV];Efficiency',
                             path='Shifter/eff',xbins=eff_bins,xmin=eff_min,xmax=eff_max)
    ## HLT mht
    metGroup.defineHistogram('mht_Ex',title='mht Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/mht',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mht_Ex_log',title='mht Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/mht',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mht_Ey',title='mht Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/mht',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mht_Ey_log',title='mht Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/mht',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mht_Et', title='mht Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/mht',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('mht_Et_log',title='mht Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/mht',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('mht_sumEt',title='mht sumEt;sumEt [GeV];Events',
                             path='Shifter/mht',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('mht_sumEt_log',title='mht sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/mht',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('mht_phi',title='mht #phi;#phi;Events',
                             path='Shifter/mht',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('mht_phi;mht_phi_etweight', title='mht #phi (etweighted);#phi;E_{T} weighted events',
                             weight='mht_Et',
                             path='Shifter/mht',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('mht_eta,cell_phi;mht_eta_phi', type='TH2F', title='mht #eta - #phi;#eta;#phi',
                             path='Shifter/mht',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    metGroup.defineHistogram('mht_eta,mht_phi;mht_eta_phi_etweight', type='TH2F', title='mht #eta - #phi (etweighted);#eta;#phi',
                             weight='mht_Et',
                             path='Shifter/mht',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    ## HLT tc_em
    metGroup.defineHistogram('tc_em_Ex',title='tc_em Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/tc_em',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tc_em_Ex_log',title='tc_em Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/tc_em',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('tc_em_Ey',title='tc_em Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/tc_em',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tc_em_Ey_log',title='tc_em Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/tc_em',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('tc_em_Et', title='tc_em Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/tc_em',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('tc_em_Et_log',title='tc_em Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/tc_em',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('tc_em_sumEt',title='tc_em sumEt;sumEt [GeV];Events',
                             path='Shifter/tc_em',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('tc_em_sumEt_log',title='tc_em sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/tc_em',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('tc_em_phi',title='tc_em #phi;#phi;Events',
                             path='Shifter/tc_em',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('tc_em_phi;tc_em_phi_etweight', title='tc_em #phi (etweighted);#phi;E_{T} weighted events',
                             weight='tc_em_Et',
                             path='Shifter/tc_em',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('tc_em_eta,cell_phi;tc_em_eta_phi', type='TH2F', title='tc_em #eta - #phi;#eta;#phi',
                             path='Shifter/tc_em',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    metGroup.defineHistogram('tc_em_eta,tc_em_phi;tc_em_eta_phi_etweight', type='TH2F', title='tc_em #eta - #phi (etweighted);#eta;#phi',
                             weight='tc_em_Et',
                             path='Shifter/tc_em',
                             xbins=eta_bins_2d,xmin=eta_min,xmax=eta_max,ybins=phi_bins_2d,ymin=phi_min,ymax=phi_max)
    ## HLT pfsum
    metGroup.defineHistogram('pfsum_Ex',title='pfsum Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/pfsum',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_Ex_log',title='pfsum Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/pfsum',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_Ey',title='pfsum Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/pfsum',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_Ey_log',title='pfsum Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/pfsum',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_Et', title='pfsum Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/pfsum',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('pfsum_Et_log',title='pfsum Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/pfsum',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('pfsum_sumEt',title='pfsum sumEt;sumEt [GeV];Events',
                             path='Shifter/pfsum',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('pfsum_sumEt_log',title='pfsum sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/pfsum',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('pfsum_phi',title='pfsum #phi;#phi;Events',
                             path='Shifter/pfsum',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('pfsum_phi;pfsum_phi_etweight', title='pfsum #phi (etweighted);#phi;E_{T} weighted events',
                             weight='pfsum_Et',
                             path='Shifter/pfsum',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT pfsum_cssk
    metGroup.defineHistogram('pfsum_cssk_Ex',title='pfsum_cssk Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/pfsum_cssk',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_cssk_Ex_log',title='pfsum_cssk Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/pfsum_cssk',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_cssk_Ey',title='pfsum_cssk Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/pfsum_cssk',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_cssk_Ey_log',title='pfsum_cssk Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/pfsum_cssk',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_cssk_Et', title='pfsum_cssk Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/pfsum_cssk',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('pfsum_cssk_Et_log',title='pfsum_cssk Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/pfsum_cssk',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('pfsum_cssk_sumEt',title='pfsum_cssk sumEt;sumEt [GeV];Events',
                             path='Shifter/pfsum_cssk',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('pfsum_cssk_sumEt_log',title='pfsum_cssk sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/pfsum_cssk',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('pfsum_cssk_phi',title='pfsum_cssk #phi;#phi;Events',
                             path='Shifter/pfsum_cssk',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('pfsum_cssk_phi;pfsum_cssk_phi_etweight', title='pfsum_cssk #phi (etweighted);#phi;E_{T} weighted events',
                             weight='pfsum_cssk_Et',
                             path='Shifter/pfsum_cssk',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT pfsum_vssk
    metGroup.defineHistogram('pfsum_vssk_Ex',title='pfsum_vssk Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/pfsum_vssk',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_vssk_Ex_log',title='pfsum_vssk Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/pfsum_vssk',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_vssk_Ey',title='pfsum_vssk Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/pfsum_vssk',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfsum_vssk_Ey_log',title='pfsum_vssk Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/pfsum_vssk',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfsum_vssk_Et', title='pfsum_vssk Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/pfsum_vssk',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('pfsum_vssk_Et_log',title='pfsum_vssk Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/pfsum_vssk',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('pfsum_vssk_sumEt',title='pfsum_vssk sumEt;sumEt [GeV];Events',
                             path='Shifter/pfsum_vssk',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('pfsum_vssk_sumEt_log',title='pfsum_vssk sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/pfsum_vssk',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('pfsum_vssk_phi',title='pfsum_vssk #phi;#phi;Events',
                             path='Shifter/pfsum_vssk',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('pfsum_vssk_phi;pfsum_vssk_phi_etweight', title='pfsum_vssk #phi (etweighted);#phi;E_{T} weighted events',
                             weight='pfsum_vssk_Et',
                             path='Shifter/pfsum_vssk',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT pfopufit
    metGroup.defineHistogram('pfopufit_Ex',title='pfopufit Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/pfopufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfopufit_Ex_log',title='pfopufit Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/pfopufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfopufit_Ey',title='pfopufit Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/pfopufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('pfopufit_Ey_log',title='pfopufit Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/pfopufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('pfopufit_Et', title='pfopufit Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/pfopufit',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('pfopufit_Et_log',title='pfopufit Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/pfopufit',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('pfopufit_sumEt',title='pfopufit sumEt;sumEt [GeV];Events',
                             path='Shifter/pfopufit',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('pfopufit_sumEt_log',title='pfopufit sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/pfopufit',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('pfopufit_phi',title='pfopufit #phi;#phi;Events',
                             path='Shifter/pfopufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('pfopufit_phi;pfopufit_phi_etweight', title='pfopufit #phi (etweighted);#phi;E_{T} weighted events',
                             weight='pfopufit_Et',
                             path='Shifter/pfopufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## cvfpufit
    metGroup.defineHistogram('cvfpufit_Ex',title='cvfpufit Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/cvfpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('cvfpufit_Ex_log',title='cvfpufit Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/cvfpufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('cvfpufit_Ey',title='cvfpufit Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/cvfpufit',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('cvfpufit_Ey_log',title='cvfpufit Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/cvfpufit',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('cvfpufit_Et', title='cvfpufit Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/cvfpufit',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('cvfpufit_Et_log',title='cvfpufit Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/cvfpufit',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('cvfpufit_sumEt',title='cvfpufit sumEt;sumEt [GeV];Events',
                             path='Shifter/cvfpufit',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('cvfpufit_sumEt_log',title='cvfpufit sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/cvfpufit',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('cvfpufit_phi',title='cvfpufit #phi;#phi;Events',
                             path='Shifter/cvfpufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('cvfpufit_phi;cvfpufit_phi_etweight', title='cvfpufit #phi (etweighted);#phi;E_{T} weighted events',
                             weight='cvfpufit_Et',
                             path='Shifter/cvfpufit',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## mhtpufit_pf
    metGroup.defineHistogram('mhtpufit_pf_Ex',title='mhtpufit_pf Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/mhtpufit_pf',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mhtpufit_pf_Ex_log',title='mhtpufit_pf Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/mhtpufit_pf',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mhtpufit_pf_Ey',title='mhtpufit_pf Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/mhtpufit_pf',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mhtpufit_pf_Ey_log',title='mhtpufit_pf Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/mhtpufit_pf',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mhtpufit_pf_Et', title='mhtpufit_pf Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/mhtpufit_pf',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('mhtpufit_pf_Et_log',title='mhtpufit_pf Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/mhtpufit_pf',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('mhtpufit_pf_sumEt',title='mhtpufit_pf sumEt;sumEt [GeV];Events',
                             path='Shifter/mhtpufit_pf',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('mhtpufit_pf_sumEt_log',title='mhtpufit_pf sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/mhtpufit_pf',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('mhtpufit_pf_phi',title='mhtpufit_pf #phi;#phi;Events',
                             path='Shifter/mhtpufit_pf',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('mhtpufit_pf_phi;mhtpufit_pf_phi_etweight', title='mhtpufit_pf #phi (etweighted);#phi;E_{T} weighted events',
                             weight='mhtpufit_pf_Et',
                             path='Shifter/mhtpufit_pf',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## mhtpufit_em
    metGroup.defineHistogram('mhtpufit_em_Ex',title='mhtpufit_em Missing E_{x};E_{x} [GeV];Events',
                             path='Shifter/mhtpufit_em',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mhtpufit_em_Ex_log',title='mhtpufit_em Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='Shifter/mhtpufit_em',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mhtpufit_em_Ey',title='mhtpufit_em Missing E_{y};E_{y} [GeV];Events',
                             path='Shifter/mhtpufit_em',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('mhtpufit_em_Ey_log',title='mhtpufit_em Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='Shifter/mhtpufit_em',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metGroup.defineHistogram('mhtpufit_em_Et', title='mhtpufit_em Missing E_{T};E_{T} [GeV];Events',
                             path='Shifter/mhtpufit_em',xbins=et_bins,xmin=et_min,xmax=et_max)
    metGroup.defineHistogram('mhtpufit_em_Et_log',title='mhtpufit_em Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='Shifter/mhtpufit_em',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metGroup.defineHistogram('mhtpufit_em_sumEt',title='mhtpufit_em sumEt;sumEt [GeV];Events',
                             path='Shifter/mhtpufit_em',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metGroup.defineHistogram('mhtpufit_em_sumEt_log',title='mhtpufit_em sumEt log;log_{10}(sumEt/GeV);Events',
                             path='Shifter/mhtpufit_em',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metGroup.defineHistogram('mhtpufit_em_phi',title='mhtpufit_em #phi;#phi;Events',
                             path='Shifter/mhtpufit_em',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metGroup.defineHistogram('mhtpufit_em_phi;mhtpufit_em_phi_etweight', title='mhtpufit_em #phi (etweighted);#phi;E_{T} weighted events',
                             weight='mhtpufit_em_Et',
                             path='Shifter/mhtpufit_em',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    ## HLT tc
    metGroup.defineHistogram('tc_Ex',title='tc Missing E_{x};E_{x} [GeV];Events',
                             path='Expert/tc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tc_Ey',title='tc Missing E_{y};E_{y} [GeV];Events',
                             path='Expert/tc',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metGroup.defineHistogram('tc_Et', title='tc Missing E_{T};E_{T} [GeV];Events',
                             path='Expert/tc',xbins=et_bins,xmin=et_min,xmax=et_max)
    ## Chain specific
    metChain1Group.defineHistogram('cell_Ex',title='cell Missing E_{x};E_{x} [GeV];Events',
                                  path='cell',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metChain1Group.defineHistogram('cell_Ex_log',title='cell Missing E_{x} log;sgn(E_{x}) log_{10}(E_{x}/GeV);Events',
                             path='cell',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metChain1Group.defineHistogram('cell_Ey',title='cell Missing E_{y};E_{y} [GeV];Events',
                                  path='cell',xbins=ec_bins,xmin=ec_min,xmax=ec_max)
    metChain1Group.defineHistogram('cell_Ey_log',title='cell Missing E_{y} log;sgn(E_{y}) log_{10}(E_{y}/GeV);Events',
                             path='cell',xbins=ec_bins_log,xmin=ec_min_log,xmax=ec_max_log)
    metChain1Group.defineHistogram('cell_Et',title='cell Missing E_{T};E_{T} [GeV];Events',
                                  path='cell',xbins=et_bins,xmin=et_min,xmax=et_max)
    metChain1Group.defineHistogram('cell_Et_log',title='cell Missing E_{T} log;log_{10}(E_{T}/GeV);Events',
                             path='cell',xbins=et_bins_log,xmin=et_min_log,xmax=et_max_log)
    metChain1Group.defineHistogram('cell_sumEt',title='cell sumEt;sumEt [GeV];Events',
                             path='cell',xbins=sumet_bins,xmin=sumet_min,xmax=sumet_max)
    metChain1Group.defineHistogram('cell_sumEt_log',title='cell sumEt log;log_{10}(sumEt/GeV);Events',
                             path='cell',xbins=sumet_bins_log,xmin=sumet_min_log,xmax=sumet_max_log)
    metChain1Group.defineHistogram('cell_phi',title='cell #phi;#phi;Events',
                             path='cell',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    metChain1Group.defineHistogram('cell_phi;cell_phi_etweight', title='cell #phi (etweighted);#phi;E_{T} weighted events',
                             weight='cell_Et',
                             path='cell',xbins=phi_bins,xmin=phi_min,xmax=phi_max)
    
    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()
    
if __name__=='__main__':
    # Setup the Run III behavior
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/'
    file = 'data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1'
    ConfigFlags.Input.Files = [nightly+file]
    ConfigFlags.Input.isMC = True
    ConfigFlags.Output.HISTFileName = 'TrigMETMonitorOutput.root'
    
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    trigMETMonitorAcc = TrigMETMonConfig(ConfigFlags)
    cfg.merge(trigMETMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigMETMonitorAcc.getEventAlgo('TrigMETMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    cfg.run() #use cfg.run(20) to only run on first 20 events

