#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from ..Config.MenuComponents import MenuSequence, MenuSequenceCA, RecoFragmentsPool, algorithmCAToGlobalWrapper, SelectionCA, InViewRecoCA, InEventRecoCA

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.CFElements import parOR, seqAND, seqOR
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
log = logging.getLogger(__name__)

from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorNamedROITool, \
  ViewCreatorCentredOnIParticleROITool, ViewCreatorFetchFromViewROITool

#muon container names (for RoI based sequences)
from .MuonRecoSequences import muonNames
muNames = muonNames().getNames('RoI')
muNamesFS = muonNames().getNames('FS')
from TrigEDMConfig.TriggerEDMRun3 import recordable

#-----------------------------------------------------#
### ************* Step1  ************* ###
#-----------------------------------------------------#
def muFastAlgSequenceCfg(flags, is_probe_leg=False):

    selAccSA = SelectionCA('L2MuFastSel', isProbe=is_probe_leg)
    
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
    muFastRecoSeq = muFastRecoSequenceCfg( flags, viewName+'RoIs', doFullScanID= isCosmic(flags), extraLoads=extraLoads )
    sequenceOut = muNames.L2SAName
    acc.merge(muFastRecoSeq, sequenceName=seql2sa.name)

    ##### L2 mutli-track mode #####
    seqFilter = seqAND("L2MuonMTSeq")
    acc.addSequence(seqFilter)
    from TrigMuonEF.TrigMuonEFConfig import MuonChainFilterAlgCfg
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter = MuonChainFilterAlgCfg(flags, "SAFilterMultiTrackChains", ChainsToFilter = MultiTrackChains, 
                                                  InputDecisions = filterInput,
                                                  L2MuFastContainer = muNames.L2SAName+"l2mtmode", 
                                                  L2MuCombContainer = muNames.L2CBName+"l2mtmode",
                                                  WriteMuFast = True, NotGate = True)

    acc.merge(MultiTrackChainFilter, sequenceName=seqFilter.name)
    muFastl2mtRecoSeq = muFastRecoSequenceCfg( flags, viewName+'RoIs', doFullScanID= isCosmic(flags), l2mtmode=True )
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
    muFastRecoSeq = muFastRecoSequenceCfg( flags, viewName+'RoIs', doFullScanID= isCosmic(flags), calib=True )
    sequenceOut = muNames.L2SAName+"Calib"
    recoSA.mergeReco(muFastRecoSeq)


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccSA.mergeReco(recoSA, robPrefetchCA=robPrefetchAlg)

    return (selAccSA, sequenceOut)

def muFastSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, is_probe_leg)

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

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, is_probe_leg)

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
def muCombAlgSequence(flags):
    ### set the EVCreator ###
    l2muCombViewsMaker = EventViewCreatorAlgorithm("IMl2muComb")
    newRoITool = ViewCreatorFetchFromViewROITool()
    newRoITool.RoisWriteHandleKey = recordable("HLT_Roi_L2SAMuon") #RoI collection recorded to EDM
    newRoITool.InViewRoIs = muNames.L2forIDName #input RoIs from L2 SA views

    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muFastRecoSequenceCfg, muCombRecoSequence, muonIDFastTrackingSequence, muonIDCosmicTrackingSequence, isCosmic
    #
    l2muCombViewsMaker.RoIsLink = "initialRoI" # ROI for merging is still from L1, we get exactly one L2 SA muon per L1 ROI
    l2muCombViewsMaker.RoITool = newRoITool # Create a new ROI centred on the L2 SA muon from Step 1
    #
    l2muCombViewsMaker.Views = "MUCombViewRoIs" if not isCosmic(flags) else "MUCombViewCosmic" #output of the views maker (key in "storegate")
    l2muCombViewsMaker.InViewRoIs = "MUIDRoIs" if not isCosmic(flags) else "InputRoI" # Name of the RoI collection inside of the view, holds the single ROI used to seed the View. Synchronized with cosmic tracking setup in: InDetCosmicTracking.py
    #
    l2muCombViewsMaker.RequireParentView = True
    l2muCombViewsMaker.ViewFallThrough = True #if this needs to access anything from the previous step, from within the view

    muCombRecoSeq, sequenceOut = muCombRecoSequence( flags, l2muCombViewsMaker.InViewRoIs, "FTF", l2mtmode=False )

    # for L2 multi-track SA
    from TrigMuonEF.TrigMuonEFConf import MuonChainFilterAlg
    from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
    MultiTrackChainFilter = MuonChainFilterAlg("CBFilterMultiTrackChains")
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter.ChainsToFilter = MultiTrackChains
    MultiTrackChainFilter.InputDecisions = [ CFNaming.inputMakerOutName(l2muCombViewsMaker.name()) ]
    MultiTrackChainFilter.L2MuFastContainer = muNames.L2SAName+"l2mtmode"
    MultiTrackChainFilter.L2MuCombContainer = muNames.L2CBName+"l2mtmode"
    MultiTrackChainFilter.WriteMuComb = True
    MultiTrackChainFilter.NotGate = True

    extraLoadsForl2mtmode = []

    for decision in MultiTrackChainFilter.InputDecisions:
      extraLoadsForl2mtmode += [( 'xAOD::TrigCompositeContainer', 'StoreGateSvc+%s' % decision )]

    muCombl2mtRecoSeq, sequenceOutL2mtCB = muCombRecoSequence( flags, l2muCombViewsMaker.InViewRoIs, "FTF", l2mtmode=True )
    muCombl2mtFilterSequence = seqAND("l2mtmuCombFilterSequence", [MultiTrackChainFilter, muCombl2mtRecoSeq])

    #Filter algorithm to run muComb only if non-Bphysics muon chains are active
    muonChainFilter = MuonChainFilterAlg("FilterBphysChains")
    muonChainFilter.ChainsToFilter = getBphysChainNames()
    muonChainFilter.InputDecisions = [ CFNaming.inputMakerOutName(l2muCombViewsMaker.name()) ]
    muonChainFilter.L2MuCombContainer = sequenceOut
    muonChainFilter.WriteMuFast = False
    muonChainFilter.WriteMuComb = True

    # for nominal muComb
    muCombFilterSequence = seqAND("l2muCombFilterSequence", [muonChainFilter, muCombRecoSeq])

    extraLoads = []

    for decision in muonChainFilter.InputDecisions:
      extraLoads += [( 'xAOD::TrigCompositeContainer' , 'StoreGateSvc+%s' % decision )]

    if isCosmic(flags):
        muTrigIDRecoSequence = muonIDCosmicTrackingSequence( flags, l2muCombViewsMaker.InViewRoIs , "", extraLoads )
    else:
        muTrigIDRecoSequence = muonIDFastTrackingSequence( flags, l2muCombViewsMaker.InViewRoIs , "", extraLoads, extraLoadsForl2mtmode )


    # for Inside-out L2SA
    from .MuonRecoSequences  import isCosmic
    muFastIORecoSequence = algorithmCAToGlobalWrapper(muFastRecoSequenceCfg,flags, l2muCombViewsMaker.InViewRoIs, doFullScanID=isCosmic(flags) , InsideOutMode=True)
    sequenceOutL2SAIO = muNames.L2SAName+"IOmode"
    insideoutMuonChainFilter = MuonChainFilterAlg("FilterInsideOutMuonChains")
    insideoutMuonChains = getInsideOutMuonChainNames()
    insideoutMuonChainFilter.ChainsToFilter = insideoutMuonChains
    insideoutMuonChainFilter.InputDecisions = [ CFNaming.inputMakerOutName(l2muCombViewsMaker.name()) ]
    insideoutMuonChainFilter.L2MuFastContainer = sequenceOutL2SAIO
    insideoutMuonChainFilter.L2MuCombContainer = muNames.L2CBName+"IOmode"
    insideoutMuonChainFilter.WriteMuFast = True
    insideoutMuonChainFilter.WriteMuComb = True
    insideoutMuonChainFilter.NotGate = True

    muFastIOFilterSequence = seqAND("l2muFastIOFilterSequence", [insideoutMuonChainFilter, muFastIORecoSequence])

    muCombIDSequence = parOR("l2muCombIDSequence", [muTrigIDRecoSequence, muCombFilterSequence, muFastIOFilterSequence, muCombl2mtFilterSequence])

    l2muCombViewsMaker.ViewNodeName = muCombIDSequence.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=l2muCombViewsMaker.name())[0]

    l2muCombSequence = seqAND("l2muCombSequence", [l2muCombViewsMaker, robPrefetchAlg, muCombIDSequence] )

    return (l2muCombSequence, l2muCombViewsMaker, sequenceOut)



