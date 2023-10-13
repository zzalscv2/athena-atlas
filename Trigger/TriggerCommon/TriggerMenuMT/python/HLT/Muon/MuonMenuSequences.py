#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from ..Config.MenuComponents import MenuSequence, MenuSequenceCA, RecoFragmentsPool, algorithmCAToGlobalWrapper, SelectionCA, InViewRecoCA, InEventRecoCA

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.CFElements import parOR, seqAND, seqOR
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
log = logging.getLogger(__name__)

from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorNamedROITool, \
  ViewCreatorCentredOnIParticleROITool

#muon container names (for RoI based sequences)
from .MuonRecoSequences import muonNames
muNames = muonNames().getNames('RoI')
muNamesLRT = muonNames().getNames('LRT')
muNamesFS = muonNames().getNames('FS')
from TrigEDMConfig.TriggerEDMRun3 import recordable

#-----------------------------------------------------#
### ************* Step1  ************* ###
#-----------------------------------------------------#
@AccumulatorCache
def muFastAlgSequenceCfg(flags, selCAName="", is_probe_leg=False):

    selAccSA = SelectionCA('L2MuFastSel'+selCAName, isProbe=is_probe_leg)
    
    viewName="L2MuFastReco"

    recoSA = InViewRecoCA(name=viewName, isProbe=is_probe_leg)

    ### get muFast reco sequence ###   
    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muFastRecoSequenceCfg, muonDecodeCfg
    recoSA.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+'RoIs'))

    extraLoads = []
    from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
    filterInput = [ CFNaming.inputMakerOutName('IM_'+viewName) ]
    for decision in filterInput:
      extraLoads += [( 'xAOD::TrigCompositeContainer', 'StoreGateSvc+%s' % decision )]

    from .MuonRecoSequences  import  isCosmic
    acc = ComponentAccumulator()
    seql2sa = seqAND("L2MuonSASeq")
    acc.addSequence(seql2sa)
    muFastRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), extraLoads=extraLoads )
    sequenceOut = muNames.L2SAName
    acc.merge(muFastRecoSeq, sequenceName=seql2sa.name)

    ##### L2 mutli-track mode #####
    seqFilter = seqAND("L2MuonMTSeq")
    acc.addSequence(seqFilter)
    from TrigMuonEF.TrigMuonEFConfig import MuonChainFilterAlgCfg
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter = MuonChainFilterAlgCfg(muonflags, "SAFilterMultiTrackChains", ChainsToFilter = MultiTrackChains, 
                                                  InputDecisions = filterInput,
                                                  L2MuFastContainer = muNames.L2SAName+"l2mtmode", 
                                                  L2MuCombContainer = muNames.L2CBName+"l2mtmode",
                                                  WriteMuFast = True, NotGate = True)

    acc.merge(MultiTrackChainFilter, sequenceName=seqFilter.name)
    muFastl2mtRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), l2mtmode=True )
    acc.merge(muFastl2mtRecoSeq, sequenceName=seqFilter.name)
    recoSA.mergeReco(acc)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetch = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)

    selAccSA.mergeReco(recoSA, robPrefetchCA=robPrefetch)


    return (selAccSA, sequenceOut)

def muFastCalibAlgSequenceCfg(flags, is_probe_leg=False):

    selAccSA = SelectionCA('L2MuFastCalibSel', isProbe=is_probe_leg)
    
    viewName="L2MuFastCalibReco"

    recoSA = InViewRecoCA(name=viewName, isProbe=is_probe_leg)

    ### get muFast reco sequence ###
    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muFastRecoSequenceCfg, muonDecodeCfg
    recoSA.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+'RoIs'))

    from .MuonRecoSequences  import  isCosmic
    muFastRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), calib=True )
    sequenceOut = muNames.L2SAName+"Calib"
    recoSA.mergeReco(muFastRecoSeq)


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccSA.mergeReco(recoSA, robPrefetchCA=robPrefetchAlg)

    return (selAccSA, sequenceOut)

def muFastSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2MufastHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = sequenceOut)

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence


def muFastCalibSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastCalibAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2MufastCalibHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = sequenceOut)

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence



def mul2mtSAOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, "mt", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2mtMufastHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = muNames.L2SAName+"l2mtmode")

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence



#-----------------------------------------------------#
### ************* Step2  ************* ###
#-----------------------------------------------------#
@AccumulatorCache
def muCombAlgSequenceCfg(flags, selCAName="", is_probe_leg=False):
    ### set the EVCreator ###
    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muFastRecoSequenceCfg, muCombRecoSequenceCfg, muonIDFastTrackingSequenceCfg, muonIDCosmicTrackingSequenceCfg, isCosmic

    selAccCB = SelectionCA('L2MuCombSel'+selCAName, isProbe=is_probe_leg)
    
    viewName="Cosmic" if isCosmic(flags) else "L2MuCombReco"

    ViewCreatorFetchFromViewROITool=CompFactory.ViewCreatorFetchFromViewROITool
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = ViewCreatorFetchFromViewROITool(RoisWriteHandleKey="HLT_Roi_L2SAMuon", InViewRoIs = muNames.L2forIDName)
    requireParentView = True

    recoCB = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')

    
    sequenceOut = muNames.L2CBName

    acc = ComponentAccumulator()
    from TrigMuonEF.TrigMuonEFConfig import MuonChainFilterAlgCfg


    from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
    filterInput = [ CFNaming.inputMakerOutName('IM_'+viewName) ]
    extraLoads = []
    for decision in filterInput:
      extraLoads += [( 'xAOD::TrigCompositeContainer' , 'StoreGateSvc+%s' % decision )]

    if isCosmic(flags):
        recoCB.mergeReco(muonIDCosmicTrackingSequenceCfg( flags, viewName+"RoIs" , "cosmics", extraLoads, extraLoads ))
    else:
        recoCB.mergeReco(muonIDFastTrackingSequenceCfg(flags, viewName+"RoIs", "muon", extraLoads, extraLoads ))

    # for nominal muComb
    seql2cb = seqAND("l2muCombFilterSequence")
    acc.addSequence(seql2cb)

    #Filter algorithm to run muComb only if non-Bphysics muon chains are active
    muonChainFilter = MuonChainFilterAlgCfg(muonflags, "FilterBphysChains", ChainsToFilter = getBphysChainNames(), 
                                            InputDecisions = filterInput,
                                            L2MuCombContainer = sequenceOut,
                                            WriteMuComb = True, WriteMuFast=False)
    acc.merge(muonChainFilter, sequenceName=seql2cb.name)



    acc.merge(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF", l2mtmode=False, l2CBname = sequenceOut ), sequenceName=seql2cb.name)

    # for L2 multi-track SA
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter = MuonChainFilterAlgCfg(muonflags, "CBFilterMultiTrackChains", ChainsToFilter = MultiTrackChains, 
                                                  InputDecisions = filterInput,
                                                  L2MuFastContainer = muNames.L2SAName+"l2mtmode", 
                                                  L2MuCombContainer = muNames.L2CBName+"l2mtmode",
                                                  WriteMuComb = True, NotGate = True)


    seql2cbmt = seqAND("l2mtmuCombFilterSequence")
    acc.addSequence(seql2cbmt)
    acc.merge(MultiTrackChainFilter, sequenceName=seql2cbmt.name)


    sequenceOutL2mtCB = muNames.L2CBName+"l2mtmode"
    acc.merge(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF", l2mtmode=True, l2CBname = sequenceOutL2mtCB ), sequenceName=seql2cbmt.name)



    # for Inside-out L2SA
    seql2iocb = seqAND("l2muFastIOFilterSequence")
    acc.addSequence(seql2iocb)

    from .MuonRecoSequences  import isCosmic
    sequenceOutL2SAIO = muNames.L2SAName+"IOmode"
    insideoutMuonChainFilter = MuonChainFilterAlgCfg("FilterInsideOutMuonChains", ChainsToFilter = getInsideOutMuonChainNames(),
                                                     InputDecisions = filterInput,
                                                     L2MuFastContainer = sequenceOutL2SAIO, L2MuCombContainer = muNames.L2CBName+"IOmode",
                                                     WriteMuFast = True, WriteMuComb = True, NotGate=True)

    acc.merge(insideoutMuonChainFilter, sequenceName=seql2iocb.name)
    acc.merge(muFastRecoSequenceCfg(muonflags, viewName+"RoIs", doFullScanID=isCosmic(flags) , InsideOutMode=True), sequenceName=seql2iocb.name)
    recoCB.mergeReco(acc)


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccCB.mergeReco(recoCB, robPrefetchCA=robPrefetchAlg)

    return (selAccCB, sequenceOut)




def muCombSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut)

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolFromDict, isProbe=is_probe_leg)


    return l2cbSequence

