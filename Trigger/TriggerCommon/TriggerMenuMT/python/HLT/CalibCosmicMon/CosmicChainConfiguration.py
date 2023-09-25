# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.CFElements import parOR
from AthenaCommon.CFElements import seqAND
from TrigEDMConfig.TriggerEDMRun3 import recordable
import AthenaCommon.SystemOfUnits as Units

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

def TrackCountHypoToolGen(chainDict):
    from TrigMinBias.TrigMinBiasConf import TrackCountHypoTool
    hypo = TrackCountHypoTool(chainDict["chainName"])
    hypo.minNtrks = 1
    return hypo

def CosmicsTrkSequence(flags):
    from TrigMinBias.TrigMinBiasConf import TrackCountHypoAlg
    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
    from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool
    
    trkInputMakerAlg = EventViewCreatorAlgorithm("IMCosmicTrkEventViewCreator")
    trkInputMakerAlg.ViewFallThrough = True
    trkInputMakerAlg.RoITool = ViewCreatorInitialROITool()
    trkInputMakerAlg.InViewRoIs = "CosmicRoIs" # contract with the consumer
    trkInputMakerAlg.Views = "CosmicViewRoIs"
    trkInputMakerAlg.RequireParentView = False

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, "cosmics", log)

    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
    seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                            rois = trkInputMakerAlg.InViewRoIs, inView = "VDVCosmicsIDTracking")

    from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA
    idTrackingAlgs = extractAlgorithmsAndAppendCA(seq.sequence("Offline"))

    trackCountHypo = TrackCountHypoAlg("CosmicsTrackCountHypoAlg", 
        minPt = [100*Units.MeV],
        maxZ0 = [401*Units.mm],
        vertexZ = [803*Units.mm])
    trackCountHypo.tracksKey = recordable("HLT_IDTrack_Cosmic_IDTrig")
    trackCountHypo.trackCountKey = "HLT_CosmicsTrackCount" # irrelevant, not recorded

    #TODO move a complete configuration of the algs to TrigMinBias package
    from TrigMinBias.TrigMinBiasMonitoring import TrackCountMonitoring
    trackCountHypo.MonTool = TrackCountMonitoring(flags, trackCountHypo) # monitoring tool configures itself using config of the hypo alg

    trkRecoSeq = parOR("CosmicTrkRecoSeq", idTrackingAlgs)
    trkSequence = seqAND("CosmicTrkSequence", [trkInputMakerAlg, trkRecoSeq])
    trkInputMakerAlg.ViewNodeName = trkRecoSeq.name()
    log.debug("Prepared ID tracking sequence")
    log.debug(trkSequence)
    return MenuSequence(flags,
                        Sequence    = trkSequence,
                        Maker       = trkInputMakerAlg,
                        Hypo        = trackCountHypo,
                        HypoToolGen = TrackCountHypoToolGen)

def EmptyMSBeforeCosmicID(flags):
    from TriggerMenuMT.HLT.Config.MenuComponents import EmptyMenuSequence
    return EmptyMenuSequence("EmptyBeforeCosmicID")

#----------------------------------------------------------------
class CosmicChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):       
                         
        steps = []
        log.debug("Assembling chain for %s", self.chainName)
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration         
        # --------------------
        if 'cosmic_id' in self.chainName:
            steps += [  self.getStep(flags, 1, 'Empty', [EmptyMSBeforeCosmicID]),
                        self.getStep(flags, 2, 'CosmicTracking', [CosmicsTrkSequence]) ]

        return self.buildChain(steps)


