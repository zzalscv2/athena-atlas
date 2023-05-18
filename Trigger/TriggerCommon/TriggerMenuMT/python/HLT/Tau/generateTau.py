# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDMRun3 import recordable

@AccumulatorCache
def _caloSeq(flags, is_probe_leg=False):
    selAcc = SelectionCA('CaloTau', isProbe=is_probe_leg)

    recoAcc = InViewRecoCA(name       = 'tauCaloMVA', 
                           InViewRoIs = 'CaloMVA_RoIs',
                           isProbe    = is_probe_leg)

    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name=recoAcc.name+'RecoVDV',
                                                                  DataObjects=[('TrigRoiDescriptorCollection', 'StoreGateSvc+'+recoAcc.inputMaker().InViewRoIs.Path),
                                                                               #( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+HLT_TAURoI'),
                                                                               ('CaloBCIDAverage', 'StoreGateSvc+CaloBCIDAverage'),
                                                                               ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                                                               ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.actualInteractionsPerCrossing'), 
                                                                               ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing')]))

    from TrigCaloRec.TrigCaloRecConfig import tauTopoClusteringCfg
    recoAcc.mergeReco(tauTopoClusteringCfg(flags,
                                           RoIs = recoAcc.inputMaker().InViewRoIs))

    recoAcc.addRecoAlgo(CompFactory.TrigTauCaloRoiUpdater("TauCaloRoiUpdater",
                                                                CaloClustersKey = 'HLT_TopoCaloClustersLC',
                                                                RoIInputKey = recoAcc.inputMaker().InViewRoIs,
                                                                RoIOutputKey = 'UpdatedCaloRoI'))

    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloOnlyMVACfg
    recoAcc.mergeReco(trigTauRecMergedCaloOnlyMVACfg(flags))
    selAcc.mergeReco(recoAcc)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix='IM_'+recoAcc.name+'_probe' if is_probe_leg else 'IM_'+recoAcc.name)    
    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)

    hypoAlg = CompFactory.TrigTauCaloHypoAlg("TauL2CaloMVAHypo",
                                                    taujets = "HLT_TrigTauRecMerged_CaloMVAOnly" )
    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigL2TauHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigL2TauHypoToolFromDict,isProbe=is_probe_leg)   
    return (selAcc , menuCA)


def tauCaloMVAMenuSeq(flags, name, is_probe_leg=False):
    (selAcc , menuCA) = _caloSeq(flags, is_probe_leg)
    return menuCA 


@AccumulatorCache
def _ftfCoreSeq(flags):                                                                                                                                                                 
    selAcc=SelectionCA('tauCoreFTF')
    newRoITool   = CompFactory.ViewCreatorFetchFromViewROITool( RoisWriteHandleKey = recordable( flags.Trigger.InDetTracking.tauCore.roi ),
                                                                           InViewRoIs = 'UpdatedCaloRoI')                                                                                                                                                      
    fastInDetReco = InViewRecoCA('FastTauCore', RoIsLink          = 'UpdatedCaloRoI',
                                                RoITool           = newRoITool,
                                                RequireParentView = True,
                                                mergeUsingFeature = True)

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    fastInDetReco.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName="tauCore"))
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVFastTauCore',
                                DataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                               ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')]) )

    fastInDetReco.addRecoAlgo(CompFactory.TrigTauTrackRoiUpdater('TrackRoiUpdater',
                                                               RoIInputKey                  = fastInDetReco.inputMaker().InViewRoIs,
                                                               RoIOutputKey                 = 'UpdatedTrackRoI',
                                                               fastTracksKey                = flags.Trigger.InDetTracking.tauCore.trkTracks_FTF,
                                                               Key_trigTauJetInputContainer = "" ))
    fastInDetReco.addRecoAlgo(CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterBDT",
                                                               RoIInputKey                  = fastInDetReco.inputMaker().InViewRoIs,
                                                               RoIOutputKey                 = "UpdatedTrackBDTRoI",
                                                               fastTracksKey                = flags.Trigger.InDetTracking.tauCore.trkTracks_FTF,
                                                               BDTweights                   = f"{flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath}/{flags.Trigger.Offline.Tau.FTFTauCoreBDTConfig}",
                                                               Key_trigTauJetInputContainer = "HLT_TrigTauRecMerged_CaloMVAOnly" ))
    selAcc.mergeReco(fastInDetReco)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_RejectEmpty",
                                                    trackcollection = flags.Trigger.InDetTracking.tauCore.trkTracks_FTF )
    selAcc.addHypoAlgo(hypoAlg)
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict)
    return (selAcc , menuCA)

def tauFTFTauCoreSeq(flags, is_probe_leg=False):
    (selAcc , menuCA) = _ftfCoreSeq(flags)
    return menuCA 

@AccumulatorCache
def _ftfIsoSeq(flags):
    selAcc=SelectionCA('tauIsoFTF')

    newRoITool   = CompFactory.ViewCreatorFetchFromViewROITool(RoisWriteHandleKey = recordable(flags.Trigger.InDetTracking.tauIso.roi),
                                                                           InViewRoIs = 'UpdatedTrackRoI')                                                                                                                        

    fastInDetReco = InViewRecoCA('FastTauIso', RoITool           = newRoITool,
                                               RequireParentView = True,
                                               ViewFallThrough   = True )

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    idTracking = trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName="tauIso")
    fastInDetReco.mergeReco(idTracking)
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVFastTauIso',
                                DataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                               ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')]) )

    selAcc.mergeReco(fastInDetReco)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByIso",
                                                    trackcollection = flags.Trigger.InDetTracking.tauIso.trkTracks_FTF )

    from TrigTauHypo.TrigTauHypoConf import TrigTrackPreSelHypoTool
    TrigTrackPreSelHypoTool.AcceptAll = True

    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict)
    return (selAcc , menuCA)

def tauFTFTauIsoSeq(flags, is_probe_leg=False):
    (selAcc , menuCA) = _ftfIsoSeq(flags)
    return menuCA

@AccumulatorCache
def _ftfIsoBDTSeq(flags):
    selAcc=SelectionCA('tauIsoBDTFTF')

    newRoITool   = CompFactory.ViewCreatorFetchFromViewROITool(RoisWriteHandleKey = recordable(flags.Trigger.InDetTracking.tauIsoBDT.roi),
                                                                   InViewRoIs = 'UpdatedTrackBDTRoI')

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    fastInDetReco = InViewRecoCA('FastTauIsoBDT',   RoITool           = newRoITool,
                                                        RequireParentView = True,
                                                        mergeUsingFeature = True )
    idTracking = trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName="tauIsoBDT")
    fastInDetReco.mergeReco(idTracking)
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVFastTauIsoBDT',
                              DataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')]) )

    selAcc.mergeReco(fastInDetReco)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByIsoBDT",
                                                    trackcollection = flags.Trigger.InDetTracking.tauIsoBDT.trkTracks_FTF )

    from TrigTauHypo.TrigTauHypoConf import TrigTrackPreSelHypoTool
    TrigTrackPreSelHypoTool.AcceptAll = True

    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict)
    return (selAcc , menuCA)

def tauFTFTauIsoBDTSeq(flags, is_probe_leg=False):
    (selAcc , menuCA) = _ftfIsoBDTSeq(flags)
    return menuCA