def muCombSequence(flags, is_probe_leg=False):

    (l2muCombSequence, l2muCombViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muCombAlgSequence, flags)

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlg
    #trigmuCombHypo = TrigmuCombHypoAlg("L2muCombHypoAlg") # avoid to have "Comb" string in the name due to HLTCFConfig.py.
    trigmuCombHypo = TrigmuCombHypoAlg("TrigL2MuCBHypoAlg")
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = l2muCombSequence,
                         Maker       = l2muCombViewsMaker,
                         Hypo        = trigmuCombHypo,
                         HypoToolGen = TrigmuCombHypoToolFromDict,
                         IsProbe     = is_probe_leg )


def muCombLRTAlgSequence(flags):
    ### set the EVCreator ###
    l2muCombLRTViewsMaker = EventViewCreatorAlgorithm("IMl2muCombLRT")
    newRoITool = ViewCreatorCentredOnIParticleROITool("l2muCombLRTROITool")

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig("muonLRT")
    newRoITool.RoIEtaWidth=IDConfig.etaHalfWidth
    newRoITool.RoIPhiWidth=IDConfig.phiHalfWidth
    if IDConfig.zedHalfWidth > 0 :
        newRoITool.RoIZedWidth=IDConfig.zedHalfWidth
        # for the LRT instance we want a *wider* roi z width, and *don't* want to 
        # update the z position for the central z
        newRoITool.UseZedPosition=False
    newRoITool.RoisWriteHandleKey = recordable("HLT_Roi_L2SAMuon_LRT") #RoI collection recorded to EDM

    #
    l2muCombLRTViewsMaker.RoIsLink = "initialRoI" # ROI for merging is still from L1, we get exactly one L2 SA muon per L1 ROI
    l2muCombLRTViewsMaker.RoITool = newRoITool # Create a new ROI centred on the L2 SA muon from Step 1
    #
    l2muCombLRTViewsMaker.Views = "MUCombLRTViewRoIs" #output of the views maker (key in "storegate")
    l2muCombLRTViewsMaker.InViewRoIs = "MUIDRoIs" # Name of the RoI collection inside of the view, holds the single ROI used to seed the View.
    #
    l2muCombLRTViewsMaker.RequireParentView = True
    l2muCombLRTViewsMaker.ViewFallThrough = True #if this needs to access anything from the previous step, from within the view

    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muCombRecoSequence, muonIDFastTrackingSequence

    muCombLRTRecoSequence, sequenceOut = muCombRecoSequence( flags, l2muCombLRTViewsMaker.InViewRoIs, "FTF_LRT" )

    extraLoads = []

    muFastIDRecoSequence = muonIDFastTrackingSequence( flags, l2muCombLRTViewsMaker.InViewRoIs , "LRT", extraLoads, doLRT=True )

    muCombLRTIDSequence = parOR("l2muCombLRTIDSequence", [muFastIDRecoSequence, muCombLRTRecoSequence])

    l2muCombLRTViewsMaker.ViewNodeName = muCombLRTIDSequence.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=l2muCombLRTViewsMaker.name())[0]

    l2muCombLRTSequence = seqAND("l2muCombLRTSequence", [l2muCombLRTViewsMaker, robPrefetchAlg, muCombLRTIDSequence] )

    return (l2muCombLRTSequence, l2muCombLRTViewsMaker, sequenceOut)



