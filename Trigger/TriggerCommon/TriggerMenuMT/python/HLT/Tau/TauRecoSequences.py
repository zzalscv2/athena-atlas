#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.CFElements import parOR, seqAND
from AthenaCommon.GlobalFlags import globalflags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool, ViewCreatorFetchFromViewROITool, ViewCreatorPreviousROITool
from TrigT2CaloCommon.CaloDef import HLTLCTopoRecoSequence
from TrigEDMConfig.TriggerEDMRun3 import recordable
from TriggerMenuMT.HLT.Config.MenuComponents import RecoFragmentsPool, algorithmCAToGlobalWrapper
from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si, ROBPrefetchingAlgCfg_Calo
from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
import AthenaCommon.CfgMgr as CfgMgr


#Retrieve short name for tau signature that can be used as suffix to be appended to the names of alg/tools
#Based on these names specific ID config is retrieved
#This utilizes name of the reco sequence from which checks specific string pattern
def _getTauSignatureShort( name ):
    signature = ""
    if "LRT" in name:
      signature = 'tauLRT'
      signatureID = 'tauLRT'
    elif "FTFCore" in name:
      signature = 'tauCore'
      signatureID = 'tauCore'
    elif "IsoInView" in name:
      signature = 'tauIso'
      signatureID = 'tauIso'
    elif "IsoBDT" in name:
      signature = 'tauIsoBDT'
      signatureID = 'tauIsoBDT'
    elif "MVA" in name:
      signature = 'tauMVA'
      signatureID = 'tauIso'
    elif "LLP" in name:
      signature = 'tauLLP'
      signatureID = 'tauIso'
    else:
      raise Exception( "getTauSignatureShort() called with incorrect non existent slice: "+name )
      return None

    return signature, signatureID

@AccumulatorCache
def tauCaloRoiUpdaterCfg(flags, inputRoIs, clusters):
    acc = ComponentAccumulator()
    alg                               = CompFactory.TrigTauCaloRoiUpdater("TauCaloRoiUpdater",
                                        RoIInputKey     = inputRoIs,
                                        RoIOutputKey    = 'UpdatedCaloRoI',
                                        CaloClustersKey = clusters)
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauTrackRoiUpdaterCfg(inflags, inputRoIs, tracks):
    acc = ComponentAccumulator()
    flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking.tauIso")
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdater",
                                        etaHalfWidth                  = flags.InDet.Tracking.ActiveConfig.etaHalfWidth,
                                        phiHalfWidth                  = flags.InDet.Tracking.ActiveConfig.phiHalfWidth,
                                        z0HalfWidth                   = flags.InDet.Tracking.ActiveConfig.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauTrackBDTRoiUpdaterCfg(inflags, inputRoIs, tracks):
    acc                               = ComponentAccumulator()
    flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking.tauIso")
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterBDT",
                                        etaHalfWidth                  = flags.InDet.Tracking.ActiveConfig.etaHalfWidth,
                                        phiHalfWidth                  = flags.InDet.Tracking.ActiveConfig.phiHalfWidth,
                                        z0HalfWidth                   = flags.InDet.Tracking.ActiveConfig.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackBDTRoI",
                                        fastTracksKey                 = tracks,
                                        BDTweights                    = f"{flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath}/{flags.Trigger.Offline.Tau.FTFTauCoreBDTConfig}",
                                        Key_trigTauJetInputContainer  = "HLT_TrigTauRecMerged_CaloMVAOnly" )
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauLRTRoiUpdaterCfg(inflags, inputRoIs, tracks):
    acc                               = ComponentAccumulator()
    flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking.tauLRT")
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterLRT",
                                        etaHalfWidth                  = flags.InDet.Tracking.ActiveConfig.etaHalfWidth,
                                        phiHalfWidth                  = flags.InDet.Tracking.ActiveConfig.phiHalfWidth,
                                        z0HalfWidth                   = flags.InDet.Tracking.ActiveConfig.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackLRTRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc

def _algoTauPrecision(flags, name, inputRoIs, tracks):
    from TrigTauRec.TrigTauRecConfig import TrigTauRecMerged_TauPrecisionMVA
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

    if "MVA" in name:
      algo                                 = TrigTauRecMerged_TauPrecisionMVA(flags, name= "TrigTauRecMerged_TauPrecision_PrecisionMVA", doTrackBDT=False, doLLP=False)
      algo.Key_trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_MVA")
      algo.Key_trigTauTrackOutputContainer = recordable("HLT_tautrack_MVA")
    elif "LLP" in name:
      algo                                 = TrigTauRecMerged_TauPrecisionMVA(flags, name= "TrigTauRecMerged_TauPrecision_PrecisionLLP", doTrackBDT=False, doLLP=True)
      algo.Key_trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_LLP")
      algo.Key_trigTauTrackOutputContainer = recordable("HLT_tautrack_LLP")
    elif "LRT" in name:
      algo                                 = TrigTauRecMerged_TauPrecisionMVA(flags, name= "TrigTauRecMerged_TauPrecision_PrecisionLRT", doTrackBDT=False, doLLP=True)
      algo.Key_trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_LRT")
      algo.Key_trigTauTrackOutputContainer = recordable("HLT_tautrack_LRT")
    else:
      raise Exception( "_algoTauPrecision : called with incorrect non existent name: "+name )
      return None

    algo.Key_trigTauTrackInputContainer  = "HLT_tautrack_dummy"
    algo.Key_trigTauJetInputContainer    = "HLT_TrigTauRecMerged_CaloMVAOnly"
    algo.Key_trigJetSeedOutputKey        = recordable("HLT_jet_seed")

    algo.RoIInputKey                     = inputRoIs
    algo.clustersKey                     = ""
    algo.Key_vertexInputContainer        = getInDetTrigConfig( "tauIso" ).vertex
    algo.Key_trackPartInputContainer     = tracks

    return algo

def tauCaloMVARecoSequence(flags, InViewRoIs, SeqName):
    global TauCaloJetContainer
    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloOnlyMVACfg
    # lc sequence
    (lcTopoInViewSequence, lcCaloSequenceOut) = RecoFragmentsPool.retrieve(HLTLCTopoRecoSequence, flags, RoIs=InViewRoIs)
    tauCaloRoiUpdaterAlg                      = algorithmCAToGlobalWrapper(tauCaloRoiUpdaterCfg,flags, inputRoIs=InViewRoIs,clusters = lcCaloSequenceOut)[0]
    tauCaloOnlyMVAAlg                         = algorithmCAToGlobalWrapper(trigTauRecMergedCaloOnlyMVACfg,flags)[0]
    RecoSequence                              = parOR( SeqName, [lcTopoInViewSequence,tauCaloRoiUpdaterAlg,tauCaloOnlyMVAAlg] )
    return (RecoSequence, tauCaloOnlyMVAAlg.Key_trigTauJetOutputContainer)