def mul2IOOvlpRmSequence(flags, is_probe_leg=False):


    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "IO", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, Trigl2IOHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBIOHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut+"IOmode")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = Trigl2IOHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence

def muCombLRTAlgSequenceCfg(flags, is_probe_leg=False):

    selAcc = SelectionCA('l2muCombLRT', isProbe=is_probe_leg)
    
    viewName="l2muCombLRT"
    ViewCreatorCenteredOnIParticleTool=CompFactory.ViewCreatorCentredOnIParticleROITool
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig("muonLRT")
  
    roiTool         = ViewCreatorCenteredOnIParticleTool(RoisWriteHandleKey = recordable("HLT_Roi_L2SAMuon_LRT"), RoIZedWidth=IDConfig.zedHalfWidth, RoIEtaWidth=IDConfig.etaHalfWidth, RoIPhiWidth=IDConfig.phiHalfWidth, UseZedPosition=False)
    requireParentView = True

    recol2cb = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)

    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muCombRecoSequenceCfg, muonIDFastTrackingSequenceCfg
    sequenceOut = muNamesLRT.L2CBName
    recol2cb.mergeReco(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF_LRT", l2CBname = sequenceOut ))

    extraLoads = []

    recol2cb.mergeReco(muonIDFastTrackingSequenceCfg(flags, viewName+"RoIs" , "muonLRT", extraLoads, doLRT=True ))


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAcc.mergeReco(recol2cb, robPrefetchAlg)


    return (selAcc, sequenceOut)



def muCombLRTSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombLRTAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBLRTHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut,
                                     RoILinkName = "l2lrtroi")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolFromDict, isProbe=is_probe_leg)


    return l2cbSequence


def muCombOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut)

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence
 




def mul2mtCBOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "mt", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, Trigl2mtCBHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2mtMuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut+"l2mtmode")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = Trigl2mtCBHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence


######################
###  EFSA step ###
######################
@AccumulatorCache
def muEFSAAlgSequenceCfg(flags, is_probe_leg=False):

    selAccMS = SelectionCA('EFMuMSSel_RoI', isProbe=is_probe_leg)
    
    viewName="EFMuMSReco_RoI"
    ViewCreatorFetchFromViewROITool=CompFactory.ViewCreatorFetchFromViewROITool
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = ViewCreatorFetchFromViewROITool(RoisWriteHandleKey="HLT_Roi_L2SAMuonForEF", InViewRoIs = "forMS", ViewToFetchFrom = "L2MuFastRecoViews")
    requireParentView = True

    recoMS = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muEFSARecoSequenceCfg, muonDecodeCfg
    #Run decoding again since we are using updated RoIs
    recoMS.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+"RoIs"))
    ### get EF reco sequence ###    
    muEFSARecoSequenceAcc, sequenceOut = muEFSARecoSequenceCfg(muonflags, viewName+'RoIs', 'RoI' )
    recoMS.mergeReco(muEFSARecoSequenceAcc)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    prefetch=ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccMS.mergeReco(recoMS, robPrefetchCA=prefetch)

    return (selAccMS, sequenceOut)


def muEFSASequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFSAAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFMSonlyHypoToolFromDict
    efmuMSHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFMSonlyHypo_RoI',
                              MuonDecisions = sequenceOut,
                              IncludeSAmuons=True)

    selAcc.addHypoAlgo(efmuMSHypo)
    
    efmuMSSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFMSonlyHypoToolFromDict, isProbe=is_probe_leg)


    return efmuMSSequence


