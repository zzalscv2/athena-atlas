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

    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloOnlyMVACfg
    from TrigTauHypo.TrigTauHypoConfig import tauCaloRoiUpdaterCfg

    recoAcc.mergeReco(tauCaloRoiUpdaterCfg(flags,inputRoIs=recoAcc.inputMaker().InViewRoIs,clusters = 'HLT_TopoCaloClustersLC'))

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
def _ftfCoreSeq(flags,name,is_probe_leg=False):                                                                                                                                                                 
    selAcc=SelectionCA('tau'+name+'FTF', isProbe=is_probe_leg)

    newRoITool   = CompFactory.ViewCreatorFetchFromViewROITool( 
                                RoisWriteHandleKey = recordable(flags.Tracking.ActiveConfig.roi),
                                InViewRoIs = 'UpdatedCaloRoI')                                                                                                

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching

    extraPrefetching = ROBPrefetching.TauCoreLargeRoI in flags.Trigger.ROBPrefetchingOptions
    if extraPrefetching:
      # Add extra RoI to prefetch ROBs for the subsequent tauIso step together with ROBs for tauCore
      prefetchRoIUpdater                   = CompFactory.RoiUpdaterTool()
      prefetchRoIUpdater.useBeamSpot       = True
      prefetchRoIUpdater.NSigma            = 1.5
      prefetchRoIUpdater.EtaWidth          = flags.Trigger.InDetTracking.tauIso.etaHalfWidth
      prefetchRoIUpdater.PhiWidth          = flags.Trigger.InDetTracking.tauIso.phiHalfWidth
      prefetchRoIUpdater.ZedWidth          = flags.Trigger.InDetTracking.tauIso.zedHalfWidth
      prefetchRoITool                      = CompFactory.ViewCreatorExtraPrefetchROITool()
      prefetchRoITool.RoiCreator           = newRoITool
      prefetchRoITool.RoiUpdater           = prefetchRoIUpdater
      prefetchRoITool.ExtraPrefetchRoIsKey = str(newRoITool.RoisWriteHandleKey) + "_forPrefetching"
      prefetchRoITool.PrefetchRoIsLinkName = "prefetchRoI"
      prefetchRoITool.MergeWithOriginal    = True                                                      

    fastInDetReco = InViewRecoCA('FastTau'+name,RoITool           = prefetchRoITool if extraPrefetching else newRoITool,
                                                ViewFallThrough   = True,
                                                RequireParentView = True,
                                                mergeUsingFeature = True,
                                                isProbe           = is_probe_leg)

    robPrefetchAlg = ROBPrefetchingAlgCfg_Si( flags, nameSuffix='IM_'+fastInDetReco.name)
    if extraPrefetching:
      robPrefetchAlg.RoILinkName = str(prefetchRoITool.PrefetchRoIsLinkName) 
      print('Anaxagoras: ',robPrefetchAlg.RoILinkName)

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    fastInDetReco.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName='tau'+name))
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVFastTau'+name,
                                DataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                               ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')]) )

    RoIs = fastInDetReco.inputMaker().InViewRoIs
    TrackCollection = flags.Tracking.ActiveConfig.trkTracks_FTF

    from TrigTauHypo.TrigTauHypoConfig import tauTrackRoiUpdaterCfg,tauTrackBDTRoiUpdaterCfg,tauLRTRoiUpdaterCfg

    if 'LRT' in name:
       fastInDetReco.mergeReco(tauLRTRoiUpdaterCfg(flags,inputRoIs = RoIs,tracks = TrackCollection))
    else:
       fastInDetReco.mergeReco(tauTrackRoiUpdaterCfg(flags,inputRoIs = RoIs,tracks = TrackCollection))
       fastInDetReco.mergeReco(tauTrackBDTRoiUpdaterCfg(flags,inputRoIs = RoIs,tracks = TrackCollection))

    selAcc.mergeReco(fastInDetReco)
    selAcc.mergeReco(fastInDetReco, robPrefetchCA=robPrefetchAlg)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg('TrackPreSelHypoAlg_PassBy'+name,
                                                 RoIForIDReadHandleKey = 'UpdatedTrackLRTRoI' if 'LRT' in name else '',
                                                 trackcollection       = flags.Tracking.ActiveConfig.trkTracks_FTF )
    selAcc.addHypoAlgo(hypoAlg)
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)

def tauFTFTauCoreSeq(flags, is_probe_leg=False):
    newflags = flags.cloneAndReplace('Tracking.ActiveConfig','Trigger.InDetTracking.tauCore')
    name='Core'
    (selAcc , menuCA) = _ftfCoreSeq(newflags,name,is_probe_leg)
    return menuCA 

def tauFTFTauLRTSeq(flags, is_probe_leg=False):
    newflags = flags.cloneAndReplace('Tracking.ActiveConfig','Trigger.InDetTracking.tauLRT')
    name='LRT'
    (selAcc , menuCA) = _ftfCoreSeq(newflags,name,is_probe_leg)
    return menuCA 

@AccumulatorCache
def _ftfTauIsoSeq(flags,name,is_probe_leg=False):
    selAcc=SelectionCA('tau'+name+'FTF', isProbe=is_probe_leg)

    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
                             RoisWriteHandleKey = recordable(flags.Tracking.ActiveConfig.roi),
                             InViewRoIs = 'UpdatedTrackBDTRoI' if 'BDT' in name else 'UpdatedTrackRoI')

    fastInDetReco = InViewRecoCA('FastTau'+name,RoITool           = newRoITool,
                                                RequireParentView = True,
                                                ViewFallThrough   = True,
                                                isProbe           = is_probe_leg)

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    idTracking = trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName='tau'+name)
    fastInDetReco.mergeReco(idTracking)
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVFastTau'+name,
                                DataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                               ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')]) )

    selAcc.mergeReco(fastInDetReco)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg('TrackPreSelHypoAlg_PassBy'+name,
                                                    trackcollection = flags.Tracking.ActiveConfig.trkTracks_FTF )

    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)

def tauFTFTauIsoSeq(flags, is_probe_leg=False):
    newflags = flags.cloneAndReplace('Tracking.ActiveConfig','Trigger.InDetTracking.tauIso')
    name = 'Iso'
    (selAcc , menuCA) = _ftfTauIsoSeq(newflags,name,is_probe_leg)
    return menuCA

def tauFTFTauIsoBDTSeq(flags, is_probe_leg=False):
    newflags = flags.cloneAndReplace('Tracking.ActiveConfig','Trigger.InDetTracking.tauIsoBDT')
    name = 'IsoBDT'
    (selAcc , menuCA) = _ftfTauIsoSeq(newflags,name,is_probe_leg)
    return menuCA