def tauCaloMVASequence(flags):
    """ Creates L2 Fast Calo sequence for Taus"""
    # EV creator
    InViewRoIs                              = "CaloMVA_RoIs"
    RecoSequenceName                        = "tauCaloMVAInViewSequence"

    tauCaloMVAViewsMaker                    = EventViewCreatorAlgorithm( "IMtauCaloMVA")
    tauCaloMVAViewsMaker.ViewFallThrough    = True
    tauCaloMVAViewsMaker.RoIsLink           = "initialRoI"
    tauCaloMVAViewsMaker.RoITool            = ViewCreatorInitialROITool()
    tauCaloMVAViewsMaker.InViewRoIs         = InViewRoIs
    tauCaloMVAViewsMaker.Views              = "TAUCaloMVAViews"
    tauCaloMVAViewsMaker.ViewNodeName       = RecoSequenceName
    (tauCaloMVAInViewSequence, sequenceOut) = tauCaloMVARecoSequence(flags, InViewRoIs, RecoSequenceName)

    tauCaloMVARecoVDV = CfgMgr.AthViews__ViewDataVerifier( "tauCaloMVARecoVDV" )
    tauCaloMVARecoVDV.DataObjects = [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s'%(InViewRoIs)),
                                     ( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                                     ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                     ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.actualInteractionsPerCrossing' ),
                                     ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' )]
    tauCaloMVAInViewSequence += tauCaloMVARecoVDV

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Calo, flags, nameSuffix=tauCaloMVAViewsMaker.name())[0]

    tauCaloMVASequence = seqAND("tauCaloMVASequence", [tauCaloMVAViewsMaker, robPrefetchAlg, tauCaloMVAInViewSequence ])
    return (tauCaloMVASequence, tauCaloMVAViewsMaker, sequenceOut)

def tauIdSequence(flags, RoIs, name):

    tauIdSequence = parOR(name)

    signatureName, signatureNameID = _getTauSignatureShort( name )
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( signatureNameID )

    ViewVerifyId = CfgMgr.AthViews__ViewDataVerifier("tauIdViewDataVerifier_"+signatureName)
    ViewVerifyId.DataObjects = [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs      ),
                                ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing'   ),
                                ( 'xAOD::VertexContainer', 'StoreGateSvc+'+getInDetTrigConfig( "tauIso" ).vertex),
                                ( 'xAOD::TauTrackContainer' , 'StoreGateSvc+HLT_tautrack_dummy' ),
                                ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly' ),
                                ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+IDTrigConfig.tracks_IDTrig() )]

    tauIdSequence+= ViewVerifyId

    tauPrecisionAlg = _algoTauPrecision(flags, name, inputRoIs = RoIs, tracks = IDTrigConfig.tracks_IDTrig())

    tauIdSequence += tauPrecisionAlg

    sequenceOut = tauPrecisionAlg.Key_trigTauJetOutputContainer

    return tauIdSequence, sequenceOut


def precTrackSequence( flags, RoIs , name):

    signatureName, signatureNameID = _getTauSignatureShort( name )
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( signatureNameID )

    ViewVerifyTrk = CfgMgr.AthViews__ViewDataVerifier("tauViewDataVerifier_"+signatureName)
    ViewVerifyTrk.DataObjects = [( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+%s' % IDTrigConfig.tracks_FTF() ),
                                 ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                                 ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                                 ( 'xAOD::TauTrackContainer' , 'StoreGateSvc+HLT_tautrack_dummy' ),
                                 ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly' ),    
                                 ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_FlaggedCondData_TRIG' ),
                                 ( 'xAOD::IParticleContainer' , 'StoreGateSvc+%s' % IDTrigConfig.tracks_FTF() )]

    # Make sure the required objects are still available at whole-event level
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    if globalflags.InputFormat.is_bytestream():
      ViewVerifyTrk.DataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
                                 ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ) ]
    else:
      topSequence.SGInputLoader.Load += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
      ViewVerifyTrk.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]

    #Precision Tracking
    PTAlgs = [] #List of precision tracking algs 
    PTTracks = [] #List of TrackCollectionKeys
    PTTrackParticles = [] #List of TrackParticleKeys
    
    from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking
    #When run in a different view than FTF some data dependencies needs to be loaded through verifier
    #Pass verifier as an argument and it will automatically append necessary DataObjects@NOTE: Don't provide any verifier if loaded in the same view as FTF
    PTTracks, PTTrackParticles, PTAlgs = makeInDetTrigPrecisionTracking( flags, config = IDTrigConfig, verifier = ViewVerifyTrk, rois = RoIs )

    from TrigInDetConfig.InDetTrigVertices import makeInDetTrigVertices
    vtxAlg = makeInDetTrigVertices( flags,
                                    whichSignature       = signatureName,
                                    inputTrackCollection = IDTrigConfig.tracks_IDTrig(),
                                    outputVtxCollection  = IDTrigConfig.vertex,
                                    config               = IDTrigConfig,
                                    adaptiveVertex       = IDTrigConfig.adaptiveVertex )

    trackSequence = parOR(name, [ViewVerifyTrk] + PTAlgs + vtxAlg )

    #Get last tracks from the list as input for other alg       
    sequenceOut = PTTrackParticles[-1]

    return trackSequence, sequenceOut