######################
###  EFCB seq ###
######################
def muEFCBAlgSequence(flags):

    #By default the EFCB sequence will run both outside-in and
    #(if zero muons are found) inside-out reconstruction
    from TrigMuonEF.TrigMuonEFConf import MuonFilterAlg, MergeEFMuonsAlg
    from .MuonRecoSequences import muEFCBRecoSequenceCfg, muEFInsideOutRecoSequenceCfg

    efcbViewsMaker = EventViewCreatorAlgorithm("IMefcbtotal")
    #
    efcbViewsMaker.RoIsLink = "roi" # Merge based on L2SA muon
    efcbViewsMaker.RoITool = ViewCreatorNamedROITool(ROILinkName="l2cbroi") # Spawn EventViews based on L2 CB RoIs
    #
    from .MuonRecoSequences import isCosmic
    efcbViewsMaker.Views = "MUEFCBViewRoIs" if not isCosmic(flags) else "CosmicEFCBViewRoIs"
    efcbViewsMaker.InViewRoIs = "MUEFCBRoIs"
    #
    efcbViewsMaker.RequireParentView = True
    efcbViewsMaker.ViewFallThrough = True
    efcbViewsMaker.mergeUsingFeature = True

    #outside-in reco sequence
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muEFCBRecoSequence = algorithmCAToGlobalWrapper(muEFCBRecoSequenceCfg, muonflagsCB, efcbViewsMaker.InViewRoIs, "RoI" )
    sequenceOutCB = muNames.EFCBOutInName

    #Algorithm to filter events with no muons
    muonFilter = MuonFilterAlg("FilterZeroMuons")
    muonFilter.MuonContainerLocation = sequenceOutCB

    #inside-out reco sequence - runs only if filter is passed

    muonEFInsideOutRecoAlgSequence = algorithmCAToGlobalWrapper(muEFInsideOutRecoSequenceCfg, muonflagsCB, efcbViewsMaker.InViewRoIs, "RoI")
    muonEFInsideOutRecoSequence = parOR("efmuInsideOutViewNode_RoI", [muonEFInsideOutRecoAlgSequence])
    sequenceOutInsideOut = muNames.EFCBInOutName
    muonInsideOutSequence = seqAND("muonEFInsideOutSequence", [muonFilter,muonEFInsideOutRecoSequence])

    #combine outside-in and inside-out sequences
    muonRecoSequence = parOR("muonEFCBandInsideOutRecoSequence", [muEFCBRecoSequence, muonInsideOutSequence])

    #Merge muon containers from outside-in and inside-out reco
    muonMerger = MergeEFMuonsAlg("MergeEFMuons")
    muonMerger.MuonCBContainerLocation = sequenceOutCB
    muonMerger.MuonInsideOutContainerLocation = sequenceOutInsideOut
    muonMerger.MuonOutputLocation = muNames.EFCBName
    sequenceOut = muonMerger.MuonOutputLocation

    #Add merging alg in sequence with reco sequences
    mergeSequence = seqOR("muonCBInsideOutMergingSequence", [muonRecoSequence, muonMerger])

    #Final sequence running in view
    efcbViewsMaker.ViewNodeName = mergeSequence.name()
    muonSequence = seqAND("muonEFCBandInsideOutSequence", [efcbViewsMaker, mergeSequence])

    return (muonSequence, efcbViewsMaker, sequenceOut)

def muEFCBSequence(flags, is_probe_leg=False):

    (muonEFCBSequence, efcbViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBAlgSequence, flags)

    # setup EFCB hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFCBHypo = TrigMuonEFHypoAlg( "TrigMuonEFCombinerHypoAlg" )
    trigMuonEFCBHypo.MuonDecisions = sequenceOut
    trigMuonEFCBHypo.MapToPreviousDecisions=True

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFCBSequence,
                         Maker       = efcbViewsMaker,
                         Hypo        = trigMuonEFCBHypo,
                         HypoToolGen = TrigMuonEFCombinerHypoToolFromDict,
                         IsProbe     = is_probe_leg )

def muEFCBIDperfSequence(flags, is_probe_leg=False):

    (muonEFCBSequence, efcbViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBAlgSequence, flags)

    # setup EFCB hypo for idperf (needs to not require CB muons)
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFCBHypo = TrigMuonEFHypoAlg( "TrigMuonEFCombinerIDperfHypoAlg", IncludeSAmuons=True  )
    trigMuonEFCBHypo.MuonDecisions = sequenceOut
    trigMuonEFCBHypo.MapToPreviousDecisions=True

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFCBSequence,
                         Maker       = efcbViewsMaker,
                         Hypo        = trigMuonEFCBHypo,
                         HypoToolGen = TrigMuonEFCombinerHypoToolFromDict,
                         IsProbe     = is_probe_leg )

def muEFIDtpSequence(flags, is_probe_leg=False):

    (muonEFCBSequence, efcbViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBAlgSequence, flags)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFIdtpHypoAlg
    trigMuonEFIdtpHypo = TrigMuonEFIdtpHypoAlg( "TrigMuonEFIdtpHypoAlg" )

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFIdtpHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFCBSequence,
                         Maker       = efcbViewsMaker,
                         Hypo        = trigMuonEFIdtpHypo,
                         HypoToolGen = TrigMuonEFIdtpHypoToolFromDict,
                         IsProbe     = is_probe_leg )


