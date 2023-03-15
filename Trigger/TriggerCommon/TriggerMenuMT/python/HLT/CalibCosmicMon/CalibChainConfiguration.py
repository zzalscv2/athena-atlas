# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool, algorithmCAToGlobalWrapper
from TrigT2CaloCommon.CaloDef import fastCaloRecoSequence
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen
from DecisionHandling.DecisionHandlingConf import InputMakerForRoI, ViewCreatorInitialROITool
from AthenaCommon.CFElements import seqAND


def getLArNoiseBurstRecoSequence(flags):
    from AthenaCommon.CFElements import parOR
    noiseBurstRecoSeq = parOR('LArNoiseRecoSeq')
    from TriggerMenuMT.HLT.CommonSequences.CaloSequences import cellRecoSequence
    cells_sequence, cells_name = RecoFragmentsPool.retrieve(cellRecoSequence, flags, RoIs='')
    noiseBurstRecoSeq += cells_sequence
    from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstRecoAlgCfg
    noiseBurstRecoSeq += algorithmCAToGlobalWrapper(TrigLArNoiseBurstRecoAlgCfg, flags, cells_name)[0]
    
    return noiseBurstRecoSeq


# --------------------
# LArNoiseBurst configuration
# --------------------
def getLArNoiseBurst(flags):

    hypoAlg = CompFactory.TrigLArNoiseBurstAlg("NoiseBurstAlg")
    from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstHypoToolGen
    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    noiseBurstInputMakerAlg = conf2toConfigurable(clusterFSInputMaker())

    noiseBurstRecoSeq = getLArNoiseBurstRecoSequence(flags)

    noiseBurstMenuSeq =  seqAND("LArNoiseMenuSeq", [noiseBurstInputMakerAlg, noiseBurstRecoSeq])

    return MenuSequence(flags,
            Sequence    = noiseBurstMenuSeq,
            Maker       = noiseBurstInputMakerAlg,
            Hypo        = hypoAlg,
            HypoToolGen = TrigLArNoiseBurstHypoToolGen)

#----------------------------------------------------------------

# --------------------
# LArPS Noise Detection EM configuration
# --------------------
def LArPSAllEMSequence(flags, name="LArPSSequence_AllEM"):

    from TrigT2CaloCommon.CaloDef import fastCalo_AllEM_EVCreator
 
    (LArPSViewsMaker_AllEM, InViewRoIs) = fastCalo_AllEM_EVCreator()
    (LArPSAllEMInViewSequence, sequenceOut) = fastCaloRecoSequence(flags, InViewRoIs,ClustersName="HLT_LArPS_AllCaloEMClusters",doAllEm=True,doAll=False)
    LArPSViewsMaker_AllEM.ViewNodeName = LArPSAllEMInViewSequence.name()
    LArPSAllEMSequence = seqAND(name,[LArPSViewsMaker_AllEM, LArPSAllEMInViewSequence])
    return (LArPSAllEMSequence, LArPSViewsMaker_AllEM, sequenceOut)

def getCaloAllEMLayersPS(flags):

    sequence, viewsmaker, sequenceOut =  RecoFragmentsPool.retrieve(LArPSAllEMSequence,flags)
    
    from TrigCaloHypo.TrigCaloHypoConfig import TrigL2CaloLayersHypoToolGen
    TrigL2CaloLayersAlg = CompFactory.TrigL2CaloLayersAlg("TrigL2CaloLayersAlg_AllEM")
    TrigL2CaloLayersAlg.TrigClusterContainerKey = sequenceOut

    return MenuSequence(flags,
        Sequence    = sequence,
        Maker       = viewsmaker,
        Hypo        = TrigL2CaloLayersAlg,
        HypoToolGen = TrigL2CaloLayersHypoToolGen)

#----------------------------------------------------------------

# --------------------
# LArPS Noise Detection all configuration
# --------------------

def LArPSAllSequence( flags,  name="LArPSSequence_All"):

    from TrigT2CaloCommon.CaloDef import fastCalo_All_EVCreator
    
    (LArPSViewsMaker_All, InViewRoIs) = fastCalo_All_EVCreator()
    (LArPSAllInViewSequence, sequenceOut) = fastCaloRecoSequence(flags, InViewRoIs,ClustersName="HLT_LArPS_AllCaloClusters",doAllEm=False,doAll=True)
    LArPSViewsMaker_All.ViewNodeName = LArPSAllInViewSequence.name()
    LArPSAllSequence = seqAND(name,[LArPSViewsMaker_All, LArPSAllInViewSequence])
    return (LArPSAllSequence, LArPSViewsMaker_All, sequenceOut)


def getCaloAllLayersPS(flags):

    sequence, viewsmaker, sequenceOut =  RecoFragmentsPool.retrieve(LArPSAllSequence,flags)

    from TrigCaloHypo.TrigCaloHypoConfig import TrigL2CaloLayersHypoToolGen
    TrigL2CaloLayersAlg = CompFactory.TrigL2CaloLayersAlg("TrigL2CaloLayersAlg_All")
    TrigL2CaloLayersAlg.TrigClusterContainerKey = sequenceOut

    return MenuSequence(flags,
        Sequence    = sequence,
        Maker       = viewsmaker,
        Hypo        = TrigL2CaloLayersAlg,
        HypoToolGen = TrigL2CaloLayersHypoToolGen)