def tauFTFSequence( flags, RoIs, name ):

    tauFTFSequence = parOR(name)

    signatureName, signatureNameID = _getTauSignatureShort( name )
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( signatureNameID )

    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
    viewAlgs, viewVerify = makeInDetTrigFastTracking( flags, config = IDTrigConfig, rois = RoIs )

    TrackCollection = IDTrigConfig.trkTracks_FTF()

    viewVerify.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                               ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')] 

    if 'LRT' in signatureName:
      tauLRTRoiUpdaterAlg = algorithmCAToGlobalWrapper(tauLRTRoiUpdaterCfg,flags,inputRoIs = RoIs,tracks = TrackCollection)[0]
      viewAlgs.append(tauLRTRoiUpdaterAlg)
    elif 'Core' in signatureName:
      tauTrackRoiUpdaterAlg = algorithmCAToGlobalWrapper(tauTrackRoiUpdaterCfg,flags,inputRoIs = RoIs,tracks = TrackCollection)[0]
      tauTrackRoiUpdaterAlgBDT = algorithmCAToGlobalWrapper(tauTrackBDTRoiUpdaterCfg,flags,inputRoIs = RoIs,tracks = TrackCollection)[0]
      viewAlgs.append(tauTrackRoiUpdaterAlgBDT)
      viewAlgs.append(tauTrackRoiUpdaterAlg)

    tauFTFSequence += viewAlgs

    sequenceOut = TrackCollection

    return tauFTFSequence, sequenceOut

# ===============================================================================================                                                           
#   Reco sequence for FTFTauCore + TrackRoIUpdater Alg (tracktwoMVA)                                                                  
# ===============================================================================================  

def tauFTFCoreSequence(flags):

    RecoSequenceName                    = "tauFTFCoreInViewSequence"

    newRoITool                          = ViewCreatorFetchFromViewROITool()
    newRoITool.RoisWriteHandleKey       = recordable("HLT_Roi_TauCore") #RoI collection recorded to EDM           
    newRoITool.InViewRoIs               = "UpdatedCaloRoI" #input RoIs from calo only step   

    extraPrefetching = ROBPrefetching.TauCoreLargeRoI in flags.Trigger.ROBPrefetchingOptions
    if extraPrefetching:
      # Add extra RoI to prefetch ROBs for the subsequent tauIso step together with ROBs for tauCore
      from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
      tauIsoConfig = getInDetTrigConfig("tauIso")
      prefetchRoIUpdater                   = CompFactory.RoiUpdaterTool()
      prefetchRoIUpdater.useBeamSpot       = True
      prefetchRoIUpdater.NSigma            = 1.5
      prefetchRoIUpdater.EtaWidth          = tauIsoConfig.etaHalfWidth
      prefetchRoIUpdater.PhiWidth          = tauIsoConfig.phiHalfWidth
      prefetchRoIUpdater.ZedWidth          = tauIsoConfig.zedHalfWidth
      prefetchRoITool                      = CompFactory.ViewCreatorExtraPrefetchROITool()
      prefetchRoITool.RoiCreator           = newRoITool
      prefetchRoITool.RoiUpdater           = prefetchRoIUpdater
      prefetchRoITool.ExtraPrefetchRoIsKey = str(newRoITool.RoisWriteHandleKey) + "_forPrefetching"
      prefetchRoITool.PrefetchRoIsLinkName = "prefetchRoI"
      prefetchRoITool.MergeWithOriginal    = True

    ftfCoreViewsMaker                   = EventViewCreatorAlgorithm("IMFTFCore")
    ftfCoreViewsMaker.mergeUsingFeature = True
    ftfCoreViewsMaker.RoITool           = prefetchRoITool if extraPrefetching else newRoITool
    ftfCoreViewsMaker.InViewRoIs        = "RoiForTauCore"
    ftfCoreViewsMaker.Views             = "TAUFTFCoreViews"
    ftfCoreViewsMaker.ViewFallThrough   = True
    ftfCoreViewsMaker.RequireParentView = True
    ftfCoreViewsMaker.ViewNodeName      = RecoSequenceName

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=ftfCoreViewsMaker.name())[0]
    if extraPrefetching:
      robPrefetchAlg.RoILinkName = str(prefetchRoITool.PrefetchRoIsLinkName)

    (tauFTFCoreInViewSequence, sequenceOut) = tauFTFSequence( flags, ftfCoreViewsMaker.InViewRoIs, RecoSequenceName)

    tauFastTrackCoreSequence = seqAND("tauFastTrackCoreSequence", [ftfCoreViewsMaker, robPrefetchAlg, tauFTFCoreInViewSequence])
    return (tauFastTrackCoreSequence, ftfCoreViewsMaker, sequenceOut)