def muEFCBLRTAlgSequence(flags):

    from .MuonRecoSequences import muEFCBRecoSequenceCfg

    efcbViewsMaker = EventViewCreatorAlgorithm("IMefcblrttotal")
    #
    efcbViewsMaker.RoIsLink = "roi" # Merge based on L2SA muon
    efcbViewsMaker.RoITool = ViewCreatorNamedROITool(ROILinkName="l2lrtroi") # Spawn EventViews based on L2 CB RoIs
    #
    efcbViewsMaker.Views = "MUEFCBLRTViewRoIs"
    efcbViewsMaker.InViewRoIs = "MUEFCBRoIs"
    #
    efcbViewsMaker.RequireParentView = True
    efcbViewsMaker.ViewFallThrough = True
    efcbViewsMaker.mergeUsingFeature = True

    #outside-in reco sequence
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muEFCBRecoAlgSequence = algorithmCAToGlobalWrapper(muEFCBRecoSequenceCfg, muonflagsCB, efcbViewsMaker.InViewRoIs, "LRT" )
    muEFCBRecoSequence = parOR("efcbViewNode_LRT", [muEFCBRecoAlgSequence])
    sequenceOut = muNamesLRT.EFCBName

    #Final sequence running in view
    efcbViewsMaker.ViewNodeName = muEFCBRecoSequence.name()
    muonSequence = seqAND("muonEFCBLRTSequence", [efcbViewsMaker, muEFCBRecoSequence])

    return (muonSequence, efcbViewsMaker, sequenceOut)

def muEFCBLRTSequence(flags, is_probe_leg=False):

    (muonEFCBLRTSequence, efcbViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBLRTAlgSequence, flags)

    # setup EFCBLRT hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFCBLRTHypo = TrigMuonEFHypoAlg( "TrigMuonEFCombinerHypoAlgLRT" )
    trigMuonEFCBLRTHypo.MuonDecisions = sequenceOut
    trigMuonEFCBLRTHypo.MapToPreviousDecisions=True

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFCBLRTSequence,
                         Maker       = efcbViewsMaker,
                         Hypo        = trigMuonEFCBLRTHypo,
                         HypoToolGen = TrigMuonEFCombinerHypoToolFromDict,
                         IsProbe     = is_probe_leg )


def muEFCBLRTIDperfSequence(flags, is_probe_leg=False):

    (muonEFCBLRTSequence, efcbViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBLRTAlgSequence, flags)

    # setup EFCBLRT hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFCBLRTHypo = TrigMuonEFHypoAlg( "TrigMuonEFCombinerHypoAlgLRTIDPerf", IncludeSAmuons=True   )
    trigMuonEFCBLRTHypo.MuonDecisions = sequenceOut
    trigMuonEFCBLRTHypo.MapToPreviousDecisions=True

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFCBLRTSequence,
                         Maker       = efcbViewsMaker,
                         Hypo        = trigMuonEFCBLRTHypo,
                         HypoToolGen = TrigMuonEFCombinerHypoToolFromDict,
                         IsProbe     = is_probe_leg )


######################
### EF SA full scan ###
######################
@AccumulatorCache
def muEFSAFSAlgSequenceCfg(flags):

    selAccMS = SelectionCA('EFMuMSSel_FS')
    
    viewName="EFMuMSReco_FS"
    ViewCreatorFSROITool=CompFactory.ViewCreatorFSROITool
    roiTool         = ViewCreatorFSROITool(RoisWriteHandleKey="MuonFS_RoIs")
    requireParentView = False
                                                         
    recoMS = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muEFSARecoSequenceCfg, muonDecodeCfg
    recoMS.mergeReco(muonDecodeCfg(muonflags,RoIs=recoMS.name+"RoIs"))
    ### get EF reco sequence ###    
    muEFSARecoSequenceAcc, sequenceOut = muEFSARecoSequenceCfg(muonflags, recoMS.name+'RoIs', 'FS' )
    recoMS.mergeReco(muEFSARecoSequenceAcc)

    selAccMS.mergeReco(recoMS)

    return (selAccMS, sequenceOut)


def muEFSAFSSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFSAFSAlgSequenceCfg(flags)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFMSonlyHypoToolFromName

    efmuMSHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFMSonlyHypo_FS',
                              MuonDecisions = sequenceOut,
                              IncludeSAmuons=True)

    selAcc.addHypoAlgo(efmuMSHypo)
    
    efmuMSSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFMSonlyHypoToolFromName, isProbe=is_probe_leg)


    return efmuMSSequence


