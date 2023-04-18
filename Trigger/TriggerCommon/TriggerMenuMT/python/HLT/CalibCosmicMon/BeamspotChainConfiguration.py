# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from AthenaCommon.CFElements import seqAND, parOR
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO,
# so let's make them functions already now
#----------------------------------------------------------------

def trkFS_trkfast_Cfg( flags ):
        return allTE_trkfast( flags, signature="FS" )

def allTE_trkfast_Cfg( flags ):
        return allTE_trkfast( flags, signature="BeamSpot" )

def allTE_trkfast( flags, signature="FS" ):
        inputMakerAlg = EventViewCreatorAlgorithm("IM_beamspot_"+signature)
        inputMakerAlg.ViewFallThrough = True
        inputMakerAlg.RoIsLink = "initialRoI"
        inputMakerAlg.RoITool = ViewCreatorInitialROITool()
        inputMakerAlg.InViewRoIs = "beamspotViewRoI_"+signature
        inputMakerAlg.Views      = "beamspotViewRoI_"+signature

        from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
        from TrigT2BeamSpot.T2VertexBeamSpotConfig import T2VertexBeamSpot_activeAllTE

        #Load signature configuration (containing cut values, names of collections, etc)
        from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
        IDTrigConfig = getInDetTrigConfig( signature )

        if(signature == "FS"):
            IDTrigConfig = getInDetTrigConfig("beamSpotFS")

        viewAlgs, viewVerify  = makeInDetTrigFastTracking(flags, config = IDTrigConfig,  rois=inputMakerAlg.InViewRoIs)

        vertexAlg = T2VertexBeamSpot_activeAllTE(flags, "vertex_"+signature )
        vertexAlg.TrackCollection = IDTrigConfig.trkTracks_FTF()

        viewVerify.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+beamspotViewRoI_'+signature ),
                                   ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                   ( 'TagInfo' , 'DetectorStore+ProcessingTags' )]

        # Make sure this is still available at whole-event level
        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        topSequence.SGInputLoader.Load += [( 'TagInfo' , 'DetectorStore+ProcessingTags' )]

        beamspotSequence = parOR( "beamspotSequence_"+signature, viewAlgs+[vertexAlg] )
        inputMakerAlg.ViewNodeName = beamspotSequence.name()
        beamspotViewsSequence = seqAND( "beamspotViewsSequence"+signature, [ inputMakerAlg, beamspotSequence ])


        #hypo
        beamspotHypoAlg = CompFactory.TrigStreamerHypoAlg("BeamspotHypoAlg_"+signature)
        beamspotHypoAlg.RuntimeValidation = False #Needed to avoid the ERROR ! Decision has no 'feature' ElementLink

        # Accept every event
        beamspotHypoToolGen = StreamerHypoToolGenerator


        return  MenuSequence( flags,
                              Sequence    = beamspotViewsSequence,
                              Maker       = inputMakerAlg,
                              Hypo        = beamspotHypoAlg,
                              HypoToolGen = beamspotHypoToolGen )


def getBeamspotVtx(flags):
        signature = "BeamspotJet"

        from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
        IDTrigConfig = getInDetTrigConfig("jet")

        #-- Setting up inputMakerAlg
        from DecisionHandling.DecisionHandlingConf import InputMakerForRoI

        # run at event level
        inputMakerAlg         = InputMakerForRoI("IM_beamspotJet_"+signature)
        inputMakerAlg.RoITool = ViewCreatorInitialROITool()

        #-- Configuring Beamspot vertex alg
        from TrigT2BeamSpot.T2VertexBeamSpotConfig import T2VertexBeamSpot_activeAllTE
        vertexAlg = T2VertexBeamSpot_activeAllTE(flags, "vertex_"+signature )
        vertexAlg.TrackCollection = IDTrigConfig.trkTracks_FTF()

        #-- Setting up beamspotSequence
        beamspotSequence = parOR( "beamspotJetSequence_"+signature, [vertexAlg] )
        beamspotViewsSequence = seqAND( "beamspotJetViewsSequence"+signature, [ inputMakerAlg, beamspotSequence ])

        #-- HypoAlg and Tool
        beamspotHypoAlg = CompFactory.TrigStreamerHypoAlg("BeamspotHypoAlg_"+signature)
        # Reject every event
        def getRejectingHypoTool(chainDict): 
                return CompFactory.TrigStreamerHypoTool(chainDict['chainName'],Pass=False)

        return  MenuSequence( flags,
                              Sequence    = beamspotViewsSequence,
                              Maker       = inputMakerAlg,
                              Hypo        = beamspotHypoAlg,
                              HypoToolGen = getRejectingHypoTool )


def getBeamspotVtxCfg( flags ):
        return getBeamspotVtx(flags)


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