# ===============================================================================================
#   Reco sequence for FTFTauLRT + TrackRoIUpdater Alg
# ===============================================================================================

def tauFTFLRTSequence(flags):

    RecoSequenceName                    = "tauFTFLRTInViewSequence"

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    config = getInDetTrigConfig("tauLRT")
    newRoITool                          = ViewCreatorFetchFromViewROITool()
    newRoITool.RoisWriteHandleKey       = recordable("HLT_Roi_TauLRT") #RoI collection recorded to EDM
    newRoITool.InViewRoIs               = "UpdatedCaloRoI" #input RoIs from calo only step
    newRoITool.doResize                 = True
    newRoITool.RoIEtaWidth              = config.etaHalfWidth
    newRoITool.RoIPhiWidth              = config.phiHalfWidth
    newRoITool.RoIZedWidth              = config.zedHalfWidth

    ftfLRTViewsMaker                   = EventViewCreatorAlgorithm("IMFTFLRT")
    ftfLRTViewsMaker.mergeUsingFeature = True
    ftfLRTViewsMaker.RoITool           = newRoITool
    ftfLRTViewsMaker.InViewRoIs        = "RoiForTauLRT"
    ftfLRTViewsMaker.Views             = "TAUFTFLRTViews"
    ftfLRTViewsMaker.ViewFallThrough   = True
    ftfLRTViewsMaker.RequireParentView = True
    ftfLRTViewsMaker.ViewNodeName      = RecoSequenceName

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=ftfLRTViewsMaker.name())[0]

    (tauFTFLRTInViewSequence, sequenceOut) = tauFTFSequence( flags, ftfLRTViewsMaker.InViewRoIs, RecoSequenceName)

    tauFastTrackLRTSequence = seqAND("tauFastTrackLRTSequence", [ftfLRTViewsMaker, robPrefetchAlg, tauFTFLRTInViewSequence])
    return (tauFastTrackLRTSequence, ftfLRTViewsMaker, sequenceOut)

# ===============================================================================================                                                          
#   Reco sequence for FTFTauIso (tracktwoMVA)                                                                  
# ===============================================================================================  

def tauFTFIsoSequence(flags):

    RecoSequenceName                   = "tauFTFIsoInViewSequence"

    newRoITool                         = ViewCreatorFetchFromViewROITool()
    newRoITool.RoisWriteHandleKey      = recordable("HLT_Roi_TauIso") #RoI collection recorded to EDM
    newRoITool.InViewRoIs              = "UpdatedTrackRoI" #input RoIs from calo only step

    ftfIsoViewsMaker                   = EventViewCreatorAlgorithm("IMFTFIso")
    ftfIsoViewsMaker.RoIsLink          = "roi"
    ftfIsoViewsMaker.RoITool           = newRoITool
    ftfIsoViewsMaker.InViewRoIs        = "RoiForTauIso"
    ftfIsoViewsMaker.Views             = "TAUFTFIsoViews"
    ftfIsoViewsMaker.ViewFallThrough   = True
    ftfIsoViewsMaker.RequireParentView = True
    ftfIsoViewsMaker.ViewNodeName      = RecoSequenceName

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=ftfIsoViewsMaker.name())[0]

    (tauFTFIsoInViewSequence, sequenceOut) = tauFTFSequence( flags, ftfIsoViewsMaker.InViewRoIs, RecoSequenceName)

    tauFastTrackIsoSequence = seqAND("tauFastTrackIsoSequence", [ftfIsoViewsMaker, robPrefetchAlg, tauFTFIsoInViewSequence])
    return (tauFastTrackIsoSequence, ftfIsoViewsMaker, sequenceOut)