######################
### EF CB full scan ###
######################
def muEFCBFSAlgSequence(flags):
    efcbfsInputMaker = EventViewCreatorAlgorithm("IMEFCBFSAlg")
    newRoITool = ViewCreatorCentredOnIParticleROITool()
    newRoITool.RoisWriteHandleKey = "MuonCandidates_FS_ROIs"
    #
    efcbfsInputMaker.mergeUsingFeature = True
    efcbfsInputMaker.RoITool = newRoITool
    #
    efcbfsInputMaker.Views = "MUCBFSViews"
    efcbfsInputMaker.InViewRoIs = "MUCBFSRoIs"
    #
    efcbfsInputMaker.RequireParentView = True
    efcbfsInputMaker.ViewFallThrough = True
    # Muon specific
    efcbfsInputMaker.PlaceMuonInView = True
    efcbfsInputMaker.InViewMuons = "InViewMuons"
    efcbfsInputMaker.InViewMuonCandidates = "MuonCandidates_FS"

    from TrigMuonEF.TrigMuonEFConf import MuonFilterAlg, MergeEFMuonsAlg
    from .MuonRecoSequences import muEFCBRecoSequenceCfg, muEFInsideOutRecoSequenceCfg
    #outside-in reco sequence
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muEFCBFSRecoSequence = algorithmCAToGlobalWrapper(muEFCBRecoSequenceCfg, muonflagsCB, efcbfsInputMaker.InViewRoIs, "FS" )
    sequenceOutCB = muNamesFS.EFCBOutInName

    #Alg fitltering for no muon events
    muonFilter =  MuonFilterAlg("FilterZeroMuonsEFCBFS")
    muonFilter.MuonContainerLocation = sequenceOutCB

    #If filter passed
    muonEFInsideOutRecoAlgSequence = algorithmCAToGlobalWrapper(muEFInsideOutRecoSequenceCfg, muonflagsCB, efcbfsInputMaker.InViewRoIs, "FS" )
    muonEFInsideOutRecoSequence = parOR("efmuInsideOutViewNode_FS", [muonEFInsideOutRecoAlgSequence])
    sequenceOutInsideOut = muNames.EFCBInOutName
    muonInsideOutSequence =  seqAND("muonEFCBFSInsideOutSequence", [muonFilter,muonEFInsideOutRecoSequence])

    #combine O-I and I-O seqs
    muonRecoSequence = parOR("muonEFCBFSInsideOutRecoSequence", [muEFCBFSRecoSequence, muonInsideOutSequence])

    #Merge muon containers from O-I and I-O reco
    muonMerger = MergeEFMuonsAlg("MergeEFCBFSMuons")
    muonMerger.MuonCBContainerLocation = sequenceOutCB
    muonMerger.MuonInsideOutContainerLocation = sequenceOutInsideOut
    muonMerger.MuonOutputLocation = muNamesFS.EFCBName
    sequenceOut = muonMerger.MuonOutputLocation

    #Add merging alg in seq with reco seq
    mergeSequence = seqOR("muonCBInsideOutMergingSequenceEFCBFS", [muonRecoSequence, muonMerger])

    efcbfsInputMaker.ViewNodeName = mergeSequence.name()    
    muonEFCBFSSequence = seqAND( "muonEFFSCBSequence", [efcbfsInputMaker, mergeSequence] )

    return (muonEFCBFSSequence, efcbfsInputMaker, sequenceOut)

def muEFCBFSSequence(flags, is_probe_leg=False):

    (muonEFCBFSSequence, efcbfsInputMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFCBFSAlgSequence, flags)

    # setup EFCB hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFCBFSHypo = TrigMuonEFHypoAlg( "TrigMuonEFFSCombinerHypoAlg" )
    trigMuonEFCBFSHypo.MuonDecisions = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromName

    return MenuSequence( flags,
                         Sequence    = muonEFCBFSSequence,
                         Maker       = efcbfsInputMaker,
                         Hypo        = trigMuonEFCBFSHypo,
                         HypoToolGen = TrigMuonEFCombinerHypoToolFromName,
                         IsProbe     = is_probe_leg )

