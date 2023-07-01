#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@file TgcRawDataMonitorAlgorithm.py
@author M.Aoki
@date 2019-10-03
@brief Python configuration for the Run III AthenaMonitoring package for TGC 
'''

def TgcRawDataMonitoringConfig(inputFlags):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    result = ComponentAccumulator()

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(inputFlags))
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TgcRawDataMonitorCfg')

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(inputFlags))

    tgcRawDataMonitorTool = CompFactory.TgcRawDataMonitorTool("TgcRawDataMonitorTool")

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    tgcRawDataMonAlg = helper.addAlgorithm(CompFactory.TgcRawDataMonitorAlgorithm,'TgcRawDataMonAlg',
                                           TrackExtrapolator = extrapolator,
                                           TgcRawDataMonitorTool = tgcRawDataMonitorTool,
                                           MuonSelectionTool = result.popToolsAndMerge(MuonSelectionToolCfg(inputFlags, 
                                                                                                            MuQuality=1,
                                                                                                            MaxEta=2.7)),
                                           doExpressProcessing = inputFlags.Common.doExpressProcessing )

    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge( TrackingGeometryCondAlgCfg(inputFlags ) )

    doGapByGapHitOcc = True
    doGapByGapEffMap = False
    doHitResiduals = True

    tgcRawDataMonAlg.FillGapByGapHistograms = (doGapByGapHitOcc or doGapByGapEffMap or doHitResiduals)

    tgcRawDataMonAlg.GRLTool = ""

    tgcRawDataMonAlg.UseOnlyCombinedMuons = True
    tgcRawDataMonAlg.UseOnlyMuidCoStacoMuons = True
    tgcRawDataMonAlg.UseMuonSelectorTool = True

    tgcRawDataMonAlg.MonitorThresholdPatterns = True
    tgcRawDataMonAlg.ThrPatternList = "MU3V,MU3VF,MU3VC,MU5VF,MU8F,MU8FC,MU9VF,MU9VFC,MU8VF,MU8VFC,MU14FCH,MU14FCHR,MU15VFCH,MU15VFCHR,MU18VFCH,MU10BOM,MU12BOM,MU8FH,MU20VFC,MU12FCH,MU4BOM,MU4BO,MU10BO,MU14EOF,MU8EOF,MU3EOF,"

    tgcRawDataMonAlg.TagAndProbe = True
    tgcRawDataMonAlg.TagAndProbeZmumu = False

    if not inputFlags.DQ.triggerDataAvailable:
        tgcRawDataMonAlg.MuonRoIContainerName = ''
        tgcRawDataMonAlg.MuonRoIContainerBCm2Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCm1Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCp1Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCp2Name = ''
        tgcRawDataMonAlg.TagAndProbe = False
        tgcRawDataMonAlg.MonitorThresholdPatterns = False
        tgcRawDataMonAlg.ThrPatternList = ''
        tgcRawDataMonAlg.MonitorTriggerMultiplicity = False
        tgcRawDataMonAlg.CtpDecisionMoniorList = ''
        tgcRawDataMonAlg.UseMuonSelectorTool = False

    if inputFlags.Trigger.EDMVersion < 3: # Run2 and before
        tgcRawDataMonAlg.MuonRoIContainerName = ''
        tgcRawDataMonAlg.MuonRoIContainerBCm2Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCm1Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCp1Name = ''
        tgcRawDataMonAlg.MuonRoIContainerBCp2Name = ''
        tgcRawDataMonAlg.MuRoIThresholdPatternsKey = ''
        tgcRawDataMonAlg.TgcCoinDataContainerNextNextBCName = ''
        tgcRawDataMonAlg.MonitorThresholdPatterns = False

    if inputFlags.Input.Format is Format.BS or 'TGC_MeasurementsAllBCs' in inputFlags.Input.Collections:
        tgcRawDataMonAlg.AnaTgcPrd=True
    if inputFlags.Input.Format is Format.BS or ('TrigT1CoinDataCollection' in inputFlags.Input.Collections and
                                                'TrigT1CoinDataCollectionNextBC' in inputFlags.Input.Collections and
                                                'TrigT1CoinDataCollectionPriorBC' in inputFlags.Input.Collections ):
        tgcRawDataMonAlg.AnaTgcCoin=True
    
    mainDir = 'Muon/MuonRawDataMonitoring/TGC/'
    import math

    myGroup = helper.addGroup(tgcRawDataMonAlg,'TgcRawDataMonitor',mainDir)

    ################################################################################################################
    commonPath = 'EventInfo/'
    myGroupCommon = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor_Common', mainDir)
    myGroupCommon.defineHistogram('mon_lb;TgcMon_LuminosityBlock',title='Luminosity Block;Luminosity Block;Number of events',
                                  path=commonPath,type='TH1F',xbins=10,xmin=0,xmax=10,opt='kAddBinsDynamically', merge='merge')
    myGroupCommon.defineHistogram('mon_bcid;TgcMon_BCID',title='BCID;BCID;Number of events',
                                  path=commonPath,type='TH1F',xbins=4096,xmin=-0.5,xmax=4095.5)
    myGroupCommon.defineHistogram('mon_pileup;TgcMon_Pileup',title='Pileup;Pileup;Number of events',
                                  path=commonPath,type='TH1F',xbins=101,xmin=-0.5,xmax=100.5)
    myGroupCommon.defineHistogram('mon_primvtx_z;TgcMon_PrimaryVertexZ',title='Primary Vertex Z;Primary Vertex Z [mm];Number of events',
                                  path=commonPath,type='TH1F',xbins=100,xmin=-200.,xmax=200.)
    ################################################################################################################
    muonPath = 'Muon/'
    myGroupMuon = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor_Muon', mainDir)
    myGroupMuon.defineHistogram('oflmuon_num;TgcMon_OfflineMuon_num',title='OfflineMuon_num;OfflineMuon num per event;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_muonType;TgcMon_OfflineMuon_muonType',title='OfflineMuon_muonType;OfflineMuon muonType;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_author;TgcMon_OfflineMuon_author',title='OfflineMuon_author;OfflineMuon author;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_quality;TgcMon_OfflineMuon_quality',title='OfflineMuon_quality;OfflineMuon quality;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_pt;TgcMon_OfflineMuon_Pt',title='OfflineMuon_Pt;OfflineMuon pT [GeV];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=0,xmax=100)
    myGroupMuon.defineHistogram('oflmuon_eta,oflmuon_phi;TgcMon_OfflineMuon_EtaVsPhi',title='OfflineMuon_EtaVsPhi;OfflineMuon Eta;OfflineMuon Phi',
                                  path=muonPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroupMuon.defineHistogram('oflmuon_pvdz;TgcMon_OfflineMuon_pvdz',title='OfflineMuon_PVDZ;Delta Z [mm];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=-100,xmax=100)
    myGroupMuon.defineHistogram('oflmuon_pvdca;TgcMon_OfflineMuon_pvdca',title='OfflineMuon_PVDCA;DCA [mm];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=-5,xmax=5)

    myGroupMuon.defineHistogram('oflmuon_probe_num;TgcMon_OfflineMuon_Probe_num',title='OfflineMuon_Probe_num;OfflineMuon num per event;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_probe_muonType;TgcMon_OfflineMuon_Probe_muonType',title='OfflineMuon_Probe_muonType;OfflineMuon muonType;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_probe_author;TgcMon_OfflineMuon_Probe_author',title='OfflineMuon_Probe_author;OfflineMuon author;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_probe_quality;TgcMon_OfflineMuon_Probe_quality',title='OfflineMuon_Probe_quality;OfflineMuon quality;Number of events',
                                  path=muonPath,type='TH1F',xbins=21,xmin=-0.5,xmax=20.5)
    myGroupMuon.defineHistogram('oflmuon_probe_pt;TgcMon_OfflineMuon_Probe_Pt',title='OfflineMuon_Probe_Pt;OfflineMuon pT [GeV];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=0,xmax=100)
    myGroupMuon.defineHistogram('oflmuon_probe_eta,oflmuon_probe_phi;TgcMon_OfflineMuon_Probe_EtaVsPhi',title='OfflineMuon_Probe_EtaVsPhi;OfflineMuon Eta;OfflineMuon Phi',
                                  path=muonPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroupMuon.defineHistogram('oflmuon_probe_pvdz;TgcMon_OfflineMuon_Probe_pvdz',title='OfflineMuon_Probe_PVDZ;Delta Z [mm];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=-100,xmax=100)
    myGroupMuon.defineHistogram('oflmuon_probe_pvdca;TgcMon_OfflineMuon_Probe_pvdca',title='OfflineMuon_Probe_PVDCA;DCA [mm];Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=-5,xmax=5)

    myGroupMuon.defineHistogram('oflmuon_deltaR;TgcMon_OfflineMuon_DeltaR',title='OfflineMuon_DeltaR;OfflineMuon DeltaR;Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=0,xmax=5)
    myGroupMuon.defineHistogram('oflmuon_deltaR_roi;TgcMon_OfflineMuon_DeltaR_to_RoIs',title='OfflineMuon_DeltaR_to_RoIs;OfflineMuon DeltaR to RoIs;Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=0,xmax=5)
    myGroupMuon.defineHistogram('oflmuon_deltaR_hlt;TgcMon_OfflineMuon_DeltaR_to_HLTs',title='OfflineMuon_DeltaR_to_HLTs;OfflineMuon DeltaR to HLTs;Number of events',
                                  path=muonPath,type='TH1F',xbins=100,xmin=0,xmax=5)


    ################################################################################################################
    trigThrPatternsPath = 'TrigPatterns/'
    for monTrig in tgcRawDataMonAlg.ThrPatternList.split(','):
        if monTrig == "":continue
        monTrigGroup = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor'+monTrig, mainDir)

        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+';L1Item_MuonRoI_Evt_Eta_'+monTrig,title='L1Item_MuonRoI_Evt_Eta_'+monTrig+';MuonRoI Eta;Entries',
                                     path=trigThrPatternsPath,type='TH1F',xbins=100,xmin=-2.5,xmax=2.5)
        monTrigGroup.defineHistogram('l1item_roi_phi_rpc_'+monTrig+';L1Item_MuonRoI_Evt_Phi_RPC_'+monTrig,title='L1Item_MuonRoI_Evt_Phi_RPC_'+monTrig+';MuonRoI Phi;Entries',
                                     path=trigThrPatternsPath,type='TH1F',xbins=32,xmin=-math.pi,xmax=math.pi)
        monTrigGroup.defineHistogram('l1item_roi_phi_tgc_'+monTrig+';L1Item_MuonRoI_Evt_Phi_TGC_'+monTrig,title='L1Item_MuonRoI_Evt_Phi_TGC_'+monTrig+';MuonRoI Phi;Entries',
                                     path=trigThrPatternsPath,type='TH1F',xbins=48,xmin=-math.pi,xmax=math.pi)

        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_phi_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsPhi_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsPhi_'+monTrig+';MuonRoI Eta;MuonRoI Phi',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_thrNumber_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsThrNumber_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsThrNumber_'+monTrig+';MuonRoI Eta;MuonRoI Threshold Number',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=15,ymin=0.5,ymax=15.5)
        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_ismorecand_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsIsMoreCand_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsIsMoreCand_'+monTrig+';MuonRoI Eta;MuonRoI IsMoreCand Flag',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_bw3coin_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsBW3Coin_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsBW3Coin_'+monTrig+';MuonRoI Eta;MuonRoI BW3Coin Flag',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_innercoin_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsInnerCoin_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsInnerCoin_'+monTrig+';MuonRoI Eta;MuonRoI InnerCoin Flag',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('l1item_roi_eta_'+monTrig+',l1item_roi_goodmf_'+monTrig+';L1Item_MuonRoI_Evt_EtaVsGoodMF_'+monTrig,title='L1Item_MuonRoI_Evt_EtaVsGoodMF_'+monTrig+';MuonRoI Eta;MuonRoI GoodMF Flag',
                                     path=trigThrPatternsPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=3,ymin=-1.5,ymax=1.5)

        monTrigGroup.defineHistogram('l1item_roi_roiNumber_'+monTrig+',l1item_roi_sector_'+monTrig+';L1Item_MuonRoI_Evt_SectorVsRoINumber_Barrel_'+monTrig,title='L1Item_MuonRoI SectorVsRoINumber Barrel '+monTrig+';RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                                     type='TH2F',cutmask='l1item_roi_barrel_'+monTrig,path=trigThrPatternsPath,xbins=29,xmin=-0.5,xmax=28.5,ybins=65,ymin=-32.5,ymax=32.5)
        monTrigGroup.defineHistogram('l1item_roi_roiNumber_'+monTrig+',l1item_roi_sector_'+monTrig+';L1Item_MuonRoI_Evt_SectorVsRoINumber_Endcap_'+monTrig,title='L1Item_MuonRoI SectorVsRoINumber Endcap '+monTrig+';RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                                     type='TH2F',cutmask='l1item_roi_endcap_'+monTrig,path=trigThrPatternsPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
        monTrigGroup.defineHistogram('l1item_roi_roiNumber_'+monTrig+',l1item_roi_sector_'+monTrig+';L1Item_MuonRoI_Evt_SectorVsRoINumber_Forward_'+monTrig,title='L1Item_MuonRoI SectorVsRoINumber Forward '+monTrig+';RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                                     type='TH2F',cutmask='l1item_roi_forward_'+monTrig,path=trigThrPatternsPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_barrel_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Barrel_sideA_'+monTrig,title='L1Item_MuonRoI PhiVsLB Barrel sideA '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideA_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=32,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_barrel_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Barrel_sideC_'+monTrig,title='L1Item_MuonRoI PhiVsLB Barrel sideC '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideC_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=32,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_endcap_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Endcap_sideA_'+monTrig,title='L1Item_MuonRoI PhiVsLB Endcap sideA '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideA_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_endcap_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Endcap_sideC_'+monTrig,title='L1Item_MuonRoI PhiVsLB Endcap sideC '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideC_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_forward_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Forward_sideA_'+monTrig,title='L1Item_MuonRoI PhiVsLB Forward sideA '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideA_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
        monTrigGroup.defineHistogram('lumiBlock_l1item_'+monTrig+',l1item_roi_phi_forward_'+monTrig+';L1Item_MuonRoI_Evt_PhiVsLB_Forward_sideC_'+monTrig,title='L1Item_MuonRoI PhiVsLB Forward sideC '+monTrig+';Luminosity block;MuonRoI Phi',type='TH2F',
                                     cutmask='l1item_roi_sideC_'+monTrig,path=trigThrPatternsPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_pt_rpc_l1item_'+monTrig+';L1Item_MuonRoI_Eff_Pt_RPC_'+monTrig,title='L1Item_MuonRoI_Eff_Pt_RPC_'+monTrig+';Offline muon pT [GeV];Efficiency',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=50,xmin=0,xmax=50)
        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_pt_tgc_l1item_'+monTrig+';L1Item_MuonRoI_Eff_Pt_TGC_'+monTrig,title='L1Item_MuonRoI_Eff_Pt_TGC_'+monTrig+';Offline muon pT [GeV];Efficiency',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=50,xmin=0,xmax=50)
        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_phi_rpc_l1item_'+monTrig+';L1Item_MuonRoI_Eff_Phi_RPC_'+monTrig,title='L1Item_MuonRoI_Eff_Phi_RPC_'+monTrig+';Offline muon phi [rad.];Efficiency',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=32,xmin=-math.pi,xmax=math.pi)
        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_phi_tgc_l1item_'+monTrig+';L1Item_MuonRoI_Eff_Phi_TGC_'+monTrig,title='L1Item_MuonRoI_Eff_Phi_TGC_'+monTrig+';Offline muon phi [rad.];Efficiency',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_eta_l1item_'+monTrig+';L1Item_MuonRoI_Eff_Eta_'+monTrig,title='L1Item_MuonRoI_Eff_Eta_'+monTrig+';Offline muon eta;Efficiency',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=100,xmin=-2.5,xmax=2.5)
        monTrigGroup.defineHistogram('muon_passed_l1item_'+monTrig+',muon_eta_l1item_'+monTrig+',muon_phi_l1item_'+monTrig+';L1Item_MuonRoI_Eff_EtaVsPhi_'+monTrig,title='L1Item_MuonRoI_Eff_EtaVsPhi_'+monTrig+';Offline muon eta; Offline muon phi',
                                type='TEfficiency',path=trigThrPatternsPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)


    ################################################################################################################
    trigMultiPath = 'TrigMultiplicity/'
    for monTrig in tgcRawDataMonAlg.CtpDecisionMoniorList.split(';'):
        tmp = monTrig.split(',')[0]
        objname = tmp.replace('Tit:','')
        if objname == "":continue
        monTrigGroup = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor'+objname, mainDir)

        monTrigGroup.defineHistogram('ctpMultiplicity;'+objname+'_ctpMultiplicity',title=objname+' ctpMultiplicity;Ctp Output Multiplicity;Number of events',
                                path=trigMultiPath,xbins=11,xmin=-0.5,xmax=10.5)
        monTrigGroup.defineHistogram('rawMultiplicity;'+objname+'_rawMultiplicity',title=objname+' rawMultiplicity;Raw Input Multiplicity;Number of events',
                                path=trigMultiPath,xbins=11,xmin=-0.5,xmax=10.5)
        monTrigGroup.defineHistogram('countDiff;'+objname+'_L1CountDiff',title=objname+' L1CountDiff;Event-by-event N(CTP out)-N(CTP in);Number of events',
                                path=trigMultiPath,xbins=11,xmin=-5.5,xmax=5.5)

        monTrigGroup.defineHistogram('roiMatching_CTPin,roiMatching_CTPout;'+objname+'_RoiMatching',title=objname+'_RoiMatching;Input;Output',
                                path=trigMultiPath,type='TH2F',xbins=2,xmin=-0.5,xmax=1.5,ybins=2,ymin=-0.5,ymax=1.5,xlabels=['NotExist','Exist'],ylabels=['NotExist','Exist'])

        monTrigGroup.defineHistogram('Eta,Phi;'+objname+'_L1Count_EtaVsPhi_inOk_outOk',title=objname+'_L1Count_EtaVsPhi_inOk_outOk;MuonRoI Eta;MuonRoI Phi',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
        monTrigGroup.defineHistogram('Eta,dRmin;'+objname+'_L1Count_EtaVsdRmin_inOk_outOk',title=objname+'_L1Count_EtaVsdRmin_inOk_outOk;MuonRoI Eta;Closest dR between Muon RoIs',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=11,ymin=-0.1,ymax=1.0)
        monTrigGroup.defineHistogram('dRmin;'+objname+'_roi_dRmin_inOk_outOk',title=objname+'_roi_dRmin_inOk_outOk;Closest dR between Muon RoIs;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=11,xmin=-0.1,xmax=1.0)
        monTrigGroup.defineHistogram('ThrNum;'+objname+'_roi_ThrNum_inOk_outOk',title=objname+'_roi_ThrNum_inOk_outOk;Threshold number (positive for barrel, negative for endcap);Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=22,xmin=-15.5,xmax=6.5)
        monTrigGroup.defineHistogram('Charge;'+objname+'_roi_Charge_inOk_outOk',title=objname+'_roi_Charge_inOk_outOk;Muon charge;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('BW3Coin;'+objname+'_roi_BW3Coin_inOk_outOk',title=objname+'_roi_BW3Coin_inOk_outOk;TGC BW3Coin flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('InnerCoin;'+objname+'_roi_InnerCoin_inOk_outOk',title=objname+'_roi_InnerCoin_inOk_outOk;TGC InnerCoin flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('GoodMF;'+objname+'_roi_GoodMF_inOk_outOk',title=objname+'_roi_GoodMF_inOk_outOk;TGC GoodMF flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('IsMoreCandInRoI;'+objname+'_roi_IsMoreCandInRoI_inOk_outOk',title=objname+'_roi_IsMoreCandInRoI_inOk_outOk;RPC IsMoreCandInRoI flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('PhiOverlap;'+objname+'_roi_PhiOverlap_inOk_outOk',title=objname+'_roi_PhiOverlap_inOk_outOk;PhiOverlap flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('EtaOverlap;'+objname+'_roi_EtaOverlap_inOk_outOk',title=objname+'_roi_EtaOverlap_inOk_outOk;EtaOverlap flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,PhiOverlap;'+objname+'_roi_dRminVsPhiOverlap_inOk_outOk',
                                title=objname+'_roi_dRminVsPhiOverlap_inOk_outOk;Closest dR between Muon RoIs;PhiOverlap flag',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('isVetoed;'+objname+'_roi_isVetoed_inOk_outOk',title=objname+'_roi_isVetoed_inOk_outOk;isVetoed flag;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,isVetoed;'+objname+'_roi_dRminVsisVetoed_inOk_outOk',
                                title=objname+'_roi_dRminVsisVetoed_inOk_outOk;Closest dR between Muon RoIs;isVetoed flag',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,isVetoed;'+objname+'_roi_EtaVsisVetoed_inOk_outOk',
                                title=objname+'_roi_EtaVsisVetoed_inOk_outOk;MuonRoI Eta;isVetoed flag',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,pTdiff;'+objname+'_L1Count_EtaVspTdiff_inOk_outOk',title=objname+'_L1Count_EtaVspTdiff_inOk_outOk;MuonRoI Eta;pT difference',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=31,ymin=-15.5,ymax=15.5)
        monTrigGroup.defineHistogram('pTdiff;'+objname+'_roi_pTdiff_inOk_outOk',title=objname+'_roi_pTdiff_inOk_outOk;pT difference;Number of events',
                                cutmask='inOk_outOk',path=trigMultiPath,type='TH1F',xbins=31,xmin=-15.5,xmax=15.5)


        monTrigGroup.defineHistogram('Eta,Phi;'+objname+'_L1Count_EtaVsPhi_inOk_outNg',title=objname+'_L1Count_EtaVsPhi_inOk_outNg;MuonRoI Eta;MuonRoI Phi',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
        monTrigGroup.defineHistogram('Eta,dRmin;'+objname+'_L1Count_EtaVsdRmin_inOk_outNg',title=objname+'_L1Count_EtaVsdRmin_inOk_outNg;MuonRoI Eta;Closest dR between Muon RoIs',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=11,ymin=-0.1,ymax=1.0)
        monTrigGroup.defineHistogram('dRmin;'+objname+'_roi_dRmin_inOk_outNg',title=objname+'_roi_dRmin_inOk_outNg;Closest dR between Muon RoIs;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=11,xmin=-0.1,xmax=1.0)
        monTrigGroup.defineHistogram('ThrNum;'+objname+'_roi_ThrNum_inOk_outNg',title=objname+'_roi_ThrNum_inOk_outNg;Threshold number (positive for barrel, negative for endcap);Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=22,xmin=-15.5,xmax=6.5)
        monTrigGroup.defineHistogram('Charge;'+objname+'_roi_Charge_inOk_outNg',title=objname+'_roi_Charge_inOk_outNg;Muon charge;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('BW3Coin;'+objname+'_roi_BW3Coin_inOk_outNg',title=objname+'_roi_BW3Coin_inOk_outNg;TGC BW3Coin flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('InnerCoin;'+objname+'_roi_InnerCoin_inOk_outNg',title=objname+'_roi_InnerCoin_inOk_outNg;TGC InnerCoin flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('GoodMF;'+objname+'_roi_GoodMF_inOk_outNg',title=objname+'_roi_GoodMF_inOk_outNg;TGC GoodMF flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('IsMoreCandInRoI;'+objname+'_roi_IsMoreCandInRoI_inOk_outNg',title=objname+'_roi_IsMoreCandInRoI_inOk_outNg;RPC IsMoreCandInRoI flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('PhiOverlap;'+objname+'_roi_PhiOverlap_inOk_outNg',title=objname+'_roi_PhiOverlap_inOk_outNg;PhiOverlap flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('EtaOverlap;'+objname+'_roi_EtaOverlap_inOk_outNg',title=objname+'_roi_EtaOverlap_inOk_outNg;EtaOverlap flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,PhiOverlap;'+objname+'_roi_dRminVsPhiOverlap_inOk_outNg',
                                title=objname+'_roi_dRminVsPhiOverlap_inOk_outNg;Closest dR between Muon RoIs;PhiOverlap flag',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('isVetoed;'+objname+'_roi_isVetoed_inOk_outNg',title=objname+'_roi_isVetoed_inOk_outNg;isVetoed flag;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,isVetoed;'+objname+'_roi_dRminVsisVetoed_inOk_outNg',
                                title=objname+'_roi_dRminVsisVetoed_inOk_outNg;Closest dR between Muon RoIs;isVetoed flag',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,isVetoed;'+objname+'_roi_EtaVsisVetoed_inOk_outNg',
                                title=objname+'_roi_EtaVsisVetoed_inOk_outNg;MuonRoI Eta;isVetoed flag',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,pTdiff;'+objname+'_L1Count_EtaVspTdiff_inOk_outNg',title=objname+'_L1Count_EtaVspTdiff_inOk_outNg;MuonRoI Eta;pT difference',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=31,ymin=-15.5,ymax=15.5)
        monTrigGroup.defineHistogram('pTdiff;'+objname+'_roi_pTdiff_inOk_outNg',title=objname+'_roi_pTdiff_inOk_outNg;pT difference;Number of events',
                                cutmask='inOk_outNg',path=trigMultiPath,type='TH1F',xbins=31,xmin=-15.5,xmax=15.5)


        monTrigGroup.defineHistogram('Eta,Phi;'+objname+'_L1Count_EtaVsPhi_inNg_outOk',title=objname+'_L1Count_EtaVsPhi_inNg_outOk;MuonRoI Eta;MuonRoI Phi',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
        monTrigGroup.defineHistogram('Eta,dRmin;'+objname+'_L1Count_EtaVsdRmin_inNg_outOk',title=objname+'_L1Count_EtaVsdRmin_inNg_outOk;MuonRoI Eta;Closest dR between Muon RoIs',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=11,ymin=-0.1,ymax=1.0)
        monTrigGroup.defineHistogram('dRmin;'+objname+'_roi_dRmin_inNg_outOk',title=objname+'_roi_dRmin_inNg_outOk;Closest dR between Muon RoIs;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=11,xmin=-0.1,xmax=1.0)
        monTrigGroup.defineHistogram('ThrNum;'+objname+'_roi_ThrNum_inNg_outOk',title=objname+'_roi_ThrNum_inNg_outOk;Threshold number (positive for barrel, negative for endcap);Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=22,xmin=-15.5,xmax=6.5)
        monTrigGroup.defineHistogram('Charge;'+objname+'_roi_Charge_inNg_outOk',title=objname+'_roi_Charge_inNg_outOk;Muon charge;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('BW3Coin;'+objname+'_roi_BW3Coin_inNg_outOk',title=objname+'_roi_BW3Coin_inNg_outOk;TGC BW3Coin flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('InnerCoin;'+objname+'_roi_InnerCoin_inNg_outOk',title=objname+'_roi_InnerCoin_inNg_outOk;TGC InnerCoin flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('GoodMF;'+objname+'_roi_GoodMF_inNg_outOk',title=objname+'_roi_GoodMF_inNg_outOk;TGC GoodMF flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('IsMoreCandInRoI;'+objname+'_roi_IsMoreCandInRoI_inNg_outOk',title=objname+'_roi_IsMoreCandInRoI_inNg_outOk;RPC IsMoreCandInRoI flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('PhiOverlap;'+objname+'_roi_PhiOverlap_inNg_outOk',title=objname+'_roi_PhiOverlap_inNg_outOk;PhiOverlap flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=3,xmin=-1.5,xmax=1.5)
        monTrigGroup.defineHistogram('EtaOverlap;'+objname+'_roi_EtaOverlap_inNg_outOk',title=objname+'_roi_EtaOverlap_inNg_outOk;EtaOverlap flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,PhiOverlap;'+objname+'_roi_dRminVsPhiOverlap_inNg_outOk',
                                title=objname+'_roi_dRminVsPhiOverlap_inNg_outOk;Closest dR between Muon RoIs;PhiOverlap flag',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=3,ymin=-1.5,ymax=1.5)
        monTrigGroup.defineHistogram('isVetoed;'+objname+'_roi_isVetoed_inNg_outOk',title=objname+'_roi_isVetoed_inNg_outOk;isVetoed flag;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=2,xmin=-0.5,xmax=1.5)
        monTrigGroup.defineHistogram('dRmin,isVetoed;'+objname+'_roi_dRminVsisVetoed_inNg_outOk',
                                title=objname+'_roi_dRminVsisVetoed_inNg_outOk;Closest dR between Muon RoIs;isVetoed flag',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=11,xmin=-0.1,xmax=1.0,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,isVetoed;'+objname+'_roi_EtaVsisVetoed_inNg_outOk',
                                title=objname+'_roi_EtaVsisVetoed_inNg_outOk;MuonRoI Eta;isVetoed flag',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=2,ymin=-0.5,ymax=1.5)
        monTrigGroup.defineHistogram('Eta,pTdiff;'+objname+'_L1Count_EtaVspTdiff_inNg_outOk',title=objname+'_L1Count_EtaVspTdiff_inNg_outOk;MuonRoI Eta;pT difference',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=31,ymin=-15.5,ymax=15.5)
        monTrigGroup.defineHistogram('pTdiff;'+objname+'_roi_pTdiff_inNg_outOk',title=objname+'_roi_pTdiff_inNg_outOk;pT difference;Number of events',
                                cutmask='inNg_outOk',path=trigMultiPath,type='TH1F',xbins=31,xmin=-15.5,xmax=15.5)



    ################################################################################################################
    trigPath = 'Trig/'

    myGroup.defineHistogram('roi_sector,roi_timing;MuonRoI_Evt_SectorVsTiming_Barrel',type='TH2F',
                            title='MuonRoI Sector vs Timing Barrel;MuonRoI Trigger Sector +1  (>0 for A, <0 for C);Timing',cutmask='roi_barrel',path=trigPath,
                            xbins=65,xmin=-32.5,xmax=32.5,ybins=5,ymin=-2.5,ymax=2.5,ylabels=['-2BC','-1BC','CurrBC','+1BC','+2BC'])
    myGroup.defineHistogram('roi_sector,roi_timing;MuonRoI_Evt_SectorVsTiming_Endcap',type='TH2F',
                            title='MuonRoI Sector vs Timing Endcap;MuonRoI Trigger Sector +1  (>0 for A, <0 for C);Timing',cutmask='roi_endcap',path=trigPath,
                            xbins=97,xmin=-48.5,xmax=48.5,ybins=5,ymin=-2.5,ymax=2.5,ylabels=['-2BC','-1BC','CurrBC','+1BC','+2BC'])
    myGroup.defineHistogram('roi_sector,roi_timing;MuonRoI_Evt_SectorVsTiming_Forward',type='TH2F',
                            title='MuonRoI Sector vs Timing Forward;MuonRoI Trigger Sector +1  (>0 for A, <0 for C);Timing',cutmask='roi_forward',path=trigPath,
                            xbins=49,xmin=-24.5,xmax=24.5,ybins=5,ymin=-2.5,ymax=2.5,ylabels=['-2BC','-1BC','CurrBC','+1BC','+2BC'])

    myGroup.defineHistogram('roi_thr,roi_sector;MuonRoI_Evt_SectorVsThreshold_Barrel',title='MuonRoI SectorVsThreshold Barrel;Threshold;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_barrel',path=trigPath,xbins=7,xmin=-0.5,xmax=6.5,ybins=65,ymin=-32.5,ymax=32.5)
    myGroup.defineHistogram('roi_thr,roi_sector;MuonRoI_Evt_SectorVsThreshold_Endcap',title='MuonRoI SectorVsThreshold Endcap;Threshold;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=16,xmin=-0.5,xmax=15.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_thr,roi_sector;MuonRoI_Evt_SectorVsThreshold_Forward',title='MuonRoI SectorVsThreshold Forward;Threshold;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=16,xmin=-0.5,xmax=15.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_roiNumber,roi_sector;MuonRoI_Evt_SectorVsRoINumber_Barrel',title='MuonRoI SectorVsRoINumber Barrel;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_barrel',path=trigPath,xbins=29,xmin=-0.5,xmax=28.5,ybins=65,ymin=-32.5,ymax=32.5)
    myGroup.defineHistogram('roi_roiNumber,roi_sector;MuonRoI_Evt_SectorVsRoINumber_Endcap',title='MuonRoI SectorVsRoINumber Endcap;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_roiNumber,roi_sector;MuonRoI_Evt_SectorVsRoINumber_Forward',title='MuonRoI SectorVsRoINumber Forward;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_roiNumber,roi_sector_wBW3Coin;MuonRoI_Evt_SectorVsRoINumber_wBW3Coin_Endcap',title='MuonRoI SectorVsRoINumber wBW3Coin Endcap;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_roiNumber,roi_sector_wBW3Coin;MuonRoI_Evt_SectorVsRoINumber_wBW3Coin_Forward',title='MuonRoI SectorVsRoINumber wBW3Coin Forward;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_roiNumber,roi_sector_wInnerCoin;MuonRoI_Evt_SectorVsRoINumber_wInnerCoin_Endcap',title='MuonRoI SectorVsRoINumber wInnerCoin Endcap;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_roiNumber,roi_sector_wInnerCoin;MuonRoI_Evt_SectorVsRoINumber_wInnerCoin_Forward',title='MuonRoI SectorVsRoINumber wInnerCoin Forward;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_charge;MuonRoI_Evt_Charge',title='MuonRoI Charge;Charge;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=3,xmin=-1.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passCharge,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_Charge',title='MuonRoI_Eff_EtaVsPhi_Charge;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passCharge,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_Charge',title='MuonRoI_Eff_Pt_TGC_Charge;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passCharge,muon_eta4gev;MuonRoI_Eff_Eta_Charge',title='MuonRoI_Eff_Eta_Charge;Offline muon eta; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passCharge,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_Charge',title='MuonRoI_Eff_Phi_TGC_Charge;Offline muon phi; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_Charge_Positive',type='TH2F',
                            title='MuonRoI Eta vs Phi Charge Positive;MuonRoI Eta;MuonRoI Phi',cutmask='roi_posCharge',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_Charge_Negative',type='TH2F',
                            title='MuonRoI Eta vs Phi Charge Negative;MuonRoI Eta;MuonRoI Phi',cutmask='roi_negCharge',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_charge;MuonRoI_Evt_ChargeVsThr',type='TH2F',
                            title='MuonRoI Charge vs Thr;MuonRoI Thresholds;MuonRoI Charge',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=3,ymin=-1.5,ymax=1.5)

    #########################
    myGroup.defineHistogram('roi_innercoin,roi_eta_tgc;MuonRoI_Eff_Eta_wInnerCoin_ThrAll',title='MuonRoI Eff Eta wInnerCoin ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_innercoin,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wInnerCoin_ThrAll',title='MuonRoI Eff Phi TGC wInnerCoin ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_innercoin,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wInnerCoin_ThrAll',title='MuonRoI Eff Eta vs Phi wInnerCoin ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_innercoin,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wInnerCoin_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wInnerCoin Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_innercoin,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wInnerCoin_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wInnerCoin Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_innveto,roi_eta_tgc;MuonRoI_Eff_Eta_wInnerCoinVeto_ThrAll',title='MuonRoI Eff Eta wInnerCoinVeto ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_innveto,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wInnerCoinVeto_ThrAll',title='MuonRoI Eff Phi TGC wInnerCoinVeto ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_innveto,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wInnerCoinVeto_ThrAll',title='MuonRoI Eff Eta vs Phi wInnerCoinVeto ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_innveto,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wInnerCoinVeto_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wInnerCoinVeto Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_innveto,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wInnerCoinVeto_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wInnerCoinVeto Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_bw3coin,roi_eta_tgc;MuonRoI_Eff_Eta_wBW3Coin_ThrAll',title='MuonRoI Eff Eta wBW3Coin ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_bw3coin,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wBW3Coin_ThrAll',title='MuonRoI Eff Phi TGC wBW3Coin ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_bw3coin,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wBW3Coin_ThrAll',title='MuonRoI Eff Eta vs Phi wBW3Coin ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_bw3coin,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBW3Coin_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBW3Coin Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_bw3coin,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBW3Coin_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBW3Coin Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_bw3coinveto,roi_eta_tgc;MuonRoI_Eff_Eta_wBW3CoinVeto_ThrAll',title='MuonRoI Eff Eta wBW3CoinVeto ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_bw3coinveto,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wBW3CoinVeto_ThrAll',title='MuonRoI Eff Phi TGC wBW3CoinVeto ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_bw3coinveto,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wBW3CoinVeto_ThrAll',title='MuonRoI Eff Eta vs Phi wBW3CoinVeto ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_bw3coinveto,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBW3CoinVeto_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBW3CoinVeto Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_bw3coinveto,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBW3CoinVeto_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBW3CoinVeto Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_goodmf,roi_eta_tgc;MuonRoI_Eff_Eta_wGoodMF_ThrAll',title='MuonRoI Eff Eta wGoodMF ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_goodmf,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wGoodMF_ThrAll',title='MuonRoI Eff Phi TGC wGoodMF ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_goodmf,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wGoodMF_ThrAll',title='MuonRoI Eff Eta vs Phi wGoodMF ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_goodmf,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wGoodMF_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wGoodMF Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_goodmf,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wGoodMF_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wGoodMF Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    myGroup.defineHistogram('roi_badmf,roi_eta_tgc;MuonRoI_Eff_Eta_wBadMF_ThrAll',title='MuonRoI Eff Eta wBadMF ThrAll;MuonRoI Eta;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('roi_badmf,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wBadMF_ThrAll',title='MuonRoI Eff Phi TGC wBadMF ThrAll;MuonRoI Phi;Efficiency',
                            type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_badmf,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wBadMF_ThrAll',title='MuonRoI Eff Eta vs Phi wBadMF ThrAll;MuonRoI Eta;MuonRoI Phi',
                            type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_badmf,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBadMF_Endcap_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBadMF Endcap ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_endcap',path=trigPath,xbins=149,xmin=-0.5,xmax=148.5,ybins=97,ymin=-48.5,ymax=48.5)
    myGroup.defineHistogram('roi_badmf,roi_roiNumber,roi_sector;MuonRoI_Eff_SectorVsRoINumber_wBadMF_Forward_ThrAll',title='MuonRoI Eff SectorVsRoINumber wBadMF Forward ThrAll;RoI Number;MuonRoI Trigger Sector +1  (>0 for A, <0 for C)',
                            type='TEfficiency',cutmask='roi_forward',path=trigPath,xbins=65,xmin=-0.5,xmax=64.5,ybins=49,ymin=-24.5,ymax=24.5)

    #########################

    myGroup.defineHistogram('roi_bw3coin;MuonRoI_Evt_BW3Coin',title='MuonRoI BW3Coin Flag;BW3Coin Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passBW3Coin,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_BW3Coin',title='MuonRoI_Eff_EtaVsPhi_BW3Coin;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passBW3Coin,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_BW3Coin',title='MuonRoI_Eff_Pt_TGC_BW3Coin;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passBW3Coin,muon_eta4gev;MuonRoI_Eff_Eta_BW3Coin',title='MuonRoI_Eff_Eta_BW3Coin;Offline muon eta; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passBW3Coin,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_BW3Coin',title='MuonRoI_Eff_Phi_TGC_BW3Coin;Offline muon phi; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_BW3Coin',type='TH2F',
                            title='MuonRoI Eta vs Phi BW3Coin;MuonRoI Eta;MuonRoI Phi',cutmask='roi_bw3coin',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_bw3coin;MuonRoI_Evt_ThrVsBW3Coin',type='TH2F',
                            title='MuonRoI Thr vs BW3Coin;MuonRoI Thresholds;MuonRoI BW3Coin',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_bw3coinveto;MuonRoI_Evt_BW3CoinVeto',title='MuonRoI BW3CoinVeto Flag;BW3CoinVeto Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passBW3CoinVeto,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_BW3CoinVeto',title='MuonRoI_Eff_EtaVsPhi_BW3CoinVeto;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passBW3CoinVeto,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_BW3CoinVeto',title='MuonRoI_Eff_Pt_TGC_BW3CoinVeto;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passBW3CoinVeto,muon_eta4gev;MuonRoI_Eff_Eta_BW3CoinVeto',title='MuonRoI_Eff_Eta_BW3CoinVeto;Offline muon eta; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passBW3CoinVeto,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_BW3CoinVeto',title='MuonRoI_Eff_Phi_TGC_BW3CoinVeto;Offline muon phi; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_BW3CoinVeto',type='TH2F',
                            title='MuonRoI Eta vs Phi BW3CoinVeto;MuonRoI Eta;MuonRoI Phi',cutmask='roi_bw3coinveto',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_bw3coinveto;MuonRoI_Evt_ThrVsBW3CoinVeto',type='TH2F',
                            title='MuonRoI Thr vs BW3CoinVeto;MuonRoI Thresholds;MuonRoI BW3CoinVeto',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_innercoin;MuonRoI_Evt_InnerCoin',title='MuonRoI InnerCoin Flag;InnerCoin Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_InnerCoin',title='MuonRoI_Eff_EtaVsPhi_InnerCoin;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_InnerCoin',title='MuonRoI_Eff_Pt_TGC_InnerCoin;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_eta4gev;MuonRoI_Eff_Eta_InnerCoin',title='MuonRoI_Eff_Eta_InnerCoin;Offline muon Eta;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_InnerCoin',title='MuonRoI_Eff_Phi_TGC_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p05eta1p3;MuonRoI_Eff_Phi_TGC_1p05eta1p3_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p05eta1p3A;MuonRoI_Eff_Phi_TGC_1p05eta1p3A_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3A_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p05eta1p3C;MuonRoI_Eff_Phi_TGC_1p05eta1p3C_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3C_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p3eta2p4;MuonRoI_Eff_Phi_TGC_1p3eta2p4_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p3eta2p4A;MuonRoI_Eff_Phi_TGC_1p3eta2p4A_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4A_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoin,muon_phi4gev_1p3eta2p4C;MuonRoI_Eff_Phi_TGC_1p3eta2p4C_InnerCoin',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4C_InnerCoin;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_InnerCoin',type='TH2F',
                            title='MuonRoI Eta vs Phi InnerCoin;MuonRoI Eta;MuonRoI Phi',cutmask='roi_innercoin',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_innercoin;MuonRoI_Evt_ThrVsInnerCoin',type='TH2F',
                            title='MuonRoI Thr vs InnerCoin;MuonRoI Thresholds;MuonRoI InnerCoin',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_lumiBlock,roi_phi_sideA;MuonRoI_Evt_PhiVsLB_wInnerCoinEtaUpTo1p3_sideA',title='MuonRoI PhiVsLB wInnerCoinEtaUpTo1p3 sideA;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_wInnerCoinEtaUpTo1p3',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_sideA;MuonRoI_Evt_PhiVsLB_wInnerCoinEtaBeyond1p3_sideA',title='MuonRoI PhiVsLB wInnerCoinEtaBeyond1p3 sideA;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_wInnerCoinEtaBeyond1p3',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_sideC;MuonRoI_Evt_PhiVsLB_wInnerCoinEtaUpTo1p3_sideC',title='MuonRoI PhiVsLB wInnerCoinEtaUpTo1p3 sideC;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_wInnerCoinEtaUpTo1p3',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_sideC;MuonRoI_Evt_PhiVsLB_wInnerCoinEtaBeyond1p3_sideC',title='MuonRoI PhiVsLB wInnerCoinEtaBeyond1p3 sideC;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_wInnerCoinEtaBeyond1p3',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

    myGroup.defineHistogram('roi_lumiBlock,roi_phi_wBW3Coin_sideA;MuonRoI_Evt_PhiVsLB_wBW3Coin_Endcap_sideA',title='MuonRoI PhiVsLB wBW3Coin Endcap sideA;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_wBW3Coin_sideA;MuonRoI_Evt_PhiVsLB_wBW3Coin_Forward_sideA',title='MuonRoI PhiVsLB wBW3Coin Forward sideA;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_wBW3Coin_sideC;MuonRoI_Evt_PhiVsLB_wBW3Coin_Endcap_sideC',title='MuonRoI PhiVsLB wBW3Coin Endcap sideC;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_endcap',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_wBW3Coin_sideC;MuonRoI_Evt_PhiVsLB_wBW3Coin_Forward_sideC',title='MuonRoI PhiVsLB wBW3Coin Forward sideC;Luminosity block;MuonRoI Phi',
                            type='TH2F',cutmask='roi_forward',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

    myGroup.defineHistogram('roi_innveto;MuonRoI_Evt_InnerCoinVeto',title='MuonRoI InnerCoinVeto Flag;InnerCoinVeto Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_InnerCoinVeto',title='MuonRoI_Eff_EtaVsPhi_InnerCoinVeto;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_InnerCoinVeto',title='MuonRoI_Eff_Pt_TGC_InnerCoinVeto;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_eta4gev;MuonRoI_Eff_Eta_InnerCoinVeto',title='MuonRoI_Eff_Eta_InnerCoinVeto;Offline muon Eta;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p05eta1p3;MuonRoI_Eff_Phi_TGC_1p05eta1p3_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p05eta1p3A;MuonRoI_Eff_Phi_TGC_1p05eta1p3A_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3A_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p05eta1p3C;MuonRoI_Eff_Phi_TGC_1p05eta1p3C_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p05eta1p3C_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p3eta2p4;MuonRoI_Eff_Phi_TGC_1p3eta2p4_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p3eta2p4A;MuonRoI_Eff_Phi_TGC_1p3eta2p4A_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4A_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('muon_l1passInnerCoinVeto,muon_phi4gev_1p3eta2p4C;MuonRoI_Eff_Phi_TGC_1p3eta2p4C_InnerCoinVeto',title='MuonRoI_Eff_Phi_TGC_1p3eta2p4C_InnerCoinVeto;Offline muon Phi;Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_InnerCoinVeto',type='TH2F',
                            title='MuonRoI Eta vs Phi InnerCoinVeto;MuonRoI Eta;MuonRoI Phi',cutmask='roi_innveto',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_innveto;MuonRoI_Evt_ThrVsInnerCoinVeto',type='TH2F',
                            title='MuonRoI Thr vs InnerCoinVeto;MuonRoI Thresholds;MuonRoI InnerCoinVeto',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_goodmf;MuonRoI_Evt_GoodMF',title='MuonRoI GoodMF Flag;GoodMF Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passGoodMF,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_GoodMF',title='MuonRoI_Eff_EtaVsPhi_GoodMF;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passGoodMF,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_GoodMF',title='MuonRoI_Eff_Pt_TGC_GoodMF;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passGoodMF,muon_eta4gev;MuonRoI_Eff_Eta_GoodMF',title='MuonRoI_Eff_Eta_GoodMF;Offline muon eta; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passGoodMF,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_GoodMF',title='MuonRoI_Eff_Phi_TGC_GoodMF;Offline muon phi; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_GoodMF',type='TH2F',
                            title='MuonRoI Eta vs Phi GoodMF;MuonRoI Eta;MuonRoI Phi',cutmask='roi_goodmf',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_goodmf;MuonRoI_Evt_ThrVsGoodMF',type='TH2F',
                            title='MuonRoI Thr vs GoodMF;MuonRoI Thresholds;MuonRoI GoodMF',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_badmf;MuonRoI_Evt_BadMF',title='MuonRoI BadMF Flag;BadMF Flag;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('muon_l1passBadMF,muon_eta4gev,muon_phi4gev;MuonRoI_Eff_EtaVsPhi_BadMF',title='MuonRoI_Eff_EtaVsPhi_BadMF;Offline muon eta; Offline muon phi',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('muon_l1passBadMF,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_BadMF',title='MuonRoI_Eff_Pt_TGC_BadMF;Offline muon pT [GeV];Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
    myGroup.defineHistogram('muon_l1passBadMF,muon_eta4gev;MuonRoI_Eff_Eta_BadMF',title='MuonRoI_Eff_Eta_BadMF;Offline muon eta; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
    myGroup.defineHistogram('muon_l1passBadMF,muon_phi4gev_tgc;MuonRoI_Eff_Phi_TGC_BadMF',title='MuonRoI_Eff_Phi_TGC_BadMF;Offline muon phi; Efficiency',
                            cutmask='muon_l1passThr1TGC',type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_BadMF',type='TH2F',
                            title='MuonRoI Eta vs Phi BadMF;MuonRoI Eta;MuonRoI Phi',cutmask='roi_badmf',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_badmf;MuonRoI_Evt_ThrVsBadMF',type='TH2F',
                            title='MuonRoI Thr vs BadMF;MuonRoI Thresholds;MuonRoI BadMF',cutmask='roi_tgc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_ismorecand;MuonRoI_Evt_RpcIsMoreCandInRoI',title='MuonRoI RpcIsMoreCandInRoI Flag;RpcIsMoreCandInRoI Flag;Number of events',
                            cutmask='roi_rpc',path=trigPath,xbins=2,xmin=-0.5,xmax=1.5)
    myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_RpcIsMoreCandInRoI',type='TH2F',
                            title='MuonRoI Eta vs Phi RpcIsMoreCandInRoI;MuonRoI Eta;MuonRoI Phi',cutmask='roi_ismorecand',path=trigPath,
                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('roi_thr,roi_ismorecand;MuonRoI_Evt_ThrVsRpcIsMoreCandInRoI',type='TH2F',
                            title='MuonRoI Thr vs RpcIsMoreCandInRoI;MuonRoI Thresholds;MuonRoI RpcIsMoreCandInRoI',cutmask='roi_rpc',path=trigPath,
                            xbins=20,xmin=-0.5,xmax=19.5,ybins=2,ymin=-0.5,ymax=1.5)

    myGroup.defineHistogram('roi_thr;MuonRoI_Evt_Thresholds_RPC',title='MuonRoI Thresholds RPC;MuonRoI Threshold number;Number of events',
                            cutmask='roi_rpc',path=trigPath,xbins=20,xmin=-0.5,xmax=19.5)
    myGroup.defineHistogram('roi_thr;MuonRoI_Evt_Thresholds_TGC',title='MuonRoI Thresholds TGC;MuonRoI Threshold number;Number of events',
                            cutmask='roi_tgc',path=trigPath,xbins=20,xmin=-0.5,xmax=19.5)

    myGroup.defineHistogram('roi_lumiBlock,roi_phi_barrel;MuonRoI_Evt_PhiVsLB_Barrel_sideA',title='MuonRoI PhiVsLB Barrel sideA;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideA',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=32,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_barrel;MuonRoI_Evt_PhiVsLB_Barrel_sideC',title='MuonRoI PhiVsLB Barrel sideC;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideC',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=32,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

    myGroup.defineHistogram('roi_lumiBlock,roi_phi_endcap;MuonRoI_Evt_PhiVsLB_Endcap_sideA',title='MuonRoI PhiVsLB Endcap sideA;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideA',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_endcap;MuonRoI_Evt_PhiVsLB_Endcap_sideC',title='MuonRoI PhiVsLB Endcap sideC;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideC',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=48,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

    myGroup.defineHistogram('roi_lumiBlock,roi_phi_forward;MuonRoI_Evt_PhiVsLB_Forward_sideA',title='MuonRoI PhiVsLB Forward sideA;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideA',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')
    myGroup.defineHistogram('roi_lumiBlock,roi_phi_forward;MuonRoI_Evt_PhiVsLB_Forward_sideC',title='MuonRoI PhiVsLB Forward sideC;Luminosity block;MuonRoI Phi',type='TH2F',
                            cutmask='roi_sideC',path=trigPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=24,ymin=-math.pi,ymax=math.pi,opt='kAddBinsDynamically', merge='merge')

    
    for n in range(1,16):

        myGroup.defineHistogram('roi_eta_wInnerCoin;MuonRoI_Evt_Eta_wInnerCoin_Thr%02d' % n,title='MuonRoI Eta wInnerCoin Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_eta_wBW3Coin;MuonRoI_Evt_Eta_wBW3Coin_Thr%02d' % n,title='MuonRoI Eta wBW3Coin Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)

        myGroup.defineHistogram('roi_eta_wInnerCoinVeto;MuonRoI_Evt_Eta_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Eta wInnerCoinVeto Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_eta_wBW3CoinVeto;MuonRoI_Evt_Eta_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Eta wBW3CoinVeto Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)

        myGroup.defineHistogram('roi_phi_wInnerCoin;MuonRoI_Evt_Phi_TGC_wInnerCoin_Thr%02d' % n,title='MuonRoI Phi TGC wInnerCoin Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_phi_wBW3Coin;MuonRoI_Evt_Phi_TGC_wBW3Coin_Thr%02d' % n,title='MuonRoI Phi TGC wBW3Coin Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)

        myGroup.defineHistogram('roi_phi_wInnerCoinVeto;MuonRoI_Evt_Phi_TGC_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Phi TGC wInnerCoinVeto Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_phi_wBW3CoinVeto;MuonRoI_Evt_Phi_TGC_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Phi TGC wBW3CoinVeto Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)

        myGroup.defineHistogram('roi_eta_wInnerCoin,roi_phi_wInnerCoin;MuonRoI_Evt_EtaVsPhi_wInnerCoin_Thr%02d' % n,title='MuonRoI Eta vs Phi wInnerCoin Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi,type='TH2F')
        myGroup.defineHistogram('roi_eta_wBW3Coin,roi_phi_wBW3Coin;MuonRoI_Evt_EtaVsPhi_wBW3Coin_Thr%02d' % n,title='MuonRoI Eta vs Phi wBW3Coin Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi,type='TH2F')

        myGroup.defineHistogram('roi_eta_wInnerCoinVeto,roi_phi_wInnerCoinVeto;MuonRoI_Evt_EtaVsPhi_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Eta vs Phi wInnerCoinVeto Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi,type='TH2F')
        myGroup.defineHistogram('roi_eta_wBW3CoinVeto,roi_phi_wBW3CoinVeto;MuonRoI_Evt_EtaVsPhi_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Eta vs Phi wBW3CoinVeto Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi,type='TH2F')

        myGroup.defineHistogram('roi_eta;MuonRoI_Evt_Eta_Thr%02d' % n,title='MuonRoI Eta Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_eta_rpc;MuonRoI_Evt_Eta_RPC_Thr%02d' % n,title='MuonRoI Eta RPC Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_eta_tgc;MuonRoI_Evt_Eta_TGC_Thr%02d' % n,title='MuonRoI Eta TGC Thr%02d;MuonRoI Eta;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_phi_rpc;MuonRoI_Evt_Phi_RPC_Thr%02d' % n,title='MuonRoI Phi RPC Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=32,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_phi_tgc;MuonRoI_Evt_Phi_TGC_Thr%02d' % n,title='MuonRoI Phi TGC Thr%02d;MuonRoI Phi;Number of events' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_eta,roi_phi;MuonRoI_Evt_EtaVsPhi_Thr%02d' % n,type='TH2F',title='MuonRoI Eta vs Phi Thr%02d;MuonRoI Eta;MuonRoI Phi' % n,
                                cutmask='thrmask'+str(n),path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('roi_innercoin,roi_eta_tgc;MuonRoI_Eff_Eta_wInnerCoin_Thr%02d' % n,title='MuonRoI Eff Eta wInnerCoin Thr%02d;MuonRoI Eta;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_innercoin,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wInnerCoin_Thr%02d' % n,title='MuonRoI Eff Phi TGC wInnerCoin Thr%02d;MuonRoI Phi;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_innercoin,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wInnerCoin_Thr%02d' % n,title='MuonRoI Eff Eta vs Phi wInnerCoin Thr%02d;MuonRoI Eta;MuonRoI Phi' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('roi_innveto,roi_eta_tgc;MuonRoI_Eff_Eta_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Eff Eta wInnerCoinVeto Thr%02d;MuonRoI Eta;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_innveto,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Eff Phi TGC wInnerCoinVeto Thr%02d;MuonRoI Phi;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_innveto,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wInnerCoinVeto_Thr%02d' % n,title='MuonRoI Eff Eta vs Phi wInnerCoinVeto Thr%02d;MuonRoI Eta;MuonRoI Phi' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('roi_bw3coin,roi_eta_tgc;MuonRoI_Eff_Eta_wBW3Coin_Thr%02d' % n,title='MuonRoI Eff Eta wBW3Coin Thr%02d;MuonRoI Eta;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_bw3coin,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wBW3Coin_Thr%02d' % n,title='MuonRoI Eff Phi TGC wBW3Coin Thr%02d;MuonRoI Phi;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_bw3coin,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wBW3Coin_Thr%02d' % n,title='MuonRoI Eff Eta vs Phi wBW3Coin Thr%02d;MuonRoI Eta;MuonRoI Phi' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('roi_bw3coinveto,roi_eta_tgc;MuonRoI_Eff_Eta_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Eff Eta wBW3CoinVeto Thr%02d;MuonRoI Eta;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('roi_bw3coinveto,roi_phi_tgc;MuonRoI_Eff_Phi_TGC_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Eff Phi TGC wBW3CoinVeto Thr%02d;MuonRoI Phi;Efficiency' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('roi_bw3coinveto,roi_eta_tgc,roi_phi_tgc;MuonRoI_Eff_EtaVsPhi_wBW3CoinVeto_Thr%02d' % n,title='MuonRoI Eff Eta vs Phi wBW3CoinVeto Thr%02d;MuonRoI Eta;MuonRoI Phi' % n,
                                cutmask='thrmask'+str(n),type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)


        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_rpc;MuonRoI_Eff_Pt_RPC_Thr%02d' % (n,n),title='MuonRoI_Eff_Pt_RPC_Thr%02d;Offline muon pT [GeV];Efficiency' % n,
                                type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_Thr%02d' % (n,n),title='MuonRoI_Eff_Pt_TGC_Thr%02d;Offline muon pT [GeV];Efficiency' % n,
                                type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
        myGroup.defineHistogram('muon_l1passThr%d,muon_phi_rpc;MuonRoI_Eff_Phi_RPC_Thr%02d' % (n,n),title='MuonRoI_Eff_Phi_RPC_Thr%02d;Offline muon phi [rad.];Efficiency' % n,
                                type='TEfficiency',path=trigPath,xbins=32,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_phi_tgc;MuonRoI_Eff_Phi_TGC_Thr%02d' % (n,n),title='MuonRoI_Eff_Phi_TGC_Thr%02d;Offline muon phi [rad.];Efficiency' % n,
                                type='TEfficiency',path=trigPath,xbins=48,xmin=-math.pi,xmax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_eta;MuonRoI_Eff_Eta_Thr%02d' % (n,n),title='MuonRoI_Eff_Eta_Thr%02d;Offline muon eta;Efficiency' % n,
                                type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5)
        myGroup.defineHistogram('muon_l1passThr%d,muon_eta,muon_phi;MuonRoI_Eff_EtaVsPhi_Thr%02d' % (n,n),title='MuonRoI_Eff_EtaVsPhi_Thr%02d;Offline muon eta; Offline muon phi' % n,
                                type='TEfficiency',path=trigPath,xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcA;MuonRoI_Eff_PtVsPhi_TGC_Endcap_sideA_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_Endcap_sideA_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_endcap',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcA;MuonRoI_Eff_PtVsPhi_TGC_Forward_sideA_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_Forward_sideA_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_forward',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=24,ymin=-math.pi,ymax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcC;MuonRoI_Eff_PtVsPhi_TGC_Endcap_sideC_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_Endcap_sideC_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_endcap',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcC;MuonRoI_Eff_PtVsPhi_TGC_Forward_sideC_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_Forward_sideC_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_forward',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=24,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_ChargePos_Thr%02d' % (n,n),title='MuonRoI_Eff_Pt_TGC_ChargePos_Thr%02d;Offline muon pT [GeV];Efficiency' % n,
                                cutmask='muon_chargePos',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc;MuonRoI_Eff_Pt_TGC_ChargeNeg_Thr%02d' % (n,n),title='MuonRoI_Eff_Pt_TGC_ChargeNeg_Thr%02d;Offline muon pT [GeV];Efficiency' % n,
                                cutmask='muon_chargeNeg',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50)

        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcA;MuonRoI_Eff_PtVsPhi_TGC_ChargePos_sideA_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_ChargePos_sideA_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_chargePos',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcA;MuonRoI_Eff_PtVsPhi_TGC_ChargeNeg_sideA_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_ChargeNeg_sideA_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_chargeNeg',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcC;MuonRoI_Eff_PtVsPhi_TGC_ChargePos_sideC_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_ChargePos_sideC_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_chargePos',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)
        myGroup.defineHistogram('muon_l1passThr%d,muon_pt_tgc,muon_phi0gev_tgcC;MuonRoI_Eff_PtVsPhi_TGC_ChargeNeg_sideC_Thr%02d' % (n,n),title='MuonRoI_Eff_PtVsPhi_TGC_ChargeNeg_sideC_Thr%02d;Offline muon pT [GeV];Offline muon phi;Efficiency' % n,
                                cutmask='muon_chargeNeg',type='TEfficiency',path=trigPath,xbins=50,xmin=0,xmax=50,ybins=48,ymin=-math.pi,ymax=math.pi)



        
    ################################################################################################################
    label_ieta_sw = []
    label_iphi_sw = []
    label_glbl_index_sw = []
    for i in range(4) : label_ieta_sw.append('')
    for i in range(24) : label_iphi_sw.append('')
    for i in range(96) : label_glbl_index_sw.append('')
    for eta in range(2):
        for sector in range(0,16):
            for phi in range(0,25):
                for lay in range(1,3):
                    if eta==0 and sector!=0:continue
                    if eta==1 and phi>2:continue
                    if eta==1 and sector==7 and phi==0:continue
                    if eta==1 and sector==11 and phi==0:continue
                    if eta==1 and sector==15 and phi==0:continue
                    if eta==1 and sector%2!=1:continue
                    eta_index = eta * 2 + lay
                    if eta==0:
                        phi_index = phi
                    else:
                        phi_index = phi + int(sector / 2) + sector
                    glbl_index = eta * 24*2 + (phi_index - 1) * 2 + lay
                    if eta==0:
                        label_ieta_sw[eta_index-1] = '%s%dL%d' % ('F' if eta==0 else 'E',eta,lay)
                        label_iphi_sw[phi_index-1] = 'F%df%d' % (sector,phi)
                        label_glbl_index_sw[glbl_index-1] = 'F%df%d%s%dL%d' % (sector,phi,'F' if eta==0 else 'E',eta,lay)
                    else:
                        label_ieta_sw[eta_index-1] = '%s%dL%d' % ('F' if eta==0 else 'E',eta,lay)
                        if lay==1:
                            label_iphi_sw[phi_index-1] = 'E%df%d(%s)' % (sector,phi,label_iphi_sw[phi_index-1])
                        label_glbl_index_sw[glbl_index-1] = 'E%df%d%s%dL%d' % (sector,phi,'F' if eta==0 else 'E',eta,lay)

    label_ieta_bw1 = []
    label_iphi_bw1 = []
    label_glbl_index_bw1 = []
    label_ieta_bw23 = []
    label_iphi_bw23 = []
    label_glbl_index_bw23 = []
    for i in range(15) : label_ieta_bw1.append('')
    for i in range(48) : label_iphi_bw1.append('')
    for i in range(720) : label_glbl_index_bw1.append('')
    for i in range(12) : label_ieta_bw23.append('')
    for i in range(48) : label_iphi_bw23.append('')
    for i in range(576) : label_glbl_index_bw23.append('')
    for eta in range(6):
        for sector in range(1,13):
            for phi in range(4):
                for lay in range(1,4):
                    for station in range(1,3):
                        if station==1 and eta==5: continue
                        if station!=1 and lay==3: continue
                        nlay = 3 if station==1 else 2
                        eta_index = eta * nlay + lay
                        phi_index = (sector - 1) * 4 + phi + 1
                        glbl_index = eta * 48*nlay + (phi_index - 1) * nlay + lay
                        if station==1:
                            label_ieta_bw1[eta_index-1] = '%s%dL%d' % ('F' if eta==0 else 'E',eta,lay)
                            label_iphi_bw1[phi_index-1] = '%df%d' % (sector,phi)
                            label_glbl_index_bw1[glbl_index-1] = '%df%d%s%dL%d' % (sector,phi,'F' if eta==0 else 'E',eta,lay)
                        else:
                            label_ieta_bw23[eta_index-1] = '%s%dL%d' % ('F' if eta==0 else 'E',eta,lay)
                            label_iphi_bw23[phi_index-1] = '%df%d' % (sector,phi)
                            label_glbl_index_bw23[glbl_index-1] = '%df%d%s%dL%d' % (sector,phi,'F' if eta==0 else 'E',eta,lay)

    label_bw24sectors = []
    for i in range(25) : label_bw24sectors.append('')
    for side in ['A','C']:
        for sector in range(1,13):
            index = 13 + sector if side=='A' else 13 - sector
            label_bw24sectors[index-1] = '%s%02d' % (side,sector)


    myGroupHit = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor_TgcHit', mainDir)
    hitPath = 'Hit/'
    hitEffPath = 'HitEff/'
    myGroupHit.defineHistogram('hit_n;TgcPrd_nHits',title='TgcPrd_nHits;Number of hits;Number of events',
                               path=hitPath,xbins=100,xmin=0,xmax=1000,opt='kAddBinsDynamically', merge='merge')
    myGroupHit.defineHistogram('hit_bcmask;TgcPrd_Timing',title='TgcPrd_Timing;Timing (BC bit mask);Number of events',
                               path=hitPath,xbins=8,xmin=-0.5,xmax=7.5,xlabels=['Undef','Next-only','Curr-only','Curr&Next','Prev-only','Prev&Next','Prev&Curr','Prev&Curr&Next'])
    
    for opt in ['', '_wTrack']:
        titlesuffix=opt
        if opt=='':titlesuffix='_All'
        myGroupHit.defineHistogram('mon_lb,hit_bw24sectors'+opt+';TgcPrd_BWSectorsVsLB'+titlesuffix,
                                   title='BWSectorsVsLB'+titlesuffix+';Luminosity block;',type='TH2F',
                                   path=hitPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors,opt='kAddBinsDynamically', merge='merge')
        myGroupHit.defineHistogram('mon_lb,hit_bw24sectors_strip'+opt+';TgcPrd_BWSectorsVsLB_Strip'+titlesuffix,
                                   title='BWSectorsVsLB_Strip'+titlesuffix+';Luminosity block;',type='TH2F',
                                   path=hitPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors,opt='kAddBinsDynamically', merge='merge')
        myGroupHit.defineHistogram('mon_lb,hit_bw24sectors_wire'+opt+';TgcPrd_BWSectorsVsLB_Wire'+titlesuffix,
                                   title='BWSectorsVsLB_Wire'+titlesuffix+';Luminosity block;',type='TH2F',
                                   path=hitPath,xbins=100,xmin=-0.5,xmax=99.5,ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors,opt='kAddBinsDynamically', merge='merge')
        myGroupHit.defineHistogram('hit_bw24sectors'+opt+',hit_bwtiming'+opt+';TgcPrd_BWSectorsVsTiming'+titlesuffix,
                                   title='BWSectorsVsTiming'+titlesuffix+';;Timing',type='TH2F',
                                   path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
        myGroupHit.defineHistogram('hit_bw24sectors_strip'+opt+',hit_bwtiming_strip'+opt+';TgcPrd_BWSectorsVsTiming_Strip'+titlesuffix,
                                   title='BWSectorsVsTiming_Strip'+titlesuffix+';;Timing',type='TH2F',
                                   path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
        myGroupHit.defineHistogram('hit_bw24sectors_wire'+opt+',hit_bwtiming_wire'+opt+';TgcPrd_BWSectorsVsTiming_Wire'+titlesuffix,
                                   title='BWSectorsVsTiming_Wire'+titlesuffix+';;Timing',type='TH2F',
                                   path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
        myGroupHit.defineHistogram('hit_bwfulleta'+opt+',hit_bw24sectors'+opt+';TgcPrd_BWSectorsVsEta'+titlesuffix,
                                   title='BWSectorsVsEta'+titlesuffix+';;',type='TH2F',
                                   path=hitPath,xbins=6,xmin=-0.5,xmax=5.5,xlabels=['F','E1','E2','E3','E4','E5'],ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors)
        myGroupHit.defineHistogram('hit_bwfulleta_strip'+opt+',hit_bw24sectors_strip'+opt+';TgcPrd_BWSectorsVsEta_Strip'+titlesuffix,
                                   title='BWSectorsVsEta_Strip'+titlesuffix+';;',type='TH2F',
                                   path=hitPath,xbins=6,xmin=-0.5,xmax=5.5,xlabels=['F','E1','E2','E3','E4','E5'],ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors)
        myGroupHit.defineHistogram('hit_bwfulleta_wire'+opt+',hit_bw24sectors_wire'+opt+';TgcPrd_BWSectorsVsEta_Wire'+titlesuffix,
                                   title='BWSectorsVsEta_Wire'+titlesuffix+';;',type='TH2F',
                                   path=hitPath,xbins=6,xmin=-0.5,xmax=5.5,xlabels=['F','E1','E2','E3','E4','E5'],ybins=25,ymin=-12.5,ymax=12.5,ylabels=label_bw24sectors)



    myGroupHit.defineHistogram('hit_bcmask_bw24sectors_All,hit_bcmask_for_bw24sectors_All;TgcPrd_BWSectorsVsBCMask_All',
                               title='BWSectorsVsBCMask_All;;',type='TH2F',
                               path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=8,ymin=-0.5,ymax=7.5,ylabels=['Undef','Next-only','Curr-only','Curr&Next','Prev-only','Prev&Next','Prev&Curr','Prev&Curr&Next'])
    myGroupHit.defineHistogram('hit_bcmask_bw24sectors_Wire,hit_bcmask_for_bw24sectors_Wire;TgcPrd_BWSectorsVsBCMask_Wire',
                               title='BWSectorsVsBCMask_Wire;;',type='TH2F',
                               path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=8,ymin=-0.5,ymax=7.5,ylabels=['Undef','Next-only','Curr-only','Curr&Next','Prev-only','Prev&Next','Prev&Curr','Prev&Curr&Next'])
    myGroupHit.defineHistogram('hit_bcmask_bw24sectors_Strip,hit_bcmask_for_bw24sectors_Strip;TgcPrd_BWSectorsVsBCMask_Strip',
                               title='BWSectorsVsBCMask_Strip;;',type='TH2F',
                               path=hitPath,xbins=25,xmin=-12.5,xmax=12.5,xlabels=label_bw24sectors,ybins=8,ymin=-0.5,ymax=7.5,ylabels=['Undef','Next-only','Curr-only','Curr&Next','Prev-only','Prev&Next','Prev&Curr','Prev&Curr&Next'])


    
    for side in ['A', 'C']:# side-A or side-C
        for station in range(1,5):# M1,2,3,4
            for s_or_w in ['S','W']:# strip or wire
                name = "%sM%02i%s" % (side,station,s_or_w) # e.g. AM01W
                nbins = 10
                label_glbl_index = []
                if station==1:
                    nbins = 720
                    label_glbl_index = label_glbl_index_bw1
                elif station==2 or station==3:
                    nbins = 576
                    label_glbl_index = label_glbl_index_bw23
                else: # station==4
                    nbins = 96
                    label_glbl_index = label_glbl_index_sw

                x_name = "mon_lb"
                y_name = "hit_glblphi_%s" % (name)
                objname = "TgcPrd_GlobalChamberIndexVsLB_%s" % (name)
                title = "GlobalChamberIndexVsLB_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                        title=title+';Luminosity block;',type='TH2F',
                                        path=hitPath,xbins=100,xmin=-0.5,xmax=99.5,
                                        ybins=nbins,ymin=0.5,ymax=nbins+0.5,ylabels=label_glbl_index,opt='kAddBinsDynamically', merge='merge')
                x_name = "hit_glblphi_%s" % (name)
                y_name = "hit_bunch_%s" % (name)
                objname = "TgcPrd_GlobalChamberIndexVsTiming_All_%s" % (name)
                title = "GlobalChamberIndexVsTiming_All_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                           title=title+';;Timing',type='TH2F',
                                           path=hitPath,xbins=nbins,xmin=0.5,xmax=nbins+0.5,xlabels=label_glbl_index,
                                           ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
                x_name = "hit_glblphi_wTrack_%s" % (name)
                y_name = "hit_bunch_wTrack_%s" % (name)
                objname = "TgcPrd_GlobalChamberIndexVsTiming_wTrack_%s" % (name)
                title = "GlobalChamberIndexVsTiming_wTrack_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                           title=title+';;Timing',type='TH2F',
                                           path=hitPath,xbins=nbins,xmin=0.5,xmax=nbins+0.5,xlabels=label_glbl_index,
                                           ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
                
                x_name = "hit_glblphi_effnum_%s" % (name)
                y_name = "hit_glblphi_effden_%s" % (name)
                objname = "TgcPrd_GlobalChamberIndex_Efficiency_%s" % (name)
                title = "GlobalChamberIndex_Efficiency_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                           title=title+';;Efficiency',type='TEfficiency',
                                           path=hitEffPath,xbins=nbins,xmin=0.5,xmax=nbins+0.5,xlabels=label_glbl_index)

                x_name = "hit_bcmask_glblphi_%s" % (name)
                y_name = "hit_bcmask_%s" % (name)
                objname = "TgcPrd_GlobalChamberIndexVsBCMask_%s" % (name)
                title = "GlobalChamberIndexVsBCMask_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                           title=title+';;',type='TH2F',
                                           path=hitPath,xbins=nbins,xmin=0.5,xmax=nbins+0.5,xlabels=label_glbl_index,
                                           ybins=8,ymin=-0.5,ymax=7.5,ylabels=['Undef',
                                                                               'Next-only',
                                                                               'Curr-only',
                                                                               'Curr&Next',
                                                                               'Prev-only',
                                                                               'Prev&Next',
                                                                               'Prev&Curr',
                                                                               'Prev&Curr&Next'])


                nbinsx = 10
                nbinsy = 10
                label_ieta = []
                label_iphi = []
                if station==1:
                    nbinsx = 15
                    nbinsy = 48
                    label_ieta = label_ieta_bw1
                    label_iphi = label_iphi_bw1
                elif station==2 or station==3:
                    nbinsx = 12
                    nbinsy = 48
                    label_ieta = label_ieta_bw23
                    label_iphi = label_iphi_bw23
                else: # station==4
                    nbinsx = 4
                    nbinsy = 24
                    label_ieta = label_ieta_sw
                    label_iphi = label_iphi_sw

                x_name = "hit_x_%s" % (name)
                y_name = "hit_y_%s" % (name)
                objname = "TgcPrd_iPhiVsiEta_%s" % (name)
                title = "iPhiVsiEta_%s" % (name)
                myGroupHit.defineHistogram(x_name+','+y_name+';'+objname,
                                           title=title+';;',type='TH2F',path=hitPath,
                                           xbins=nbinsx,xmin=0.5,xmax=nbinsx+0.5,xlabels=label_ieta,
                                           ybins=nbinsy,ymin=0.5,ymax=nbinsy+0.5,ylabels=label_iphi)

                eff_name = "hit_effnum_x_%s" % (name)
                x_name = "hit_effden_x_%s" % (name)
                y_name = "hit_effden_y_%s" % (name)
                objname = "TgcPrd_iPhiVsiEta_Efficiency_%s" % (name)
                title = "iPhiVsiEta_Efficiency_%s" % (name)
                myGroupHit.defineHistogram(eff_name+','+x_name+','+y_name+';'+objname,
                                           title=title+';;',type='TEfficiency',
                                           path=hitEffPath,
                                           xbins=nbinsx,xmin=0.5,xmax=nbinsx+0.5,xlabels=label_ieta,
                                           ybins=nbinsy,ymin=0.5,ymax=nbinsy+0.5,ylabels=label_iphi)



    for side in ['A', 'C']:# side-A or side-C
        for sector in range(0,16):# Sector 00...15 (00 for FI, 01..15 for EI, 01..12 for BW)
            for station in range(1,5):# M1,2,3,4
                for phi in range(25):# internal phi 0,1,2,3...24 (0..3 for BW and EI, 1..24 for FI)
                    for eta in range(6):# eta index 1,,,5 for Endcap, and 0 for Forward
                        for lay in range(1,4):# sub-layer 1,2,3 (triplet) or 1,2 (doublet)
                            if station<4 and (sector>12 or sector==0):continue # BW only 1..12 sectors
                            if station==4 and eta==0 and sector!=0:continue # FI only sector-0
                            if station==4 and eta==1 and sector%2==0:continue # EI only odd-sectors (1,3,5,7..15)
                            if station==4 and eta==0 and phi==0:continue # FI only 1..24 internal phi
                            if (station<4 or (station==4 and eta==1)) and phi>3: continue # BW and EI only 0..3 internal phi
                            if station==1 and eta==5:continue # BW M1 has only 1..4 eta
                            if station==4 and eta>1:continue # EI/FI eta 0 or 1
                            if station>1 and lay>2:continue # triplet is only M1
                            for s_or_w in ['S','W']:# strip or wire
                                if station==1 and lay==2 and s_or_w=="S":continue # there is no strip channel on M1 L2
                                nbins = 100
                                if s_or_w=="S":
                                    nbins = 32
                                else:
                                    if station==1:
                                        if eta==1:     nbins = 24
                                        elif eta==2:   nbins = 23
                                        elif eta==3:
                                            if lay==1: nbins = 61
                                            else:      nbins = 62
                                        elif eta==4:
                                            if lay==1: nbins = 92
                                            else:      nbins = 91
                                        else: # forward
                                            if lay==2: nbins = 104
                                            else:      nbins = 105
                                    elif station==2:
                                        if eta==1 or eta==2 or eta==3: nbins = 32
                                        elif eta==4:                   nbins = 103
                                        elif eta==5:                   nbins = 110
                                        else:                          nbins = 125 # forward
                                    elif station==3:
                                        if eta==1:   nbins = 31
                                        elif eta==2: nbins = 30
                                        elif eta==3: nbins = 32
                                        elif eta==4: nbins = 106
                                        elif eta==5: nbins = 96
                                        else:        nbins = 122
                                    else: # EI/FI
                                        if eta==1:
                                            if (sector==1 and phi==0) or (sector==1 and phi==2) or (sector==9 and phi==0) or (sector==9 and phi==2) or sector==11 or sector==15:
                                                nbins = 16 # EI short
                                            else:
                                                nbins = 24 # EI
                                        else:        nbins = 32 # FI
                                
                                if doGapByGapHitOcc:
                                    chamber_name = "%s%02dM%02df%02d%s%02dL%02d%s" % (side,sector,station,phi,'F' if eta==0 else 'E',eta,lay,s_or_w)
                                    objname = 'TgcPrd_ChannelOccupancy_'+ chamber_name
                                    title = 'Occ_'+chamber_name
                                    myGroupHit.defineHistogram('hit_on_'+chamber_name+';'+objname,
                                                               title=title+';Channel ID;Number of events',
                                                               path="GapByGapHits/",xbins=nbins,xmin=0.5,xmax=nbins+0.5)

                                if doGapByGapEffMap:
                                    objname = 'TgcPrd_GapByGapEffMap_' + chamber_name
                                    title = 'Eff_' + chamber_name
                                    objectx = 'hit_localX_effden_' + chamber_name
                                    objecty = 'hit_localY_effden_' + chamber_name
                                    objectz = 'hit_effnum_' + chamber_name
                                    myGroupHit.defineHistogram(objectz+','+objectx+','+objecty+';'+objname,
                                                               title=title+';X [mm];Y [mm]',type='TEfficiency',
                                                               path="GapByGapEffMap/",xbins=100,xmin=-1000,xmax=1000,ybins=100,ymin=-1500,ymax=1500)

                                if lay == 1 and doHitResiduals:
                                    chamber_name = "%s%02dM%02df%02d%s%02d%s" % (side,sector,station,phi,'F' if eta==0 else 'E',eta,s_or_w)
                                    objname = 'TgcPrd_Residual_' + chamber_name
                                    title = 'Res_'+chamber_name
                                    myGroupHit.defineHistogram('hit_residual_on_'+chamber_name+';'+objname,
                                                               title=title+';Residual [mm];Number of events',
                                                               path="Residuals/",xbins=400,xmin=-1000,xmax=1000)






    ################################################################################################################
    myGroupCoin = helper.addGroup(tgcRawDataMonAlg, 'TgcRawDataMonitor_TgcCoin', mainDir)
    coinPath = 'Coin/'
    myGroupCoin.defineHistogram('nTgcCoinDetElementIsNull;h_nTgcCoinDetElementIsNull',title='nTgcCoinDetElementIsNull',
                            path=coinPath,xbins=101,xmin=-0.5,xmax=100.5)
    myGroupCoin.defineHistogram('nTgcCoinPostOutPtrIsNull;h_nTgcCoinPostOutPtrIsNull',title='nTgcCoinPostOutPtrIsNull',
                            path=coinPath,xbins=101,xmin=-0.5,xmax=100.5)

    for Det in ['Eifi','Tile','Rpc','Nsw']:
        det = Det.lower()
        for Region in ['','_Endcap','_Forward']:
            region = Region.lower()
            nrois = 64 if Region == '_Forward' else 148
            nsectors = 24 if Region == '_Forward' else 48
            rmask_endfwd = 'coin_inner_tgc_forward' if Region == '_Forward' else 'coin_inner_tgc_endcap'
            rmask_coverage = 'coin_inner_tgc_etaupto1p3'
            if Det == 'Nsw':
                if Region == '_Endcap':
                    rmask_coverage = 'coin_inner_tgc_etafrom1p3_endcap'
                else:
                    rmask_coverage = 'coin_inner_tgc_forward'

            ### Evt 1D and 2D histograms ###
            if (Det != 'Nsw' and Region == '') or (Det == 'Nsw' and Region != ''):
                myGroupCoin.defineHistogram('coin_inner_'+det+'_slSector'+region+',coin_inner_'+det+'_deltaBcid;InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaBcid',
                                            title='InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaBcid;SL Trigger Sector +1  (>0 for A, <0 for C);Delta Bcid ('+Det+' - ATLAS)',
                                            path=coinPath,type='TH2F',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,ybins=31,ymin=-15.5,ymax=15.5)
                myGroupCoin.defineHistogram('coin_inner_'+det+'_slSector'+region+',coin_inner_'+det+'_deltaTiming;InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaTiming',
                                            title='InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaTiming;SL Trigger Sector +1  (>0 for A, <0 for C);Delta Signal Timing',
                                            path=coinPath,type='TH2F',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,ybins=11,ymin=-5.5,ymax=5.5)
                myGroupCoin.defineHistogram('coin_inner_'+det+'_slSector'+region+',coin_inner_'+det+'_deltaBcid;InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaBcid_BcCurr',
                                            title='InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaBcid_BcCurr;SL Trigger Sector +1  (>0 for A, <0 for C);Delta Bcid ('+Det+' - ATLAS)',
                                            path=coinPath,type='TH2F',cutmask='coin_inner_'+det+'_currBc',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,ybins=31,ymin=-15.5,ymax=15.5)
                myGroupCoin.defineHistogram('mon_bcid,coin_inner_'+det+'_deltaBcid;InnerCoin_Evt2D_'+Det+Region+'_BcidVsDeltaBcid_BcCurr',
                                            title='InnerCoin_Evt2D_'+Det+Region+'_BcidVsDeltaBcid_BcCurr;Bcid;Delta Bcid ('+Det+' - ATLAS)',
                                            path=coinPath,type='TH2F',cutmask='coin_inner_'+det+region+'_currBc',xbins=4096,xmin=-0.5,xmax=4095.5,ybins=31,ymin=-15.5,ymax=15.5)
                myGroupCoin.defineHistogram('coin_inner_'+det+'_slSector'+region+',coin_inner_'+det+'_deltaTiming;InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaTiming_goodBcid0',
                                            title='InnerCoin_Evt2D_'+Det+Region+'_SectorVsDeltaTiming_goodBcid0;SL Trigger Sector +1  (>0 for A, <0 for C);Delta Signal Timing',
                                            path=coinPath,type='TH2F',cutmask='coin_inner_'+det+'_goodBcid0',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,ybins=11,ymin=-5.5,ymax=5.5)
                ### Bcid matching window scan ### 
                for bcid in ['','0','1','2']:
                    goodBcid = '_goodBcid'+bcid if bcid != '' else ''
                    cmask = 'coin_inner_'+det+goodBcid if bcid != '' else ''
                    myGroupCoin.defineHistogram('mon_lb,coin_inner_'+det+'_slSector_goodTiming'+region+';InnerCoin_Evt2D_'+Det+Region+'_SectorVsLB_BcCurr'+goodBcid,
                                                title='InnerCoin_Evt2D_'+Det+Region+'_SectorVsLB_BcCurr'+goodBcid+';Lumi Block;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                path=coinPath,type='TH2F',cutmask=cmask,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=100,xmin=0.5,xmax=100.5,
                                                opt='kAddBinsDynamically', merge='merge')
            ### Efficiency and Reduction ###
            for htype in ['Eff','Reduction']:
                fake = '_fake' if htype == 'Reduction' else ''
                ### Sector or SectorVs ###
                if (Det != 'Nsw' and Region == '') or (Det == 'Nsw' and Region != ''):
                    myGroupCoin.defineHistogram('coin_inner_tgc_coinflag'+Det+',coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_CoinFlag',
                                                title='InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_CoinFlag;SL Trigger Sector +1 (>0 for A, <0 for C);Efficiency',
                                                path=coinPath,type='TEfficiency',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,cutmask=rmask_coverage)
                    myGroupCoin.defineHistogram('coin_inner_tgc_coinflag'+Det+',coin_inner_tgc_roi,coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_CoinFlag',
                                                title='InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_CoinFlag;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                path=coinPath,type='TEfficiency',cutmask=rmask_endfwd,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=nrois,xmin=-0.5,xmax=nrois-0.5)
                    myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+'_goodBcid0,coin_inner_tgc_roi,coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_goodBcid0',
                                                title='InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_goodBcid0;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                path=coinPath,type='TEfficiency',cutmask=rmask_endfwd,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=nrois,xmin=-0.5,xmax=nrois-0.5)
                    myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+',coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_goodTiming',
                                                title='InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_goodTiming;SL Trigger Sector +1 (>0 for A, <0 for C);Efficiency',
                                                path=coinPath,type='TEfficiency',xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5,cutmask=rmask_coverage)
                    myGroupCoin.defineHistogram('coin_inner_tgc_anyBc'+Det+',coin_inner_tgc_roi,coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI',
                                                title='InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                path=coinPath,type='TEfficiency',cutmask=rmask_endfwd,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=nrois,xmin=-0.5,xmax=nrois-0.5)
                    myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+',coin_inner_tgc_roi,coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_goodTiming',
                                                title='InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_goodTiming;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                path=coinPath,type='TEfficiency',cutmask=rmask_endfwd,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=nrois,xmin=-0.5,xmax=nrois-0.5)
                ### Eta or EtaVsPhi ##
                if Region == '':
                    myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+',coin_inner_tgc'+fake+'_eta;InnerCoin_'+htype+'1D_'+Det+'_Eta_goodTiming',
                                                title='InnerCoin_'+htype+'1D_'+Det+'_Eta_goodTiming;RoI Eta;Efficiency',
                                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5)
                    myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+',coin_inner_tgc'+fake+'_eta,coin_inner_tgc'+fake+'_phi;InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_goodTiming',
                                                title='InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_goodTiming;RoI Eta;RoI Phi',
                                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                    myGroupCoin.defineHistogram('coin_inner_tgc_coinflag'+Det+',coin_inner_tgc'+fake+'_eta,coin_inner_tgc'+fake+'_phi;InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_CoinFlag',
                                                title='InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_CoinFlag;RoI Eta;RoI Phi',
                                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                    myGroupCoin.defineHistogram('coin_inner_tgc_coinflag'+Det+',coin_inner_tgc'+fake+'_eta;InnerCoin_'+htype+'1D_'+Det+'_Eta_CoinFlag',
                                                title='InnerCoin_'+htype+'1D_'+Det+'_Eta_CoinFlag;RoI Eta;Efficiency',
                                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5)

                ### Signal timing scan ###
                for Bunch in ['Any','Prev','Curr','Next','NextNext']:
                    bunch = Bunch.lower()
                    ### Bcid matchinng window scan ###
                    for bcid in ['','0','1','2']:
                        goodBcid = '_goodBcid'+bcid if bcid != '' else ''
                        if (Det != 'Nsw' and Region == '') or (Det == 'Nsw' and Region != ''):
                            myGroupCoin.defineHistogram('coin_inner_tgc_'+bunch+'Bc'+Det+goodBcid+',coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_Bc'+Bunch+goodBcid,
                                                        title='InnerCoin_'+htype+'1D_'+Det+Region+'_Sector_Bc'+Bunch+goodBcid+';SL Trigger Sector +1  (>0 for A, <0 for C);Efficiency',
                                                        path=coinPath,type='TEfficiency',cutmask=rmask_coverage,xbins=nsectors*2+1,xmin=-1*nsectors-0.5,xmax=nsectors+0.5)
                            myGroupCoin.defineHistogram('coin_inner_tgc_'+bunch+'Bc'+Det+goodBcid+',coin_inner_tgc_roi,coin_inner_tgc'+fake+'_sector;InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_Bc'+Bunch+goodBcid,
                                                        title='InnerCoin_'+htype+'2D_'+Det+Region+'_SectorVsRoI_Bc'+Bunch+goodBcid+';RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                                        path=coinPath,type='TEfficiency',cutmask=rmask_endfwd,ybins=nsectors*2+1,ymin=-1*nsectors-0.5,ymax=nsectors+0.5,xbins=nrois,xmin=-0.5,xmax=nrois-0.5)
                        if Region == '':
                            myGroupCoin.defineHistogram('coin_inner_tgc_'+bunch+'Bc'+Det+goodBcid+',coin_inner_tgc'+fake+'_eta,coin_inner_tgc'+fake+'_phi;InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_Bc'+Bunch+goodBcid,
                                                        title='InnerCoin_'+htype+'2D_'+Det+'_EtaVsPhi_Bc'+Bunch+goodBcid+';RoI Eta;RoI Phi',
                                                        path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                            myGroupCoin.defineHistogram('coin_inner_tgc_currBc'+Det+goodBcid+',coin_inner_tgc'+fake+'_eta;InnerCoin_'+htype+'1D_'+Det+'_Eta_Bc'+Bunch+goodBcid,
                                                        title='InnerCoin_'+htype+'1D_'+Det+'_Eta_Bc'+Bunch+goodBcid+';RoI Eta;Efficiency',
                                                        path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5)


    #Tile inner coincidence
    myGroupCoin.defineHistogram('coin_inner_tile2_slSector,coin_inner_tile2_tmdbDecisions;InnerCoin_Evt2D_Tile_SectorVsTmdbDecisions',
                                title='InnerCoin_Evt2D_Tile_SectorVsTmdbDecisions;SL Trigger Sector +1  (>0 for A, <0 for C);TMDB Module Decisions',
                                path=coinPath,type='TH2F',xbins=97,xmin=-48.5,xmax=48.5,ybins=3,ymin=0.5,ymax=3.5,ylabels=['D5 only','D6 only','D5+D6'])
    myGroupCoin.defineHistogram('coin_inner_tile2_slSector,coin_inner_tile2_tmdbDecisions;InnerCoin_Evt2D_Tile_SectorVsTmdbDecisions_BcCurr',
                                title='InnerCoin_Evt2D_Tile_SectorVsTmdbDecisions_BcCurr;SL Trigger Sector +1  (>0 for A, <0 for C);TMDB Module Decisions',
                                path=coinPath,type='TH2F',cutmask='coin_inner_tile2_currBc',xbins=97,xmin=-48.5,xmax=48.5,ybins=3,ymin=0.5,ymax=3.5,ylabels=['D5 only','D6 only','D5+D6'])

    #Nsw inner coincidence
    myGroupCoin.defineHistogram('coin_inner_nsw_roiEta,coin_inner_nsw_R;InnerCoin_Evt2D_Nsw_RoiEtaVsR',
                                title='InnerCoin_Evt2D_Nsw_RoiEtaVsR;SL RoI Eta;NSW R',
                                path=coinPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=251,ymin=-0.5,ymax=250.5)
    myGroupCoin.defineHistogram('coin_inner_nsw_roiPhi,coin_inner_nsw_Phi;InnerCoin_Evt2D_Nsw_RoiPhiVsPhi',
                                title='InnerCoin_Evt2D_Nsw_RoiPhiVsPhi;SL RoI Phi;NSW Phi',
                                path=coinPath,type='TH2F',xbins=48*10,xmin=-math.pi,xmax=math.pi,ybins=61,ymin=-0.5,ymax=60.5)
    myGroupCoin.defineHistogram('coin_inner_nsw_roiEta,coin_inner_nsw_deltaR;InnerCoin_Evt2D_Nsw_RoiEtaVsDeltaR',
                                title='InnerCoin_Evt2D_Nsw_RoiEtaVsDeltaR;SL RoI Eta;DeltaR(Nsw - SL)',
                                path=coinPath,type='TH2F',xbins=100,xmin=-2.5,xmax=2.5,ybins=1000,ymin=-500,ymax=500)


    ## NSW info per SL input
    # Endcap
    for side in ['A','C']:
        label_slnswinput_index = []
        slid = 0
        for sec in range(1,13):# 1..12
            for dup in range(2):# 0..1
                slid += 1
                for inp in range(6):# 0..6
                    label = '%s%02dE%02d-%d' % (side,sec,slid,inp) # A01E01-0
                    label_slnswinput_index.append(label)
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Endcap,coin_inner_nsw_deltaBcid;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaBcid',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaBcid;;Delta Bcid (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=144,xmin=-0.5,xmax=143.5,xlabels=label_slnswinput_index,ybins=31,ymin=-15.5,ymax=15.5)
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Endcap,coin_inner_nsw_deltaTiming;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaTiming',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaTiming;;Delta Signal Timing (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=144,xmin=-0.5,xmax=143.5,xlabels=label_slnswinput_index,ybins=11,ymin=-5.5,ymax=5.5)

        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Endcap,coin_inner_nsw_deltaBcid;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaBcid_goodTiming',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaBcid_goodTiming;;Delta Bcid (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=144,xmin=-0.5,xmax=143.5,xlabels=label_slnswinput_index,ybins=31,ymin=-15.5,ymax=15.5,cutmask='coin_inner_nsw_goodTiming')
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Endcap,coin_inner_nsw_deltaTiming;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaTiming_goodBcid0',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsDeltaTiming_goodBcid0;;Delta Signal Timing (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=144,xmin=-0.5,xmax=143.5,xlabels=label_slnswinput_index,ybins=11,ymin=-5.5,ymax=5.5,cutmask='coin_inner_nsw_goodBcid0')

        for bcid in ['0','1','2']:
            myGroupCoin.defineHistogram('mon_lb,coin_inner_nsw_slInputIndex_'+side+'Endcap;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsLB_NswGoodTimingBcid'+bcid,
                                        title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsLB_NswGoodTimingBcid'+bcid+';Lumi Block;',cutmask='coin_inner_nsw_goodTimingBcid'+bcid,
                                        opt='kAddBinsDynamically', merge='merge',path=coinPath,type='TH2F',
                                        ybins=144,ymin=-0.5,ymax=143.5,ylabels=label_slnswinput_index,xbins=100,xmin=0.5,xmax=100.5)
        for bunch in ['Prev','Curr','Next','NextNext']:
            myGroupCoin.defineHistogram('mon_lb,coin_inner_nsw_slInputIndex_'+side+'Endcap;InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsLB_Bc'+bunch,
                                        title='InnerCoin_Evt2D_Nsw_'+side+'Endcap_SLInputVsLB_Bc'+bunch+';Lumi Block;',cutmask='coin_inner_nsw_Bc'+bunch,
                                        opt='kAddBinsDynamically', merge='merge',path=coinPath,type='TH2F',
                                        ybins=144,ymin=-0.5,ymax=143.5,ylabels=label_slnswinput_index,xbins=100,xmin=0.5,xmax=100.5)


    # Forward
    for side in ['A','C']:
        label_slnswinput_index = []
        for sec in range(1,13):# 1..12
            for inp in range(6):# 0..6
                label = '%s%02dF%02d-%d' % (side,sec,sec,inp) # A01F01-0
                label_slnswinput_index.append(label)
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Forward,coin_inner_nsw_deltaBcid;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaBcid',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaBcid;;Delta Bcid (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=72,xmin=-0.5,xmax=71.5,xlabels=label_slnswinput_index,ybins=31,ymin=-15.5,ymax=15.5)
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Forward,coin_inner_nsw_deltaTiming;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaTiming',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaTiming;;Delta Signal Timing (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=72,xmin=-0.5,xmax=71.5,xlabels=label_slnswinput_index,ybins=11,ymin=-5.5,ymax=5.5)

        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Forward,coin_inner_nsw_deltaBcid;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaBcid_goodTiming',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaBcid_goodTiming;;Delta Bcid (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=72,xmin=-0.5,xmax=71.5,xlabels=label_slnswinput_index,ybins=31,ymin=-15.5,ymax=15.5,cutmask='coin_inner_nsw_goodTiming')
        myGroupCoin.defineHistogram('coin_inner_nsw_slInputIndex_'+side+'Forward,coin_inner_nsw_deltaTiming;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaTiming_goodBcid0',
                                    title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsDeltaTiming_goodBcid0;;Delta Signal Timing (Nsw - ATLAS)',
                                    path=coinPath,type='TH2F',xbins=72,xmin=-0.5,xmax=71.5,xlabels=label_slnswinput_index,ybins=11,ymin=-5.5,ymax=5.5,cutmask='coin_inner_nsw_goodBcid0')

        for bcid in ['0','1','2']:
            myGroupCoin.defineHistogram('mon_lb,coin_inner_nsw_slInputIndex_'+side+'Forward;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsLB_NswGoodTimingBcid'+bcid,
                                        title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsLB_NswGoodTimingBcid'+bcid+';Lumi Block;',cutmask='coin_inner_nsw_goodTimingBcid'+bcid,
                                        opt='kAddBinsDynamically', merge='merge',path=coinPath,type='TH2F',
                                        ybins=72,ymin=-0.5,ymax=71.5,ylabels=label_slnswinput_index,xbins=100,xmin=0.5,xmax=100.5)
        for bunch in ['Prev','Curr','Next','NextNext']:
            myGroupCoin.defineHistogram('mon_lb,coin_inner_nsw_slInputIndex_'+side+'Forward;InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsLB_Bc'+bunch,
                                        title='InnerCoin_Evt2D_Nsw_'+side+'Forward_SLInputVsLB_Bc'+bunch+';Lumi Block;',cutmask='coin_inner_nsw_Bc'+bunch,
                                        opt='kAddBinsDynamically', merge='merge',path=coinPath,type='TH2F',
                                        ybins=72,ymin=-0.5,ymax=71.5,ylabels=label_slnswinput_index,xbins=100,xmin=0.5,xmax=100.5)

    ## CoinFlagC
    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_roi,coin_inner_tgc_sector;InnerCoin_Eff2D_CoinFlagC_Endcap_SectorVsRoI',
                                title='InnerCoin_Eff2D_CoinFlagC_Endcap_SectorVsRoI;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                path=coinPath,type='TEfficiency',cutmask='coin_inner_tgc_endcap',ybins=97,ymin=-48.5,ymax=48.5,xbins=149,xmin=-0.5,xmax=148.5)
    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_roi,coin_inner_tgc_fake_sector;InnerCoin_Reduction2D_CoinFlagC_Endcap_SectorVsRoI',
                                title='InnerCoin_Reduction2D_CoinFlagC_Endcap_SectorVsRoI;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                path=coinPath,type='TEfficiency',cutmask='coin_inner_tgc_endcap',ybins=97,ymin=-48.5,ymax=48.5,xbins=149,xmin=-0.5,xmax=148.5)

    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_roi,coin_inner_tgc_sector;InnerCoin_Eff2D_CoinFlagC_Forward_SectorVsRoI',
                                title='InnerCoin_Eff2D_CoinFlagC_Forward_SectorVsRoI;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                path=coinPath,type='TEfficiency',cutmask='coin_inner_tgc_forward',ybins=49,ymin=-24.5,ymax=24.5,xbins=65,xmin=-0.5,xmax=64.5)
    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_roi,coin_inner_tgc_fake_sector;InnerCoin_Reduction2D_CoinFlagC_Forward_SectorVsRoI',
                                title='InnerCoin_Reduction2D_CoinFlagC_Forward_SectorVsRoI;RoI;SL Trigger Sector +1  (>0 for A, <0 for C)',
                                path=coinPath,type='TEfficiency',cutmask='coin_inner_tgc_forward',ybins=49,ymin=-24.5,ymax=24.5,xbins=65,xmin=-0.5,xmax=64.5)

    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_eta,coin_inner_tgc_phi;InnerCoin_Eff2D_CoinFlagC_EtaVsPhi',
                                title='InnerCoin_Eff2D_CoinFlagC_EtaVsPhi;RoI Eta;RoI Phi',
                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_fake_eta,coin_inner_tgc_fake_phi;InnerCoin_Reduction2D_CoinFlagC_EtaVsPhi',
                                title='InnerCoin_Reduction2D_CoinFlagC_EtaVsPhi;RoI Eta;RoI Phi',
                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_eta;InnerCoin_Eff1D_CoinFlagC_Eta',
                                title='InnerCoin_Eff1D_CoinFlagC_Eta;RoI Eta;Efficiency',
                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5)
    myGroupCoin.defineHistogram('coin_inner_tgc_coinflagC,coin_inner_tgc_fake_eta;InnerCoin_Reduction1D_CoinFlagC_Eta',
                                title='InnerCoin_Reduction1D_CoinFlagC_EtaVsPhi;RoI Eta;Efficiency',
                                path=coinPath,type='TEfficiency',xbins=100,xmin=-2.5,xmax=2.5)

    for coinType in ['SL','HPT','LPT','EIFI']:
        for region in ['','Endcap','Forward']:
            suffix0 = coinType if region == '' else coinType+'_'+region
            nrois = 148 if region == 'Endcap' else 64
            nsectors = 48 if region == 'Endcap' else 24
            for chanType in ['','Wire','Strip']:
                if coinType == 'SL' and chanType != '': continue # no wire or strip for "SL"
                if coinType == 'HPT' and chanType == '': continue # always wire or strip for "HPT"
                if coinType == 'LPT' and chanType == '': continue # always wire or strip for "LPT"
                if coinType == 'EIFI' and chanType == '': continue # always wire or strip for "EIFI" tracklet
                suffix = suffix0+'_' if chanType == '' else suffix0+'_'+chanType+'_'

                if coinType == 'SL':
                    for thr in range(1,16): #1.2...15
                        cut = suffix+"coin_cutmask_pt"+str(thr)
                        PT = "_Thr%02d" % thr
                        if region == '':
                            myGroupCoin.defineHistogram(suffix+'coin_eta,'+suffix+'coin_phi;'+suffix+'TgcCoin_Evt_EtaVsPhi'+PT,
                                                    title=suffix+'TgcCoin_Evt_EtaVsPhi'+PT+';Eta;Phi',
                                                    type='TH2F',path=coinPath,cutmask=cut,
                                                    xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                        else:
                            myGroupCoin.defineHistogram(suffix+'coin_roi,'+suffix+'coin_sector;'+suffix+'TgcCoin_Evt_SectorVsRoI'+PT,
                                                    title=suffix+'TgcCoin_Evt_SectorVsRoI'+PT+';RoI;Trigger Sector +1  (>0 for A, <0 for C)',
                                                    type='TH2F',path=coinPath,cutmask=cut,
                                                    xbins=nrois+1,xmin=-0.5,xmax=nrois+0.5,
                                                    ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)

                if region == '':
                    myGroupCoin.defineHistogram(suffix+'coin_eta,'+suffix+'coin_phi;'+suffix+'TgcCoin_Evt_EtaVsPhi',
                                            title=suffix+'TgcCoin_Evt_EtaVsPhi;Eta;Phi',
                                            type='TH2F',path=coinPath,
                                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                    myGroupCoin.defineHistogram(suffix+'coin_ext_matched,'+suffix+'coin_ext_eta,'+suffix+'coin_ext_phi;'+suffix+'TgcCoin_Eff_EtaVsPhi',
                                            title=suffix+'TgcCoin_Eff_EtaVsPhi;Eta;Phi',
                                            type='TEfficiency',path=coinPath,
                                            xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                    if coinType == 'SL':
                        for coinflag in ['Qpos','Qneg','F','C','H','EI','Tile','RPC','NSW']:
                            myGroupCoin.defineHistogram(suffix+'coin_eta,'+suffix+'coin_phi;'+suffix+'TgcCoin_Evt_EtaVsPhi_CoinFlag'+coinflag,
                                                        title=suffix+'TgcCoin_Evt_EtaVsPhi_CoinFlag'+coinflag+';Eta;Phi',cutmask=suffix+'coin_CoinFlag'+coinflag,
                                                        type='TH2F',path=coinPath,
                                                        xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)
                        for coinflag in ['Q','F','C','H','EI','Tile','RPC','NSW']:
                            myGroupCoin.defineHistogram(suffix+'coin_ext_matched_CoinFlag'+coinflag+','+suffix+'coin_ext_matched_eta,'+suffix+'coin_ext_matched_phi;'+suffix+'TgcCoin_Eff_EtaVsPhi_CoinFlag'+coinflag,
                                                        title=suffix+'TgcCoin_Eff_EtaVsPhi_CoinFlag'+coinflag+';Eta;Phi',
                                                        type='TEfficiency',path=coinPath,
                                                        xbins=100,xmin=-2.5,xmax=2.5,ybins=48,ymin=-math.pi,ymax=math.pi)

                else:
                    myGroupCoin.defineHistogram(suffix+'coin_sector,'+suffix+'coin_bunch;'+suffix+'TgcCoin_Evt_SectorVsTiming',
                                            title=suffix+'TgcCoin_Evt_SectorVsTiming;Trigger Sector +1  (>0 for A, <0 for C);Timing',
                                            type='TH2F',path=coinPath,
                                            xbins=nsectors*2+1,xmin=-nsectors-0.5,xmax=nsectors+0.5,
                                            ybins=3,ymin=-1.5,ymax=1.5,ylabels=['Previous','Current','Next'])
                    myGroupCoin.defineHistogram(suffix+'coin_lb,'+suffix+'coin_sector;'+suffix+'TgcCoin_Evt_SectorVsLB',
                                            title=suffix+'TgcCoin_Evt_SectorVsLB;LumiBlock;Trigger Sector +1  (>0 for A, <0 for C)',
                                            type='TH2F',path=coinPath,
                                            xbins=100,xmin=0.5,xmax=100.5,opt='kAddBinsDynamically', merge='merge',
                                            ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)

                    if coinType == 'SL':
                        for coinflag in ['EI','Tile','RPC','NSW','F','C','H']:
                            if region == 'Forward' :
                                if coinflag == 'EI' :continue
                                if coinflag == 'Tile' :continue
                                if coinflag == 'RPC' :continue
                            myGroupCoin.defineHistogram(suffix+'coin_roi,'+suffix+'coin_sector;'+suffix+'TgcCoin_Evt_SectorVsRoI_CoinFlag'+coinflag,
                                                        title=suffix+'TgcCoin_Evt_SectorVsRoI_CoinFlag'+coinflag+';RoI;Trigger Sector +1  (>0 for A, <0 for C)',
                                                        type='TH2F',path=coinPath,cutmask=suffix+'coin_CoinFlag'+coinflag,
                                                        xbins=nrois+1,xmin=-0.5,xmax=nrois+0.5,
                                                        ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)
                            if coinflag != 'H' :
                                myGroupCoin.defineHistogram(suffix+'coin_lb,'+suffix+'coin_sector;'+suffix+'TgcCoin_Evt_SectorVsLB_CoinFlag'+coinflag,
                                                            title=suffix+'TgcCoin_Evt_SectorVsLB_CoinFlag'+coinflag+';LumiBlock;Trigger Sector +1  (>0 for A, <0 for C)',
                                                            type='TH2F',path=coinPath,cutmask=suffix+'coin_CoinFlag'+coinflag,
                                                            xbins=100,xmin=0.5,xmax=100.5,opt='kAddBinsDynamically', merge='merge',
                                                            ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)

                        myGroupCoin.defineHistogram(suffix+'coin_roi,'+suffix+'coin_sector;'+suffix+'TgcCoin_Evt_SectorVsRoI',
                                                title=suffix+'TgcCoin_Evt_SectorVsRoI;RoI;Trigger Sector +1  (>0 for A, <0 for C)',
                                                type='TH2F',path=coinPath,
                                                xbins=nrois+1,xmin=-0.5,xmax=nrois+0.5,
                                                ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)
                        myGroupCoin.defineHistogram(suffix+'coin_veto_roi,'+suffix+'coin_veto_sector;'+suffix+'TgcCoin_Evt_SectorVsRoI_wVeto',
                                                title=suffix+'TgcCoin_Evt_SectorVsRoI_wVeto;RoI;Trigger Sector +1  (>0 for A, <0 for C)',
                                                type='TH2F',path=coinPath,
                                                xbins=nrois+1,xmin=-0.5,xmax=nrois+0.5,
                                                ybins=nsectors*2+1,ymin=-nsectors-0.5,ymax=nsectors+0.5)
                        myGroupCoin.defineHistogram(suffix+'coin_sector,'+suffix+'coin_pt;'+suffix+'TgcCoin_Evt_SectorVsThr',
                                                title=suffix+'TgcCoin_Evt_SectorVsThr;Trigger Sector +1  (>0 for A, <0 for C);Threshold',
                                                type='TH2F',path=coinPath,
                                                xbins=nsectors*2+1,xmin=-nsectors-0.5,xmax=nsectors+0.5,
                                                ybins=15,ymin=0.5,ymax=15.5)
                        myGroupCoin.defineHistogram(suffix+'coin_sector,'+suffix+'coin_isPositiveDeltaR;'+suffix+'TgcCoin_Evt_SectorVsisPositiveDeltaR',
                                                title=suffix+'TgcCoin_Evt_SectorVsisPositiveDeltaR;Trigger Sector +1  (>0 for A, <0 for C);isPositiveDeltaR',
                                                type='TH2F',path=coinPath,
                                                xbins=nsectors*2+1,xmin=-nsectors-0.5,xmax=nsectors+0.5,
                                                ybins=2,ymin=-0.5,ymax=1.5)



    ################################################################################################################
    acc = helper.result()
    result.merge(acc)
    return result
    
if __name__=='__main__':
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.isMC = True

    import glob
    import sys
    if len(sys.argv) == 3:
        inputs = sys.argv[1].split(',')
        flags.Input.Files = inputs
        flags.Output.HISTFileName = sys.argv[2]
    else:
        inputs = glob.glob('data/*')
        flags.Input.Files = inputs
        flags.Output.HISTFileName = 'ExampleMonitorOutput.root'

    if not flags.Input.isMC:
        flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2023-01"

    flags.lock()
    flags.dump()

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    tgcRawDataMonitorAcc = TgcRawDataMonitoringConfig(flags)
    cfg.merge(tgcRawDataMonitorAcc)
    cfg.getEventAlgo('TgcRawDataMonAlg').OutputLevel = INFO
    cfg.getEventAlgo('TgcRawDataMonAlg').MonitorThresholdPatterns = True
    cfg.getEventAlgo('TgcRawDataMonAlg').TagAndProbe = True
    cfg.getEventAlgo('TgcRawDataMonAlg').TgcRawDataMonitorTool.OutputLevel = INFO
    cfg.getEventAlgo('TgcRawDataMonAlg').FillGapByGapHistograms = True
    cfg.getEventAlgo('TgcRawDataMonAlg').UseMuonSelectorTool = True
    cfg.getEventAlgo('TgcRawDataMonAlg').UseOnlyCombinedMuons = True
    cfg.getEventAlgo('TgcRawDataMonAlg').UseOnlyMuidCoStacoMuons = True
    cfg.getEventAlgo('TgcRawDataMonAlg').RequireIsolated = True
    cfg.getEventAlgo('TgcRawDataMonAlg').IsolationWindow = 1.0
    cfg.getEventAlgo('TgcRawDataMonAlg').ResidualWindow = 200.
    cfg.getEventAlgo('TgcRawDataMonAlg').nHitsInOtherTGCWire = 3
    cfg.getEventAlgo('TgcRawDataMonAlg').nHitsInOtherTGCStrip = 2
    cfg.getEventAlgo('TgcRawDataMonAlg').MaskChannelFileName = 'tgc_mask_channels.txt'
    cfg.getEventAlgo('TgcRawDataMonAlg').TagMuonInDifferentSystem = False
    cfg.getEventAlgo('TgcRawDataMonAlg').StreamerFilter = '' # such as 'HLT_noalg_L1MU14FCH'
    cfg.getEventAlgo('TgcRawDataMonAlg').doExpressProcessing = False

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    cfg.merge(AtlasFieldCacheCondAlgCfg(flags))

    if not flags.Input.isMC:
        from AthenaConfiguration.ComponentFactory import CompFactory
        cfg.getEventAlgo('TgcRawDataMonAlg').GRLTool = CompFactory.GoodRunsListSelectorTool('GoodRunsListSelectorTool')
        cfg.getEventAlgo('TgcRawDataMonAlg').GRLTool.GoodRunsListVec = ['data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml']

    flags_dummy = initConfigFlags()
    flags_dummy.Input.Files = flags.Input.Files
    flags_dummy.Trigger.triggerConfig = "FILE"
    flags_dummy.Trigger.triggerMenuSetup = "Dev_pp_run3_v1"
    flags_dummy.lock()
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg,generateL1Menu
    cfg.merge( L1ConfigSvcCfg( flags_dummy ) )
    generateL1Menu(flags_dummy)

    cfg.printConfig(withDetails=False, summariseProps = False)

    cfg.run()