# ===============================================================================================                                                                                                  
#   Reco sequence for FTFTauIsoBDT (tracktwoMVABDT)                                                                                                                                                
# ===============================================================================================                                                                                                  

def tauFTFIsoBDTSequence(flags):

    RecoSequenceName                   = "tauFTFIsoBDTInViewSequence"

    newRoITool                         = ViewCreatorFetchFromViewROITool()
    newRoITool.RoisWriteHandleKey      = recordable("HLT_Roi_TauIsoBDT") #RoI collection recorded to EDM                                                                                           
    newRoITool.InViewRoIs              = "UpdatedTrackBDTRoI" #input RoIs from calo only step                                                                                                      

    ftfIsoViewsMaker                   = EventViewCreatorAlgorithm("IMFTFIsoBDT")
    ftfIsoViewsMaker.RoIsLink          = "roi"
    ftfIsoViewsMaker.RoITool           = newRoITool
    ftfIsoViewsMaker.InViewRoIs        = "RoiForTauIsoBDT"
    ftfIsoViewsMaker.Views             = "TAUFTFIsoBDTViews"
    ftfIsoViewsMaker.ViewFallThrough   = True
    ftfIsoViewsMaker.RequireParentView = True
    ftfIsoViewsMaker.ViewNodeName      = RecoSequenceName

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=ftfIsoViewsMaker.name())[0]

    (tauFTFIsoBDTInViewSequence, sequenceOut) = tauFTFSequence( flags, ftfIsoViewsMaker.InViewRoIs, RecoSequenceName)

    tauFastTrackIsoBDTSequence = seqAND("tauFastTrackIsoBDTSequence", [ftfIsoViewsMaker, robPrefetchAlg, tauFTFIsoBDTInViewSequence])
    return (tauFastTrackIsoBDTSequence, ftfIsoViewsMaker, sequenceOut)

# ===============================================================================================                                                            
#   Reco sequence for Precision tracking (from FTF Iso algorithm)   (tracktwoMVA)                           
# ===============================================================================================                                                            

def tauPrecIsoTrackSequence(flags):

    RecoSequenceName                       = "precFTFIsoInViewSequence"

    tauPrecIsoViewsMaker                   = EventViewCreatorAlgorithm("IMPrecIsoTrack")
    tauPrecIsoViewsMaker.RoIsLink          = "roi"
    tauPrecIsoViewsMaker.RoITool           = ViewCreatorPreviousROITool()
    tauPrecIsoViewsMaker.InViewRoIs        = "RoiForTauIso"
    tauPrecIsoViewsMaker.Views             = "TAUPrecIsoViews"
    tauPrecIsoViewsMaker.ViewFallThrough   = True
    tauPrecIsoViewsMaker.RequireParentView = True
    tauPrecIsoViewsMaker.ViewNodeName      = RecoSequenceName

    (tauPrecIsoTrackInViewSequence, sequenceOut) = precTrackSequence(flags, tauPrecIsoViewsMaker.InViewRoIs, RecoSequenceName)

    tauPrecIsoTrkSequence = seqAND("tauPrecIsoTrkSequence", [tauPrecIsoViewsMaker, tauPrecIsoTrackInViewSequence ])
    return (tauPrecIsoTrkSequence, tauPrecIsoViewsMaker, sequenceOut)

# ===============================================================================================
#   Reco sequence for Precision tracking (from FTF LRT algorithm)
# ===============================================================================================

