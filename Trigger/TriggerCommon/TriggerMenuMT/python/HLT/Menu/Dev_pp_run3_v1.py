# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],

from . import MC_pp_run3_v1 as mc_menu
from .SignatureDicts import ChainStore
from ..Config.Utility.ChainDefInMenu import ChainProp

# this is not the best option, due to flake violation, this list has to be changed when some groups are removed

from .Physics_pp_run3_v1 import (PhysicsStream,
                                                                 SingleMuonGroup,
                                                                 MultiMuonGroup,
                                                                 SingleElectronGroup,
                                                                 MultiElectronGroup,
                                                                 SinglePhotonGroup,
                                                                 METGroup,
                                                                 SingleJetGroup,
                                                                 MultiJetGroup,
                                                                 SingleBjetGroup,
                                                                 MultiBjetGroup,
                                                                 SingleTauGroup,
                                                                 MultiTauGroup,
                                                                 TauBJetGroup,
                                                                 TauMETGroup,                        
                                                                 BphysicsGroup,
                                                                 BphysElectronGroup,
                                                                 EgammaMETGroup,
                                                                 EgammaMuonGroup,
                                                                 EgammaBjetGroup,
                                                                 MuonJetGroup,
                                                                 MuonMETGroup,
                                                                 EgammaJetGroup,
                                                                 MinBiasGroup,
                                                                 PrimaryLegGroup,
                                                                 PrimaryPhIGroup,
                                                                 PrimaryL1MuGroup,
                                                                 SupportGroup,
                                                                 SupportLegGroup,
                                                                 SupportPhIGroup,
                                                                 TagAndProbeLegGroup,
                                                                 UnconvTrkGroup,
                                                                 METPhaseIStreamersGroup,
                                                                 EOFTLALegGroup,
                                                                 LegacyTopoGroup,
                                                                 Topo2Group,
                                                                 Topo3Group,
                                                                 EOFL1MuGroup,
                                                                 EOFBPhysL1MuGroup,
                                                                 EOFBeeLegGroup,
                                                                 )

DevGroup = ['Development']

