# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, InEventRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from TrigTrackingHypo.IDCalibHypoConfig import IDCalibHypoToolFromDict, createIDCalibHypoAlg
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from ..CommonSequences.FullScanInDetConfig import commonInDetFullScanCfg
from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker

from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg, jetmetTopoClusteringCfg
from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstRecoAlgCfg
from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstHypoToolGen
from TrigT2CaloCommon.CaloDef import clusterFSInputMaker


def getLArNoiseBurstRecoCfg(flags):
    acc = InEventRecoCA("LArNoiseBurstRecoSequence", inputMaker=clusterFSInputMaker())
    cells_name = 'CaloCellsFS' 
    acc.mergeReco(hltCaloCellMakerCfg(flags=flags, name="HLTCaloCellMakerFS", roisKey=''))
    acc.mergeReco(jetmetTopoClusteringCfg(flags, RoIs=''))
    acc.mergeReco(TrigLArNoiseBurstRecoAlgCfg(flags, cells_name))
    return acc


# --------------------
# LArNoiseBurst configuration
# --------------------
@AccumulatorCache
def getLArNoiseBurstSequenceCfg(flags):

    hypoAlg = CompFactory.TrigLArNoiseBurstAlg("NoiseBurstAlg")
    InEventReco = InEventRecoCA("LArNoiseBurstRecoSequence", inputMaker=clusterFSInputMaker())

    noiseBurstRecoSeq = getLArNoiseBurstRecoCfg(flags)

    InEventReco.mergeReco(noiseBurstRecoSeq)
    selAcc = SelectionCA("LArNoiseBurstMenuSequence")
    selAcc.mergeReco(InEventReco)
    selAcc.addHypoAlgo(hypoAlg)
    
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigLArNoiseBurstHypoToolGen)
    
def getLArNoiseBurst(flags):
    if isComponentAccumulatorCfg():
       return getLArNoiseBurstSequenceCfg(flags)
    else:
       return menuSequenceCAToGlobalWrapper(getLArNoiseBurstSequenceCfg,flags)
#----------------------------------------------------------------

# --------------------
# LArPS Noise Detection EM configuration
# --------------------

@AccumulatorCache
def getCaloAllEMLayersPSSequenceCfg(flags,doAllorAllEM=False):

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    nameselAcc = "LArPSSequence_AllEM"
    namerecoAcc = "fastCaloInViewSequenceAllEM"
    hypoAlgName = "TrigL2CaloLayersAlg_AllEM"
    output = "HLT_LArPS_AllCaloEMClusters"
    if doAllorAllEM :
       nameselAcc = "LArPSSequence_AllEM"
       namerecoAcc = "fastCaloInViewSequenceAll"
       hypoAlgName = "TrigL2CaloLayersAlg_All"
       output = "HLT_LArPS_AllCaloClusters"
    selAcc = SelectionCA(nameselAcc)
    InViewRoIs="EMCaloRoIs"
    reco = InViewRecoCA(namerecoAcc,InViewRoIs=InViewRoIs)
    reco.mergeReco(fastCaloVDVCfg(InViewRoIs=InViewRoIs))
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output,doAllEm=not doAllorAllEM,doAll=doAllorAllEM))

    selAcc.mergeReco(reco)
    
    from TrigCaloHypo.TrigCaloHypoConfig import TrigL2CaloLayersHypoToolGen
    TrigL2CaloLayersAlg = CompFactory.TrigL2CaloLayersAlg(hypoAlgName)
    TrigL2CaloLayersAlg.TrigClusterContainerKey = output
    selAcc.addHypoAlgo(TrigL2CaloLayersAlg)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigL2CaloLayersHypoToolGen)

def getCaloAllEMLayersPS(flags):
    if isComponentAccumulatorCfg():
       return getCaloAllEMLayersPSSequenceCfg(flags,doAllorAllEM=False)
    else:
       return menuSequenceCAToGlobalWrapper(getCaloAllEMLayersPSSequenceCfg,flags,doAllorAllEM=False)