def muCombLRTSequence(flags, is_probe_leg=False):

    (l2muCombLRTSequence, l2muCombLRTViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muCombLRTAlgSequence, flags)

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlg
    trigmuCombHypo = TrigmuCombHypoAlg("TrigL2MuCBHypoAlg_LRT")
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = sequenceOut
    trigmuCombHypo.RoILinkName = "l2lrtroi"

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = l2muCombLRTSequence,
                         Maker       = l2muCombLRTViewsMaker,
                         Hypo        = trigmuCombHypo,
                         HypoToolGen = TrigmuCombHypoToolFromDict,
                         IsProbe     = is_probe_leg )


def muCombOvlpRmSequence(flags, is_probe_leg=False):

    (l2muCombSequence, l2muCombViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muCombAlgSequence, flags)

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlg
    trigmuCombHypo = TrigmuCombHypoAlg("TrigL2MuCBHypoAlg")
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoToolwORFromDict

    return MenuSequence( flags,
                         Sequence    = l2muCombSequence,
                         Maker       = l2muCombViewsMaker,
                         Hypo        = trigmuCombHypo,
                         HypoToolGen = TrigmuCombHypoToolwORFromDict,
                         IsProbe     = is_probe_leg )



def mul2IOOvlpRmSequence(flags, is_probe_leg=False):

    (l2muCombSequence, l2muCombViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muCombAlgSequence, flags)

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlg
    trigmuCombHypo = TrigmuCombHypoAlg("TrigL2MuCBIOHypoAlg")
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = muNames.L2CBName+"IOmode"

    # from TrigMuonHypo.TrigMuonHypoConfig import TrigL2MuonOverlapRemoverMucombToolFromDict
    from TrigMuonHypo.TrigMuonHypoConfig import Trigl2IOHypoToolwORFromDict

    return MenuSequence( flags,
                         Sequence    = l2muCombSequence,
                         Maker       = l2muCombViewsMaker,
                         Hypo        = trigmuCombHypo,
                         HypoToolGen = Trigl2IOHypoToolwORFromDict,
                         IsProbe     = is_probe_leg )