def getDevSignatures():
    chains = ChainStore()
    chains['Muon'] = [


        #-- nscan ATR-19376, TODO: to be moved to physics once rated
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_J50', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_J50', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_J50', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_XE40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_XE40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_XE40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan10_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),

        #ATR-26727 - low mass Drell-Yan triggers
        ChainProp(name='HLT_2mu4_7invmAA9_L12MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_11invmAA60_L12MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),


        ChainProp(name='HLT_mu6_ivarmedium_L1MU5VF', groups=DevGroup+SingleMuonGroup),

        # ATR-22782 4muon
        ChainProp(name='HLT_2mu4_L1BPH-7M11-25DR99-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+DevGroup),
        ChainProp(name='HLT_2mu4_ivarloose_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+DevGroup),
        ChainProp(name='HLT_mu4_ivarloose_mu4_mu3noL1_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF', 'MU3VF', 'FSNOSEED'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+DevGroup),

        # Test ID T&P
        ChainProp(name='HLT_mu14_idtp_L1MU8F', groups=SingleMuonGroup+SupportGroup, monGroups=['idMon:shifter','idMon:t0']),

        # ATR-19452
        ChainProp(name='HLT_2mu4_muonqual_L12MU3V',  groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_2mu6_muonqual_L12MU5VF', groups=DevGroup+MultiMuonGroup),

        #ATR-21003
        ChainProp(name='HLT_2mu14_l2io_L12MU8F', groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_2mu6_l2io_L12MU5VF', groups=DevGroup+MultiMuonGroup),
        
        # Test T&P dimuon
        ChainProp(name='HLT_mu24_mu6_L1MU14FCH', l1SeedThresholds=['MU14FCH','MU3V'], groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu6_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU3V'], groups=DevGroup+MultiMuonGroup),

        #ATR-21566, di-muon TLA
        ChainProp(name='HLT_mu10_PhysicsTLA_L1MU8F',   stream=['TLA'], groups=SingleMuonGroup+DevGroup),
        ChainProp(name='HLT_mu10_mu6_probe_PhysicsTLA_L1MU8F', stream=['TLA'],l1SeedThresholds=['MU8F','PROBEMU3V'], groups=MultiMuonGroup+DevGroup),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L12MU3V',  stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        ChainProp(name='HLT_2mu6_PhysicsTLA_L12MU5VF', stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        ChainProp(name='HLT_2mu10_PhysicsTLA_L12MU8F', stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        # di-muon TLA with L1TOPO
        ChainProp(name='HLT_mu6_mu4_PhysicsTLA_L1BPH-7M22-MU5VFMU3VF', l1SeedThresholds=['MU5VF','MU3VF'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L1BPH-7M22-0DR20-2MU3V', l1SeedThresholds=['MU3V'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),

        # ATR-22782, 4mu analysis
        ChainProp(name='HLT_mu4_ivarloose_mu4_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF','MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_mu4_ivarloose_2mu3noL1_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF','FSNOSEED'], stream=['BphysDelayed'], groups=MultiMuonGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Egamma'] = [
        # ElectronChains----------

        # electron forward triggers (keep this only for dev now)
        #ChainProp(name='HLT_e30_etcut_fwd_L1EM22VHI', groups=SingleElectronGroup),

        # ATR-27156 Phase-1
        # dnn chains
        ChainProp(name='HLT_e26_dnnloose_L1EM22VHI', groups=SupportLegGroup+SingleElectronGroup+['RATE:CPS_EM22VHI'], monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_dnnmedium_L1EM22VHI', groups=SupportLegGroup+SingleElectronGroup+['RATE:CPS_EM22VHI'], monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_dnntight_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_dnntight_ivarloose_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0']),
        ChainProp(name='HLT_e60_dnnmedium_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        # test 
        ChainProp(name='HLT_e5_etcut_L1eEM5' , groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),


        #ATR-22749
        ChainProp(name='HLT_2e5_lhvloose_nogsf_bBeeM6000_L12EM3', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+DevGroup),
        ChainProp(name='HLT_e9_lhvloose_e5_lhvloose_nogsf_bBeeM6000_L1BPH-0M9-EM7-EM5', l1SeedThresholds=['EM7','EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhvloose_nogsf_bBeeM6000_L1BPH-0DR3-EM7J15', l1SeedThresholds=['EM7'], stream=['BphysDelayed'], groups=BphysElectronGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e9_lhvloose_nogsf_bBeeM6000_L1BPH-0DR3-EM7J15', l1SeedThresholds=['EM7'], stream=['BphysDelayed'], groups=BphysElectronGroup+DevGroup+LegacyTopoGroup),

        #ATR-27201, noringer version for BeeX triggers
        ChainProp(name='HLT_e5_lhvloose_noringer_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['EM3'], stream=['BphysDelayed','express'], groups=PrimaryLegGroup+BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_noringer_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['EM3'], stream=['BphysDelayed','express'], groups=PrimaryLegGroup+BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_noringer_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=EOFBeeLegGroup+BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_noringer_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=EOFBeeLegGroup+BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_noringer_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_noringer_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_noringer_bBeeM6000_L1EM22VHI', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_noringer_bBeeM6000_L1EM22VHI', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_noringer_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_noringer_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),

        ChainProp(name='HLT_g20_loose_noiso_L1EM15VH', groups=SupportLegGroup+SinglePhotonGroup+['RATE:CPS_EM15VH']),

        # ATR-27096 - Test rates for LRT electron veryloose chain at high d0
        ChainProp(name='HLT_e30_lhvloose_nopix_lrttight_L1EM22VHI', l1SeedThresholds=['EM22VHI'], groups=SingleElectronGroup+DevGroup),
        ChainProp(name='HLT_e30_lhvloose_nopix_lrtxtight_L1EM22VHI', l1SeedThresholds=['EM22VHI'], groups=SingleElectronGroup+DevGroup),
        ChainProp(name='HLT_e30_lhvloose_nopix_lrtvxtight_L1EM22VHI', l1SeedThresholds=['EM22VHI'], groups=SingleElectronGroup+DevGroup),


        # Photon chains for TLA
        ChainProp(name='HLT_g35_loose_PhysicsTLA_L1EM22VHI',stream=['TLA'], groups=SinglePhotonGroup+DevGroup),

        # Alternative formulation of T&P chains with generic mass cut combohypotool
        # With & without 'probe' expression to check count consistency
        # ATR-24117

        # Jpsiee
        ChainProp(name='HLT_e5_lhtight_e9_etcut_probe_1invmAB5_L1JPSI-1M5-EM7', l1SeedThresholds=['EM3','PROBEEM7'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM3','PROBEEM12'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e9_lhtight_e4_etcut_probe_1invmAB5_L1JPSI-1M5-EM7', l1SeedThresholds=['EM7','PROBEEM3'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_etcut_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM12','PROBEEM3'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),

    ]

    chains['MET'] = [

        ChainProp(name='HLT_xe30_cell_L1XE30',       l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mht_L1XE30',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_L1XE30',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_trkmht_L1XE30',     l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_L1XE30',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_cssk_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_vssk_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfopufit_L1XE30',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_cvfpufit_L1XE30',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_em_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_pf_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        ChainProp(name='HLT_xe110_tc_em_L1XE50',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_mht_L1XE50',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_tcpufit_L1XE50',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_L1XE50',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_cssk_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_vssk_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        # Test chains to determine rate after calo-only preselection for tracking
        ChainProp(name='HLT_xe60_cell_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        # ATR-25509 Triggers needed to test nSigma=3
        ChainProp(name='HLT_xe30_pfopufit_sig30_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_sig30_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        # ATR-25574 MET NN 
        ChainProp(name='HLT_xe55_cell_xe90_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe55_cell_xe90_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=METGroup+DevGroup),
    ]


    chains['Jet'] = [

        # pflow jet chains without pile-up residual correction for calibration derivations and calibration cross-checks ATR-26827
        ChainProp(name='HLT_j0_perf_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # candidate jet TLA chains ATR-20395
        ChainProp(name='HLT_4j25_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=DevGroup+MultiJetGroup), # adding for study of EMTopo TLA fast b-tagging.
        ChainProp(name='HLT_j20_pf_ftf_presel4j85_PhysicsTLA_L13J50', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j20_pf_ftf_presel5j50_PhysicsTLA_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j20_pf_ftf_presel6j40_PhysicsTLA_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ## with calo fast-tag presel - so actually btag TLA ATR-23002
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2j25XX2j25b85_PhysicsTLA_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+DevGroup),

       

        #
        ChainProp(name='HLT_4j20c_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+DevGroup), #ATR-26012
        ChainProp(name='HLT_6j25c_L14J15', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],        groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_010jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_020jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_050jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        #
        ChainProp(name='HLT_6j25c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],        groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_010jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_020jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_050jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        ### PURE TEST CHAINS

        ChainProp(name='HLT_j0_FBDJNOSHARED10etXX20etXX34massXX50fbet_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j0_FBDJSHARED_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j60_j0_FBDJSHARED_L1J20', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),

        # Emerging Jets test chains ATR-21593

        # alternate emerging jet single-jet chain
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),


        # backup emerging jets chains to be used for rate refinement in enhanced bias reprocessing
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j175_0eta180_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        # end of emerging jets chains

        #####

        # Primary jet chains w/o preselection, for comparison
        ChainProp(name='HLT_2j250c_j120c_pf_ftf_L1J100',    l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup ),
        ChainProp(name='HLT_4j115_pf_ftf_L13J50', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_5j85_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j55c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j70_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_7j45_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_10j40_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        ChainProp(name='HLT_j0_HT1000_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),

        # CSSKPFlow
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=DevGroup+MultiJetGroup+LegacyTopoGroup),

        ##### End no-preselection

        # Prototyping RoI jet tracking
        ChainProp(name="HLT_j80_pf_ftf_preselj20_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name="HLT_j80_pf_ftf_preselj20b95_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name="HLT_j80_pf_ftf_preselj20b77_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),

        ChainProp(name="HLT_j80_roiftf_preselj20_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name="HLT_j80_95bdips_roiftf_preselj20_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name="HLT_j80_77bdips_roiftf_preselj20_L1J20", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        #
        ChainProp(name='HLT_2j20c_2j20c_85bdips_roiftf_presel4c20_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j25c_2j25c_85bdips_roiftf_presel4c25_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j30c_2j30c_85bdips_roiftf_presel4c30_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j35c_2j35c_85bdips_roiftf_presel4c35_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45c_2j35c_85bdips_roiftf_presel4c45_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        #
        ChainProp(name='HLT_2j25c_2j25c_85bdips_roiftf_presel4c25_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j20c_2j20c_85bdips_roiftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j30c_2j30c_85bdips_roiftf_presel4c30_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j35c_2j35c_85bdips_roiftf_presel4c35_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45c_2j35c_85bdips_roiftf_presel4c45_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        #
        # some specific tighter test chains at 20 and 25 GeV
        ChainProp(name='HLT_2j25c_2j25c_80bdips_roiftf_presel4c25_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j20c_2j20c_80bdips_roiftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=MultiJetGroup+DevGroup),




        # ATR-24720 Testing additions to Run 3 baseline menu
        # HT preselection studies
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel3j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4c40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240XX020jvt_pf_ftf_presel4c40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_presel4j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j50_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel5j25_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_presel5j25_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ###
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),


         #TLA+PEB test for jets ATR-21596, matching "multijet+PFlow" TLA chain in physics menu for cross-check of event size
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # HT preseleection tests
        ChainProp(name='HLT_j20_pf_ftf_preselcHT650_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_pf_ftf_preselcHT850_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # jet preselection
        ChainProp(name='HLT_j20_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # with HT leg at the HLT
        # + ht preselection
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # + jet preselection
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # no preselection
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT500XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),

        ### END PURE TEST CHAINS

        # ATR-24838 Large R L1J100 jet chains with jLJ L1 items (L1J100->L1jLJ140)
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j110_0eta200_emergingPTF0p1dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j110_0eta180_emergingPTF0p09dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_tracklessdR1p2_a10r_subjesIS_ftf_0eta200_L1jLJ140',    groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_tracklessdR1p2_a10r_subjesIS_ftf_0eta200_L1jLJ140',    groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        # Duplicated with old naming conventions only for validation
        ChainProp(name='HLT_j45_320eta490_L1J15p31ETA49', groups=DevGroup+SingleJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j220_320eta490_L1J75p31ETA49', groups=DevGroup+SingleJetGroup, l1SeedThresholds=['FSNOSEED'], monGroups=['jetMon:shifter']),

        ChainProp(name='HLT_j420_a10t_lcw_jes_35smcINF_L1SC111-CJ15', groups=DevGroup+SingleJetGroup+LegacyTopoGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10t_lcw_jes_35smcINF_L1J100', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1SC111-CJ15', groups=DevGroup+MultiJetGroup+LegacyTopoGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1jLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1gLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),

        ]


    chains['Bjet'] = [
        
        # single bjet pflow options, # changes according to ATR-23883
        ChainProp(name="HLT_j225_0eta290_bdl1d60_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
        ChainProp(name="HLT_j225_0eta290_bdl1d85_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),

        ChainProp(name='HLT_j275_0eta290_bdl1d70_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
        ChainProp(name='HLT_j275_0eta290_bdl1d77_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),

        ChainProp(name='HLT_j300_0eta290_bdl1d60_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
        ChainProp(name='HLT_j300_0eta290_bdl1d77_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),

        ChainProp(name='HLT_j360_0eta290_bdl1d60_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
        ChainProp(name='HLT_j360_0eta290_bdl1d70_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
        ChainProp(name='HLT_j360_0eta290_bdl1d85_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),

        # dl1d test chains
        ChainProp(name="HLT_j275_0eta290_020jvt_bdl1d60_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=DevGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d70_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=DevGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d77_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=DevGroup+SingleBjetGroup),

        ######################################################################################################################################################################################################################################################
        # TEST CHAINS WITH ROIFTF PRESEL
        #HH4b chains with b-jet preselections
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d85_pf_ftf_presel2c20XX2c20b90_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b90_L1J45p0ETA21_3J15p0ETA25',l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=DevGroup+MultiBjetGroup),
        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b90_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=DevGroup+MultiBjetGroup),


        # HH4b test chains
        ChainProp(name='HLT_j80c_j55c_j28c_j20c_SHARED_2j20c_bdl1d77_pf_ftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_j55c_j28c_j20c_SHARED_2j20c_bdl1d70_pf_ftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_j55c_j28c_j20c_SHARED_2j20c_bdl1d65_pf_ftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_j55c_j28c_j20c_SHARED_2j20c_bdl1d60_pf_ftf_presel4c20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),

        # 2b test chains
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d82_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d80_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),

        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d75_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d72_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d70_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d65_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d60_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),

        # 3b test chains
        # this one is in Physics.
        # ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d80_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d77_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d75_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d72_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d70_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryLegGroup+MultiBjetGroup),

        # 3b85 symmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d85_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        # 2b77 symmetric b-jet pt for VBFDelayed
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_preselc60XXc45XXc25XXc20_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=DevGroup+MultiBjetGroup),

        # Tests of potential TLA chains for cost/rate
        # ATR-23002 - b-jets
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_3j20_020jvt_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED']*2, groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup),

        # TLA btag ATR-23002
        ## dijet btag TLA
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj140_PhysicsTLA_L1J50', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj140_PhysicsTLA_L1J50_DETA20-J50J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj180_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=SingleBjetGroup+DevGroup),
        ## multijet btag TLA - HT190
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        # multijet btag TLA - MultiJet L1
        ChainProp(name='HLT_j140_j20_0eta290_boffperf_pf_ftf_preselj140XXj45_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ## with calo fast-tag presel
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2j25XX2j25b85_PhysicsTLA_L14J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),

        # Maintain consistency with old naming conventions for validation
        ChainProp(name='HLT_j45_0eta290_020jvt_pf_ftf_boffperf_L1J20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j45_0eta290_020jvt_pf_ftf_boffperf_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j200_0eta290_020jvt_pf_ftf_boffperf_preselj140_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j200_0eta290_020jvt_pf_ftf_boffperf_preselj140_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),

        # [ATR-26414] Extra GN1 b-tagger chains
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn180_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn185_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn180_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn185_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
       
        # Extra GN1 + DL1dbb chains
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn182bb96_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177bb96_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn175bb90_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
 
        # [ATR-26676] Extra Extra GN1 b-tagger chains + Dl1d "equivalents"
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn165_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn170_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn172_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn175_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),

        # Testing preselection with fastGN1 in place of fastDips
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn175_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d65_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d70_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d72_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d75_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d80_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
          
        # [ATR-27058] Add Extra GN1 + Fast GN1 chains
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn165_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn170_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn172_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn180_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn185_pf_ftf_presel2c20XX2c20bg85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),

        #ATR-27132
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25_cTAU20M_2cTAU12M', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=DevGroup+MultiBjetGroup),
    ]

    chains['Tau'] = [
        ChainProp(name="HLT_tau25_looseRNN_tracktwoMVA_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_looseRNN_tracktwoLLP_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_tightRNN_tracktwoMVA_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_tightRNN_tracktwoLLP_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau35_looseRNN_tracktwoMVA_L1TAU20IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau35_tightRNN_tracktwoMVA_L1TAU20IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau160_ptonly_L1TAU100", groups=SingleTauGroup),
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25",l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25",l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),


        # Asymmetric Tau triggers for HH->bbtautau ATR-22230
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup), 
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup), 
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup), 
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup), 
        # ATR-26852: New Asymmetric ditau triggers for HH->bbtautau
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),
        #ATR-27121
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1J45p0ETA21_3J15p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),#gianipez
        #ATR-27132
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR28-eTAU30MeTAU20M-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR32-eTAU30MeTAU20M-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_10DR32-eTAU30MeTAU20M-jJ55", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30MeTAU12M"         , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30MeTAU20M" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30MeTAU12M" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30MeTAU12M", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        #
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1J45p0ETA21_3J15p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),#gianipez
        #ATR-27132
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR28-eTAU30MeTAU20M-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR32-eTAU30MeTAU20M-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_10DR32-eTAU30MeTAU20M-jJ55", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30MeTAU12M"         , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30MeTAU20M" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30MeTAU12M" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30MeTAU12M", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
      

        # eta L1
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),

        # ---- jTAU and eTAU seeded chains to investigate cTAU performance
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1jTAU20",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1jTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1jTAU30",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1jTAU30M",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1jTAU30M",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30M",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # LRT tau dev ATR-23787
        # [ATR-26377] Remove NoHLTRepro groups in order to include chains in reprocessings to get cost/rate information
        ChainProp(name="HLT_tau25_looseRNN_trackLRT_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1TAU12IM", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_tightRNN_trackLRT_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1eTAU20", groups=DevGroup),
        ChainProp(name="HLT_tau80_mediumRNN_trackLRT_L1TAU60", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_mediumRNN_trackLRT_L1eTAU80", groups=DevGroup),
        ChainProp(name="HLT_tau160_mediumRNN_trackLRT_L1TAU100", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau160_mediumRNN_trackLRT_L1eTAU140", groups=DevGroup),
        ChainProp(name="HLT_tau25_idperf_trackLRT_L1TAU12IM", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_idperf_tracktwoLLP_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau80_idperf_trackLRT_L1TAU60", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau160_idperf_trackLRT_L1TAU100", groups=DevGroup, monGroups=['tauMon:t0']), 

        
    ]

    chains['Bphysics'] = [
        #ATR-21003; default dimuon and Bmumux chains from Run2; l2io validation; should not be moved to Physics
        ChainProp(name='HLT_2mu4_noL2Comb_bJpsimumu_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_mu6_noL2Comb_mu4_noL2Comb_bJpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BpmumuKp_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BsmumuPhi_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_LbPqKm_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),

        #ATR-27341; R(D*) measurement
        ChainProp(name='HLT_2mu4_bBmux_BdmuDstarX_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+DevGroup),
        ChainProp(name='HLT_2mu4_bBmux_BdmuDstarX_L1BPH-7M11-25DR99-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+DevGroup),
        ChainProp(name='HLT_2mu4_bBmux_BdmuDstarX_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+DevGroup),
        ChainProp(name='HLT_mu6_mu4_bBmux_BdmuDstarX_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+DevGroup),
        ChainProp(name='HLT_mu11_mu6_bBmux_BdmuDstarX_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+DevGroup),
    ]

    chains['Combined'] = [

        # Test chains for muon + jet/MET merging/aligning
        ChainProp(name='HLT_mu6_xe30_mht_L1XE30', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=MuonMETGroup),
        ChainProp(name='HLT_mu6_j45_nojcalib_L1J20', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=MuonJetGroup),

        # mu-tag & tau-probe triggers for LLP (ATR-23150)
        ChainProp(name='HLT_mu26_ivarmedium_tau100_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau100_mediumRNN_tracktwoLLP_probe_03dRAB_L1EM22VHI', l1SeedThresholds=['EM22VHI','PROBETAU60'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleElectronGroup),

        # tau + jet and tau + photon tag and probe (ATR-24031)
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***
        #ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBETAU8','FSNOSEED'], groups=TagAndProbeLegGroup+TauJetGroup),
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***


        #Photon+MET new NN without isolation ATR-26410
        ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe60_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe70_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe80_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe60_pfopufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe60_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe70_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe80_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        #ChainProp(name='HLT_g50_tight_xe40_cell_xe60_pfopufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe70_pfopufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        #Photon+MET NN ATR-25574
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_xe70_nn_80mTAD_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        #Photon+MET ATR-25384
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),

        #added for debugging: ATR-24946
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'], groups=DevGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe40_tcpufit_xe40_pfopufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+3*['FSNOSEED'], groups=DevGroup+EgammaMETGroup),


        # Tests of potential TLA chains for cost/rate
        # ATR-19317 - dijet+ISR
        ChainProp(name='HLT_g35_loose_3j25_pf_ftf_L1EM22VHI',          l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),
        ChainProp(name='HLT_g35_medium_3j25_pf_ftf_L1EM22VHI',         l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),
        ChainProp(name='HLT_g35_tight_3j25_0eta290_boffperf_pf_ftf_L1EM22VHI', l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),


        # high-mu AFP
        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=['PS:Online']+MinBiasGroup+SupportLegGroup),

        # Test PEB chains for AFP (single/di-lepton-seeded, can be prescaled)
        # ATR-23946
        ChainProp(name='HLT_noalg_AFPPEB_L1EM22VHI', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1MU14FCH', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L12MU5VF', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=MinBiasGroup),

        #ATR-23156 will be superseeded by ATR-24698
        ChainProp(name='HLT_mu4_j20_0eta290_boffperf_pf_ftf_dRAB03_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup),
        ChainProp(name='HLT_mu4_j35_0eta290_boffperf_pf_ftf_dRAB03_L1BTAG-MU3VjJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j45_0eta290_boffperf_pf_ftf_dRAB03_L1BTAG-MU5VFjJ50', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),

        #ATR-24698
        #L1Topo
        ChainProp(name='HLT_mu4_j35_0eta290_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu4_j45_0eta290_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j60_0eta290_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j100_0eta290_boffperf_pf_ftf_dRAB04_L1BTAG-MU5VFjJ90', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SingleBjetGroup+Topo2Group),
        #jFEX
        ChainProp(name='HLT_mu4_j20_0eta290_boffperf_pf_ftf_dRAB04_L1MU3V_jJ30', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup), # added temporarily
        ChainProp(name='HLT_mu4_j35_0eta290_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup),
        ChainProp(name='HLT_mu4_j45_0eta290_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup),
        ChainProp(name='HLT_mu6_j60_0eta290_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup),
        ChainProp(name='HLT_mu6_j100_0eta290_boffperf_pf_ftf_dRAB04_L1MU5VF_jJ90', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SingleBjetGroup),
        #Legacy
        ChainProp(name='HLT_mu4_j20_0eta290_boffperf_pf_ftf_dRAB04_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup), # added temporarily
        # other muon in jet chains moved to Physicis menu

        # ATR-21596 
        # Muon+HT Test chains for PEB
        ChainProp(name='HLT_mu6_probe_j20_pf_ftf_JetPEBPhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU5VF','FSNOSEED'], stream=['TLAJetPEB'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu10_probe_j20_pf_ftf_JetPEBPhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU8F','FSNOSEED'], stream=['TLAJetPEB'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),

        # Phase-I egamma+X chains with muon L1
        ChainProp(name='HLT_e9_lhvloose_L1eEM5_mu20_mu8noL1_L1MU14FCH', l1SeedThresholds=['eEM5','MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),

        # Maintain consistency with old naming conventions for validation
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_pf_ftf_boffperf_L1EM22VHI', l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=DevGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter']),


        # b+2τ test chains for HH->bbττ:
        # versions with 1b and 3j1b requirements
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j20c_020jvt_bgn185_pf_ftf_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM', 'FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM', 'FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_4j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM']+2*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_4j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM']+2*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),


        # b+τ test chains for HH->bbττ
        # this is a scan in
        #   τ pt: [ 20 , 25 , 35 ]
        #   τ ID OP: [ mediumRNN , tightRNN ]
        #   b-tagging OP: [ 70% , 77% , 85% ]
        # ATR-27327
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn170_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn177_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        ChainProp(name='HLT_tau35_tightRNN_tracktwoMVA_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['TAU8']+5*['FSNOSEED'], stream=['VBFDelayed'], groups=DevGroup+TauBJetGroup),
        
        # Tau+MET: ATR-27252
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eTAU60_2cTAU20M_jXE80', stream=[PhysicsStream], l1SeedThresholds=['eTAU60','cTAU20M','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),  


    ]

    chains['Beamspot'] = [
        ChainProp(name='HLT_beamspot_allTE_trkfast_BeamSpotPEB_L1J15',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot']),        
    ]

    chains['MinBias'] = [

    ]

    chains['Calib'] = [
        #ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_ANY', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=['RATE:ALFACalibration','BW:Detector']+LowMuGroup),
        # Calib Chains
        ChainProp(name='HLT_larpsallem_L1EM3', groups=SingleElectronGroup+SupportLegGroup),
    ]

    chains['Streaming'] = [

        # ATR-24037
        ChainProp(name='HLT_noalg_L1jXEPerf100',     l1SeedThresholds=['FSNOSEED'], groups=METPhaseIStreamersGroup),

    ]

    chains['Monitor'] = [
       ChainProp(name='HLT_l1topodebug_legacy_L1All', l1SeedThresholds=['FSNOSEED'], stream=['L1TopoMismatches'], groups=['PS:Online', 'PS:NoHLTRepro', 'RATE:Monitoring', 'BW:Other']),
    ]

    chains['UnconventionalTracking'] = [
        #Isolated High Pt Trigger Test chain for optimisation studies
        ChainProp(name='HLT_isotrk50_L1XE50', groups=UnconvTrkGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),


        ChainProp(name='HLT_fslrt0_L1J100', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L14J15', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L1XE50', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L1All',  groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),

        # TrigVSI
        ChainProp(name='HLT_fsvsi0_L1XE50',         groups=PrimaryLegGroup+UnconvTrkGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_dispvtx0_loose_L1XE50', groups=PrimaryLegGroup+UnconvTrkGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        # TrigVSI, ATR-25722
        ChainProp(name='HLT_fsvsi0_L1All', groups=PrimaryLegGroup+UnconvTrkGroup+DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
    ]

    return chains

def setupMenu(menu_name):

    chains = mc_menu.setupMenu(menu_name)

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevSignatures().items():
        chains[sig] += chainsInSig

    return chains