def getCaloAllLayersPS(flags):
    if isComponentAccumulatorCfg():
       return getCaloAllEMLayersPSSequenceCfg(flags,doAllorAllEM=True)
    else:
       return menuSequenceCAToGlobalWrapper(getCaloAllEMLayersPSSequenceCfg,flags,doAllorAllEM=True)

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
        return self.getStep(flags,2,'IDCalibFTFCfg',[IDCalibFTFSeq])

    def getIDCalibTrigger(self, flags, i):
        return self.getStep(flags,3,'IDCalibTriggerCfg',[IDCalibTriggerSeq])

#----------------------------------------------------------------

# --------------------
# IDCalib trigger configurations
# --------------------

@AccumulatorCache
def IDCalibTriggerCfg(flags):
    DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_IDCalib_HypoOnlyStep" )
    DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

    reco = InEventRecoCA('IDCalibEmptySeq_reco',inputMaker=DummyInputMakerAlg)

    theHypoAlg = createIDCalibHypoAlg(flags, "IDCalibHypo")
    theHypoAlg.tracksKey = getInDetTrigConfig('fullScan').tracks_FTF()

    selAcc = SelectionCA('IDCalibEmptySeq_sel')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)

    msca = MenuSequenceCA(
        flags, selAcc, 
        HypoToolGen=IDCalibHypoToolFromDict,
    )
    return msca
    
def IDCalibTriggerSeq(flags):
    if isComponentAccumulatorCfg():
        return IDCalibTriggerCfg(flags)
    else:
        return menuSequenceCAToGlobalWrapper(IDCalibTriggerCfg, flags)


# --------------------

@AccumulatorCache
def IDCalibFTFCfg(flags):
    reco = InEventRecoCA('IDCalibTrkrecoSeq_reco',inputMaker=getTrackingInputMaker("ftf"))
    reco.mergeReco(commonInDetFullScanCfg(flags))

    selAcc = SelectionCA('IDCalibTrkrecoSeq')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(CompFactory.TrigStreamerHypoAlg("IDCalibTrkDummyStream"))

    msca = MenuSequenceCA(
        flags, selAcc,
        HypoToolGen = lambda chainDict: CompFactory.TrigStreamerHypoTool(chainDict['chainName'])
    )
    return msca

def IDCalibFTFSeq(flags):
    if isComponentAccumulatorCfg():
        return IDCalibFTFCfg(flags)
    else:
        return menuSequenceCAToGlobalWrapper(IDCalibFTFCfg, flags)

#----------------------------------------------------------------

# --------------------
# HLT step for the AcceptedEvents chains
# --------------------
@AccumulatorCache
def acceptedEventsCfg(flags):
    '''
    Return MenuSequenceCA for an HLT step used by the AcceptedEvents chains. This step is a trivial
    always-reject hypo with no reco. The step itself should be noop as only the HLTSeeding and the
    end-of-event sequence parts of AcceptedEvents chains are actually used.
    '''
    # Implementation identical to the timeburner chain but with zero sleep time

    inputMaker = CompFactory.InputMakerForRoI(
        "IM_AcceptedEvents",
        RoITool = CompFactory.ViewCreatorInitialROITool(),
        RoIs="AcceptedEventsRoIs",
    )
    reco = InEventRecoCA('AcceptedEvents_reco',inputMaker=inputMaker)
    # TimeBurner alg works as a reject-all hypo
    selAcc = SelectionCA('AcceptedEventsSequence')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(
        TimeBurnerCfg(
            flags,
            name="AcceptedEventsHypo",
            SleepTimeMillisec = 0,
        )
    )

    msca = MenuSequenceCA(
        flags, selAcc,
        HypoToolGen=TimeBurnerHypoToolGen
    )
    return msca

def acceptedEventsSequence(flags):

    if isComponentAccumulatorCfg():
        return acceptedEventsCfg(flags)
    else:
        return menuSequenceCAToGlobalWrapper(acceptedEventsCfg, flags)