def efLateMuRoIAlgSequenceCfg(flags):

    selAcc = SelectionCA('EFLateMuSel')
    
    viewName="EFLateMuRoIReco"
    viewcreator         = CompFactory.ViewCreatorInitialROITool
    roiTool = viewcreator()
    requireParentView = True
                                                         
    recoLateMu = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView)

    from .MuonRecoSequences import efLateMuRoISequenceCfg

    #Get Late Muon RoIs
    efLateMuRoIAcc, sequenceOut = efLateMuRoISequenceCfg(flags)
    recoLateMu.mergeReco(efLateMuRoIAcc)

    selAcc.mergeReco(recoLateMu)
    return (selAcc, sequenceOut)

def efLateMuRoISequence(flags):

    (selAcc, sequenceOut) = efLateMuRoIAlgSequenceCfg(flags)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonLateMuRoIHypoAlgCfg, TrigMuonLateMuRoIHypoToolFromDict
    latemuHypo = TrigMuonLateMuRoIHypoAlgCfg( flags,
                              name = 'TrigMuonLateMuRoIHypoAlg',
                              LateRoIs = sequenceOut)

    selAcc.addHypoAlgo(latemuHypo)
    
    latemuRoISequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonLateMuRoIHypoToolFromDict)


    return latemuRoISequence


def efLateMuAlgSequenceCfg(flags):

    from .MuonRecoSequences import muEFInsideOutRecoSequenceCfg, muonDecodeCfg, muonIDFastTrackingSequenceCfg
    selAcc = SelectionCA('EFLateMuAlg')
    
    viewName="EFLateMuReco"
    viewcreator         = CompFactory.ViewCreatorNamedROITool
    roiTool = viewcreator(ROILinkName="feature")
    requireParentView = True
                                                         
    recoLateMu = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, mergeUsingFeature=True)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    #decode data in these RoIs
    recoLateMu.mergeReco(muonDecodeCfg(muonflags,RoIs=recoLateMu.name+"RoIs"))
    #ID fast tracking
    recoLateMu.mergeReco(muonIDFastTrackingSequenceCfg(flags, recoLateMu.name+"RoIs","muonLate" ))
    #inside-out reco sequence
    recoLateMu.mergeReco(muEFInsideOutRecoSequenceCfg(muonflagsCB, recoLateMu.name+"RoIs", "LateMu"))
    sequenceOut = muNames.EFCBInOutName+'_Late'


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName)
    selAcc.mergeReco(recoLateMu, robPrefetchCA = robPrefetchAlg)

    return (selAcc, sequenceOut)

def efLateMuSequence(flags):

    (selAcc, sequenceOut) = efLateMuAlgSequenceCfg(flags)

    # setup EFCB hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg
    trigMuonEFLateHypo = TrigMuonEFHypoAlgCfg( "TrigMuonEFCombinerLateMuHypoAlg", MuonDecisions = sequenceOut )

    selAcc.addHypoAlgo(trigMuonEFLateHypo)
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequenceCA(flags, selAcc,
                          HypoToolGen = TrigMuonEFCombinerHypoToolFromDict )



######################
### efMuiso step ###
######################
def muEFIsoAlgSequence(flags, doMSiso=False):
    name = ""
    if doMSiso:
        name = "MS"
    efmuIsoViewsMaker = EventViewCreatorAlgorithm("IMefmuiso"+name)
    newRoITool = ViewCreatorCentredOnIParticleROITool()
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig("muonIso")
    newRoITool.RoIEtaWidth=IDConfig.etaHalfWidth
    newRoITool.RoIPhiWidth=IDConfig.phiHalfWidth
    newRoITool.RoIZedWidth=IDConfig.zedHalfWidth
    if doMSiso:
        newRoITool.RoisWriteHandleKey = "Roi_MuonIsoMS"
    else:
        newRoITool.RoisWriteHandleKey = recordable("HLT_Roi_MuonIso")
    #
    efmuIsoViewsMaker.mergeUsingFeature = True
    efmuIsoViewsMaker.RoITool = newRoITool
    #
    efmuIsoViewsMaker.Views = "MUEFIsoViewRoIs"+name
    efmuIsoViewsMaker.InViewRoIs = "MUEFIsoRoIs"+name
    #
    efmuIsoViewsMaker.ViewFallThrough = True
    # Muon specific
    # TODO - this should be deprecated here and removed in the future, now that we mergeUsingFeature, each parent View should only have one muon.
    # therefore the xAOD::Muon should be got via ViewFallThrough, rather than being copied in here as "IsoViewMuons"
    efmuIsoViewsMaker.PlaceMuonInView = True
    efmuIsoViewsMaker.InViewMuonCandidates = "IsoMuonCandidates"+name
    efmuIsoViewsMaker.InViewMuons = "IsoViewMuons"+name

    ### get EF reco sequence ###
    from .MuonRecoSequences  import efmuisoRecoSequence
    efmuisoRecoSequence, sequenceOut = efmuisoRecoSequence( flags, efmuIsoViewsMaker.InViewRoIs, efmuIsoViewsMaker.InViewMuons, doMSiso )

    efmuIsoViewsMaker.ViewNodeName = efmuisoRecoSequence.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=efmuIsoViewsMaker.name())[0]

    ### Define a Sequence to run for muIso ###
    efmuIsoSequence = seqAND("efmuIsoSequence"+name, [ efmuIsoViewsMaker, robPrefetchAlg, efmuisoRecoSequence ] )

    return (efmuIsoSequence, efmuIsoViewsMaker, sequenceOut)

