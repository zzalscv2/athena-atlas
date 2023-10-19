# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator
from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, InViewRecoCA, menuSequenceCAToGlobalWrapper

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO,
# so let's make them functions already now
#----------------------------------------------------------------

def trkFS_trkfast_Cfg( flags ):
    if isComponentAccumulatorCfg():
        return allTE_trkfast( flags, signature="FS" )
    else:
        return menuSequenceCAToGlobalWrapper(allTE_trkfast, flags, signature="FS" )

def allTE_trkfast_Cfg(flags):
    if isComponentAccumulatorCfg():
        return allTE_trkfast( flags, signature="beamSpot" )
    else:
        return menuSequenceCAToGlobalWrapper(allTE_trkfast, flags, signature="beamSpot" )


def allTE_trkfast( flags, signature="FS" ):



        _signature=signature
        if(signature == "FS"):
            _signature = "beamSpotFS"

        beamspotSequence = InViewRecoCA('beamspotSequence_'+signature)

        flagsWithTrk = getFlagsForActiveConfig(flags, _signature, log)
        beamspotSequence.mergeReco(trigInDetFastTrackingCfg(flagsWithTrk, 
                                                            roisKey=beamspotSequence.inputMaker().InViewRoIs,
                                                            signatureName=_signature))

        from TrigT2BeamSpot.T2VertexBeamSpotConfig import T2VertexBeamSpot_activeAllTE
        vertexAlg = T2VertexBeamSpot_activeAllTE(flags, "vertex_"+_signature )
        vertexAlg.TrackCollection = flagsWithTrk.Tracking.ActiveConfig.trkTracks_FTF


        beamspotSequence.addRecoAlgo(vertexAlg)
        beamspotViewsSequence = SelectionCA('beamspotViewsSequence'+_signature)
        beamspotViewsSequence.mergeReco(beamspotSequence)


        #hypo
        beamspotHypoAlg = CompFactory.TrigStreamerHypoAlg("BeamspotHypoAlg_"+_signature)
        beamspotHypoAlg.RuntimeValidation = False #Needed to avoid the ERROR ! Decision has no 'feature' ElementLink

        beamspotViewsSequence.addHypoAlgo(beamspotHypoAlg)

        # Accept every event
        beamspotHypoToolGen = StreamerHypoToolGenerator

        return  MenuSequenceCA( flags,
                                beamspotViewsSequence,
                                HypoToolGen = beamspotHypoToolGen )


def getBeamspotVtx(flags):
        signature = "BeamspotJet"

        from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
        IDTrigConfig = getInDetTrigConfig("fullScan")

        # run at event level
        inputMakerAlg         = CompFactory.InputMakerForRoI("IM_beamspotJet_"+signature)
        inputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

        #-- Configuring Beamspot vertex alg
        from TrigT2BeamSpot.T2VertexBeamSpotConfig import T2VertexBeamSpot_activeAllTE
        vertexAlg = T2VertexBeamSpot_activeAllTE(flags, "vertex_"+signature )
        vertexAlg.TrackCollection = IDTrigConfig.trkTracks_FTF()

        #-- Setting up beamspotSequence
        beamspotSequence = InEventRecoCA('beamspotJetSequence_'+signature,inputMaker=inputMakerAlg)
        beamspotSequence.addRecoAlgo(vertexAlg)
        beamspotViewsSequence = SelectionCA('beamspotJetViewsSequence'+signature)
        beamspotViewsSequence.mergeReco(beamspotSequence)

        #-- HypoAlg and Tool
        beamspotHypoAlg = CompFactory.TrigStreamerHypoAlg("BeamspotHypoAlg_"+signature)

        beamspotViewsSequence.addHypoAlgo(beamspotHypoAlg)

        # Reject every event
        def getRejectingHypoTool(chainDict): 
                return CompFactory.TrigStreamerHypoTool(chainDict['chainName'],Pass=False)

        return  MenuSequenceCA( flags,
                                beamspotViewsSequence,
                                HypoToolGen = getRejectingHypoTool )


def getBeamspotVtxCfg( flags ):
        if isComponentAccumulatorCfg():
            return getBeamspotVtx(flags)
        else:
            return menuSequenceCAToGlobalWrapper(getBeamspotVtx, flags)


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class BeamspotChainConfiguration(ChainConfigurationBase):

        def __init__(self, chainDict, jc_name = None):
                ChainConfigurationBase.__init__(self,chainDict)
                self.jc_name=jc_name


        def assembleChainImpl(self, flags):
                chainSteps = []
                log.debug("Assembling chain for %s", self.chainName)
                stepDictionary = self.getStepDictionary()
                key = ''

                if self.chainPart['beamspotChain'] != '':
                        stepName = f"Step4_{self.jc_name}_beamspotJet"
                        chainSteps = [self.getStep(flags, 4, stepName, [getBeamspotVtxCfg])]

                else:
                        key = self.chainPart['addInfo'][0] + "_" + self.chainPart['l2IDAlg'][0] #TODO: hardcoded index

                        steps=stepDictionary[key]
                        for step in steps:
                                chainstep = getattr(self, step)(flags)
                                chainSteps+=[chainstep]
                        
                myChain = self.buildChain(chainSteps)
                return myChain

        def getStepDictionary(self):
                # --------------------
                # define here the names of the steps and obtain the chainStep configuration
                # --------------------
                stepDictionary = {
                        "allTE_trkfast":['getAllTEStep'],
                        "trkFS_trkfast":['getTrkFSStep'],  
                }
                return stepDictionary
                
        # --------------------
        # Configuration TrkFS step
        # --------------------
        def getTrkFSStep(self, flags):
                return self.getStep(flags,1,"trkFS_trkfast",[trkFS_trkfast_Cfg])

        # --------------------
        # Configuration of costmonitor (costmonitor ?? but isn't this is the actua chain configuration ??)
        # --------------------
        def getAllTEStep(self, flags):
                return self.getStep(flags,1,"allTE_trkfast",[allTE_trkfast_Cfg])