#----------------------------------------------------------------
class CalibChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):       
                         
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        stepDictionary = self.getStepDictionary()
                
        if 'acceptedevts' in self.chainPart['purpose']:
            steps=stepDictionary['AcceptedEvents']
        elif self.chainPart['purpose'][0] == 'larnoiseburst':
            steps=stepDictionary['LArNoiseBurst']
        elif self.chainPart['purpose'][0] == 'larpsallem':
            steps=stepDictionary['LArPSAllEM']
        elif self.chainPart['purpose'][0] == 'larpsall':
            steps=stepDictionary['LArPSAll']
        elif self.chainPart['purpose'][0] == 'idcalib':
            steps=stepDictionary['IDCalib']
        for i, step in enumerate(steps): 
            chainstep = getattr(self, step)(flags, i)
            chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps)
        return myChain


    def getStepDictionary(self):
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "AcceptedEvents": ['getAcceptedEventsStep'],
            "LArNoiseBurst": ['getAllTEStep'],
            "LArPSAllEM" : ['getCaloAllEMStep'],
            "LArPSAll" : ['getCaloAllStep'],
            "IDCalib": ['getIDCalibEmpty', 'getIDCalibEmpty', 'getIDCalibFTFReco', 'getIDCalibTrigger']
        }
        return stepDictionary


    def getAcceptedEventsStep(self, flags, i):
        return self.getStep(flags,1, 'AcceptedEvents', [acceptedEventsSequence])

    def getAllTEStep(self, flags, i):
        return self.getStep(flags,1, 'LArNoiseBurst', [getLArNoiseBurst])

    def getCaloAllEMStep(self, flags, i):
        return self.getStep(flags,1, 'LArPSALLEM', [getCaloAllEMLayersPS])

    def getCaloAllStep(self, flags, i):
        return self.getStep(flags,1, 'LArPSALL', [getCaloAllLayersPS])

    def getIDCalibEmpty(self, flags, i):
        return self.getEmptyStep(1, 'IDCalibEmptyStep')

    def getIDCalibFTFReco(self, flags, i):
        return self.getStep(flags,2,'IDCalibFTFCfg',[IDCalibFTFCfg])

    def getIDCalibTrigger(self, flags, i):
        return self.getStep(flags,3,'IDCalibTriggerCfg',[IDCalibTriggerCfg])

#----------------------------------------------------------------

# --------------------
# IDCalib trigger configurations
# --------------------

def IDCalibTriggerCfg(flags):

    from TrigTrackingHypo.IDCalibHypoConfig import IDCalibHypoToolFromDict
    from TrigTrackingHypo.IDCalibHypoConfig import createIDCalibHypoAlg
    theHypoAlg = createIDCalibHypoAlg(flags, "IDCalibHypo")

    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanSequence
    ( TrkSeq, InputMakerAlg, sequenceOut ) = RecoFragmentsPool.retrieve(getCommonInDetFullScanSequence,flags)
    theHypoAlg.tracksKey = sequenceOut

    from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
    from AthenaConfiguration.ComponentFactory import CompFactory
    DummyInputMakerAlg = conf2toConfigurable(CompFactory.InputMakerForRoI( "IM_IDCalib_HypoOnlyStep" ))
    DummyInputMakerAlg.RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool())

    return MenuSequence( flags,
                         Sequence    = seqAND("IDCalibEmptySeq",[DummyInputMakerAlg]),
                         Maker       = DummyInputMakerAlg,
                         Hypo        = theHypoAlg,
                         HypoToolGen = IDCalibHypoToolFromDict,
    )

# --------------------

def IDCalibFTFCfg(flags):

    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanSequence
    ( TrkSeq, InputMakerAlg, sequenceOut ) = RecoFragmentsPool.retrieve(getCommonInDetFullScanSequence,flags)

    from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlg
    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator
    HypoAlg = TrigStreamerHypoAlg("IDCalibTrkDummyStream")

    return MenuSequence( flags,
                         Sequence    = seqAND("IDCalibTrkrecoSeq", TrkSeq),
                         Maker       = InputMakerAlg,
                         Hypo        = HypoAlg,
                         HypoToolGen = StreamerHypoToolGenerator )

#----------------------------------------------------------------

# --------------------
# HLT step for the AcceptedEvents chains
# --------------------
def acceptedEventsSequence(flags):
    '''
    Return MenuSequence for an HLT step used by the AcceptedEvents chains. This step is a trivial
    always-reject hypo with no reco. The step itself should be noop as only the HLTSeeding and the
    end-of-event sequence parts of AcceptedEvents chains are actually used.
    '''
    # Implementation identical to the timeburner chain but with zero sleep time
    inputMaker = InputMakerForRoI("IM_AcceptedEvents")
    inputMaker.RoITool = ViewCreatorInitialROITool()
    inputMaker.RoIs="AcceptedEventsRoIs"
    inputMakerSeq = seqAND("AcceptedEventsSequence", [inputMaker])

    # TimeBurner alg works as a reject-all hypo
    hypoAlg = conf2toConfigurable(TimeBurnerCfg(flags,
                                                name="AcceptedEventsHypo",
                                                SleepTimeMillisec = 0))

    return MenuSequence(flags,
        Sequence    = inputMakerSeq,
        Maker       = inputMaker,
        Hypo        = hypoAlg,
        HypoToolGen = TimeBurnerHypoToolGen)