def muEFIsoSequence(flags, is_probe_leg=False):

    (efmuIsoSequence, efmuIsoViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFIsoAlgSequence, flags)

    # set up hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoAlg
    trigmuefIsoHypo = TrigMuonEFTrackIsolationHypoAlg("EFMuisoHypoAlg")
    trigmuefIsoHypo.EFMuonsName = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = efmuIsoSequence,
                         Maker       = efmuIsoViewsMaker,
                         Hypo        = trigmuefIsoHypo,
                         HypoToolGen = TrigMuonEFTrackIsolationHypoToolFromDict,
                         IsProbe     = is_probe_leg )

def muEFMSIsoSequence(flags, is_probe_leg=False):

    (efmuIsoSequence, efmuIsoViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muEFIsoAlgSequence, flags, doMSiso=True)

    # set up hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoAlg
    trigmuefIsoHypo = TrigMuonEFTrackIsolationHypoAlg("EFMuMSisoHypoAlg")
    trigmuefIsoHypo.EFMuonsName = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = efmuIsoSequence,
                         Maker       = efmuIsoViewsMaker,
                         Hypo        = trigmuefIsoHypo,
                         HypoToolGen = TrigMuonEFTrackIsolationHypoToolFromDict,
                         IsProbe     = is_probe_leg )

####################################################
##  Muon RoI Cluster Trigger for MS LLP Searches  ##
####################################################

def muRoiClusterSequence(flags):

    from TrigLongLivedParticles.TrigLongLivedParticlesConfig import MuonClusterConfig
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConfig import MuonClusterHypoAlgConfig, TrigLongLivedParticlesHypoToolFromDict

    selAcc = SelectionCA('muRoIClusterSel')
    
    viewName="MuRoIClusReco"
    viewcreator         = CompFactory.ViewCreatorInitialROITool
    roiTool = viewcreator()
                                                         
    recoRoICluster = InEventRecoCA(name=viewName, RoITool = roiTool, mergeUsingFeature = False, RoIs = 'HLT_muVtxCluster_RoIs')
    recoRoICluster.mergeReco(MuonClusterConfig(flags, 'muvtxMuonCluster'))
    selAcc.mergeReco(recoRoICluster)


    hypoAlg = MuonClusterHypoAlgConfig( flags,
                              name = 'MuRoiClusterHypoAlg')

    selAcc.addHypoAlgo(hypoAlg)
    
    muRoIClusterSequence = MenuSequenceCA(flags, selAcc,
                                          HypoToolGen = TrigLongLivedParticlesHypoToolFromDict)

    return muRoIClusterSequence



##############################
### Get Bphysics triggers to #
### filter chains where we   #
### don't want to run muComb #
##############################

def getBphysChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Bphysics']]
    return chains

############################################################
### Get muon triggers except L2 inside-out trigger
### to filter chains where we don't want to run L2SA IO mode
############################################################

def getInsideOutMuonChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Muon'] if "l2io" in chain.name]
    chains += [chain.name for chain in menu.chainsInMenu['Bphysics'] if not any(key in chain.name for key in ['noL2Comb','l2mt'])]
    return chains

############################################################
### Get muon triggers except L2 multi-track trigger
### to filter chains where we don't want to run L2SA multi-track mode
############################################################

def getMultiTrackChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Muon'] if "l2mt" in chain.name]
    chains += [chain.name for chain in menu.chainsInMenu['Bphysics'] if "l2mt" in chain.name]
    return chains