def mul2mtCBOvlpRmSequence(flags, is_probe_leg=False):

    (l2muCombSequence, l2muCombViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(muCombAlgSequence, flags)

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlg
    trigmuCombHypo = TrigmuCombHypoAlg("TrigL2mtMuCBHypoAlg")
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = muNames.L2CBName+"l2mtmode"

    from TrigMuonHypo.TrigMuonHypoConfig import Trigl2mtCBHypoToolwORFromDict

    return MenuSequence( flags,
                         Sequence    = l2muCombSequence,
                         Maker       = l2muCombViewsMaker,
                         Hypo        = trigmuCombHypo,
                         HypoToolGen = Trigl2mtCBHypoToolwORFromDict,
                         IsProbe     = is_probe_leg )



######################
###  EFSA step ###
######################
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
    from .MuonRecoSequences import muEFCBRecoSequence, muEFInsideOutRecoSequence

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
    muEFCBRecoSequence, sequenceOutCB = muEFCBRecoSequence(muonflagsCB, efcbViewsMaker.InViewRoIs, "RoI" )

    #Algorithm to filter events with no muons
    muonFilter = MuonFilterAlg("FilterZeroMuons")
    muonFilter.MuonContainerLocation = sequenceOutCB

    #inside-out reco sequence - runs only if filter is passed

    muonEFInsideOutRecoSequence, sequenceOutInsideOut = muEFInsideOutRecoSequence(muonflagsCB, efcbViewsMaker.InViewRoIs, "RoI")
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

    from .MuonRecoSequences import muEFCBRecoSequence

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
    muEFCBRecoSequence, sequenceOut = muEFCBRecoSequence(muonflagsCB, efcbViewsMaker.InViewRoIs, "LRT" )

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
    from .MuonRecoSequences import muEFCBRecoSequence, muEFInsideOutRecoSequence 
    #outside-in reco sequence
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muEFCBFSRecoSequence, sequenceOutCB = muEFCBRecoSequence(muonflagsCB, efcbfsInputMaker.InViewRoIs, "FS" )
    
    #Alg fitltering for no muon events
    muonFilter =  MuonFilterAlg("FilterZeroMuonsEFCBFS")
    muonFilter.MuonContainerLocation = sequenceOutCB

    #If filter passed
    muonEFInsideOutRecoSequence, sequenceOutInsideOut = muEFInsideOutRecoSequence(muonflagsCB, efcbfsInputMaker.InViewRoIs, "FS" )
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


def efLateMuAlgSequence(flags):

    from .MuonRecoSequences import muEFInsideOutRecoSequence, muonDecodeCfg, muonIDFastTrackingSequence
    eflateViewsMaker = EventViewCreatorAlgorithm("IMeflatemu")
    roiTool = ViewCreatorNamedROITool() # Use an existing ROI which is linked to the navigation with a custom name.
    roiTool.ROILinkName = "feature" # The ROI is actually linked as Step 1's feature. So the custom name is "feature".
    #
    eflateViewsMaker.mergeUsingFeature = True # Expect to have efLateMuRoIAlgSequence produce one Decision Object per lateROI, keep these distinct in the merging
    eflateViewsMaker.RoITool = roiTool
    #
    eflateViewsMaker.Views = "MUEFLATEViewRoIs"
    eflateViewsMaker.InViewRoIs = "MUEFLATERoIs"
    #
    eflateViewsMaker.ViewFallThrough = True

    #Clone and replace offline flags so we can set muon trigger specific values
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    #decode data in these RoIs
    viewAlgs_MuonPRD = algorithmCAToGlobalWrapper(muonDecodeCfg,muonflags,RoIs=eflateViewsMaker.InViewRoIs.path())
    #ID fast tracking
    muFastIDRecoSequence = muonIDFastTrackingSequence( flags, eflateViewsMaker.InViewRoIs,"Late" )
    #inside-out reco sequence
    #Clone and replace offline flags so we can set muon trigger specific values
    #muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    muonEFInsideOutRecoSequence, sequenceOut = muEFInsideOutRecoSequence(muonflagsCB, eflateViewsMaker.InViewRoIs, "LateMu")

    lateMuRecoSequence = parOR("lateMuonRecoSequence", [viewAlgs_MuonPRD, muFastIDRecoSequence, muonEFInsideOutRecoSequence])

    #Final sequence running in view
    eflateViewsMaker.ViewNodeName = lateMuRecoSequence.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Muon, flags, nameSuffix=eflateViewsMaker.name())[0]

    muonSequence = seqAND("lateMuonOutSequence", [eflateViewsMaker, robPrefetchAlg, lateMuRecoSequence])

    return (muonSequence, eflateViewsMaker, sequenceOut)

def efLateMuSequence(flags):

    (muonEFLateSequence, eflateViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(efLateMuAlgSequence, flags)

    # setup EFCB hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlg
    trigMuonEFLateHypo = TrigMuonEFHypoAlg( "TrigMuonEFCombinerLateMuHypoAlg" )
    trigMuonEFLateHypo.MuonDecisions = sequenceOut

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = muonEFLateSequence,
                         Maker       = eflateViewsMaker,
                         Hypo        = trigMuonEFLateHypo,
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