def tauPrecLRTTrackSequence(flags):

    RecoSequenceName                       = "precFTFLRTInViewSequence"

    tauPrecLRTViewsMaker                   = EventViewCreatorAlgorithm("IMPrecLRTTrack")
    tauPrecLRTViewsMaker.RoIsLink          = "roi"
    tauPrecLRTViewsMaker.RoITool           = ViewCreatorPreviousROITool()
    tauPrecLRTViewsMaker.InViewRoIs        = "RoiForTauLRT"
    tauPrecLRTViewsMaker.Views             = "TAUPrecLRTViews"
    tauPrecLRTViewsMaker.ViewFallThrough   = True
    tauPrecLRTViewsMaker.RequireParentView = True
    tauPrecLRTViewsMaker.ViewNodeName      = RecoSequenceName

    (tauPrecLRTTrackInViewSequence, sequenceOut) = precTrackSequence(flags, tauPrecLRTViewsMaker.InViewRoIs, RecoSequenceName)

    tauPrecLRTTrkSequence = seqAND("tauPrecLRTTrkSequence", [tauPrecLRTViewsMaker, tauPrecLRTTrackInViewSequence ])
    return (tauPrecLRTTrkSequence, tauPrecLRTViewsMaker, sequenceOut)

# ===============================================================================================                                                            
#    Reco sequence for Tau Precision MVA Alg (tracktwoMVA)                                                                                 
# ===============================================================================================                                                            

def tauMVASequence(flags):

    RecoSequenceName = "tauMVAInViewSequence"

    mvaViewsMaker                   = EventViewCreatorAlgorithm("IMTauMVA")
    mvaViewsMaker.RoIsLink          = "roi"
    mvaViewsMaker.RoITool           = ViewCreatorPreviousROITool()
    mvaViewsMaker.InViewRoIs        = "RoiForTauIso"
    mvaViewsMaker.Views             = "TAUMVAViews"
    mvaViewsMaker.ViewFallThrough   = True
    mvaViewsMaker.RequireParentView = True
    mvaViewsMaker.ViewNodeName      = RecoSequenceName

    (tauMVAInViewSequence, sequenceOut) = tauIdSequence(flags, mvaViewsMaker.InViewRoIs, RecoSequenceName)

    tauSequence = seqAND("tauSequence", [mvaViewsMaker, tauMVAInViewSequence ])
    return (tauSequence, mvaViewsMaker, sequenceOut)

# ===============================================================================================                                                            
#    Reco sequence for Tau Precision LLP Alg (tracktwoLLP)                                                                                 
# ===============================================================================================                                                            

def tauLLPSequence(flags):

    RecoSequenceName = "tauLLPInViewSequence"

    mvaViewsMaker                   = EventViewCreatorAlgorithm("IMTauLLP")
    mvaViewsMaker.RoIsLink          = "roi"
    mvaViewsMaker.RoITool           = ViewCreatorPreviousROITool()
    mvaViewsMaker.InViewRoIs        = "RoiForTauIso"
    mvaViewsMaker.Views             = "TAULLPViews"
    mvaViewsMaker.ViewFallThrough   = True
    mvaViewsMaker.RequireParentView = True
    mvaViewsMaker.ViewNodeName      = RecoSequenceName

    (tauLLPInViewSequence, sequenceOut) = tauIdSequence(flags, mvaViewsMaker.InViewRoIs, RecoSequenceName)

    tauSequence = seqAND("tauLLPSequence", [mvaViewsMaker, tauLLPInViewSequence ])
    return (tauSequence, mvaViewsMaker, sequenceOut)

# ===============================================================================================
#    Reco sequence for Tau Precision LRT Alg
# ===============================================================================================

def tauLRTSequence(flags):

    RecoSequenceName = "tauLRTInViewSequence"

    mvaViewsMaker                   = EventViewCreatorAlgorithm("IMTauLRT")
    mvaViewsMaker.RoIsLink          = "roi"
    mvaViewsMaker.RoITool           = ViewCreatorPreviousROITool()
    mvaViewsMaker.InViewRoIs        = "RoiForTauLRT"
    mvaViewsMaker.Views             = "TAULRTViews"
    mvaViewsMaker.ViewFallThrough   = True
    mvaViewsMaker.RequireParentView = True
    mvaViewsMaker.ViewNodeName      = RecoSequenceName

    (tauLRTInViewSequence, sequenceOut) = tauIdSequence(flags, mvaViewsMaker.InViewRoIs, RecoSequenceName)

    tauLRTSequence = seqAND("tauLRTSequence", [mvaViewsMaker, tauLRTInViewSequence ])
    return (tauLRTSequence, mvaViewsMaker, sequenceOut)
