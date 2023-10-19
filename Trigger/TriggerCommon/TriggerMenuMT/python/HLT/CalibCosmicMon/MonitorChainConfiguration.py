# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, menuSequenceCAToGlobalWrapper
from DecisionHandling.DecisionHandlingConf import InputMakerForRoI, ViewCreatorInitialROITool
from AthenaCommon.CFElements import parOR
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen, L1CorrelationAlgCfg
from L1TopoOnlineMonitoring import L1TopoOnlineMonitoringConfig as TopoMonConfig
from AthenaConfiguration.Enums import Format
from TrigHypoCommonTools.TrigHypoCommonTools import TrigGenericHypoToolFromDict
from TrigEDMConfig.TriggerEDMRun3 import recordable

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO, 
# so let's make them functions already now
#----------------------------------------------------------------
def timeBurnerCfg(flags):
    # Input maker - required by the framework, but inputs don't matter for TimeBurner
    inputMaker = CompFactory.InputMakerForRoI("IM_TimeBurner",
                                              RoITool=CompFactory.ViewCreatorInitialROITool(),
                                              RoIs="TimeBurnerInputRoIs",
    )
    reco = InEventRecoCA('TimeBurner_reco',inputMaker=inputMaker)
    # TimeBurner alg works as a reject-all hypo
    selAcc = SelectionCA('TimeBurnerSequence')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(
        TimeBurnerCfg(flags,
                      name="TimeBurnerHypo",
                      SleepTimeMillisec=200
        )
    )

    msca = MenuSequenceCA(flags, selAcc,
                          HypoToolGen=TimeBurnerHypoToolGen)
    return msca

def timeBurnerSequence(flags):
    if isComponentAccumulatorCfg():
        return timeBurnerCfg(flags)
    else:
        return menuSequenceCAToGlobalWrapper(timeBurnerCfg, flags)

def L1TopoOnlineMonitorSequenceCfg(flags, isLegacy):

        recoAlgCfg = TopoMonConfig.getL1TopoLegacyOnlineMonitor if isLegacy else TopoMonConfig.getL1TopoPhase1OnlineMonitor
        recoAlg = RecoFragmentsPool.retrieve(recoAlgCfg, flags)

        topoSimAlgs = []
        # if running on data without L1Sim, need to add L1TopoSim
        if flags.Input.Format is Format.BS and not flags.Trigger.doLVL1:
            topoSimAlgCfg = TopoMonConfig.getL1TopoLegacySimForOnlineMonitor if isLegacy else TopoMonConfig.getL1TopoPhase1SimForOnlineMonitor
            topoSimAlgs = RecoFragmentsPool.retrieve(topoSimAlgCfg, flags)

        # Input maker for FS initial RoI
        inputMaker = InputMakerForRoI("IM_L1TopoOnlineMonitor")
        inputMaker.RoITool = ViewCreatorInitialROITool()
        inputMaker.RoIs="L1TopoOnlineMonitorInputRoIs"

        topoMonSeqAlgs = [inputMaker]
        if topoSimAlgs:
            topoMonSeqAlgs.extend(topoSimAlgs)
        topoMonSeqAlgs.append(recoAlg)

        topoMonSeq = parOR("L1TopoOnlineMonitorSequence", topoMonSeqAlgs)

        hypoAlg = TopoMonConfig.getL1TopoOnlineMonitorHypo(flags)

        return MenuSequence(flags,
            Sequence    = topoMonSeq,
            Maker       = inputMaker,
            Hypo        = hypoAlg,
            HypoToolGen = TopoMonConfig.L1TopoOnlineMonitorHypoToolGen)

def L1TopoLegacyOnlineMonitorSequenceCfg(flags):
    return L1TopoOnlineMonitorSequenceCfg(flags, True)

def L1TopoPhase1OnlineMonitorSequenceCfg(flags):
    return L1TopoOnlineMonitorSequenceCfg(flags, False)


def MistimeMonSequenceCfg(flags):
        inputMaker = CompFactory.InputMakerForRoI("IM_MistimeMon",
                                                  RoITool = CompFactory.ViewCreatorInitialROITool(),
                                                  RoIs="MistimeMonInputRoIs",
        )

        outputName = recordable("HLT_TrigCompositeMistimeJ400")
        reco = InEventRecoCA('Mistime_reco',inputMaker=inputMaker)
        recoAlg = L1CorrelationAlgCfg(flags, "MistimeMonj400", ItemList=['L1_J400'],
                                      TrigCompositeWriteHandleKey=outputName, trigCompPassKey=outputName+".pass",
                                      l1AKey=outputName+".l1a_type", otherTypeKey=outputName+".other_type",
                                      beforeAfterKey=outputName+".beforeafterflag")
        reco.addRecoAlgo(recoAlg)
        selAcc =  SelectionCA("MistimeMonSequence")
        selAcc.mergeReco(reco)

        # Hypo to select on trig composite pass flag
        hypoAlg = CompFactory.TrigGenericHypoAlg("MistimeMonJ400HypoAlg", TrigCompositeContainer=outputName)
        selAcc.addHypoAlgo(hypoAlg)

        return MenuSequenceCA(flags, selAcc,
                HypoToolGen = TrigGenericHypoToolFromDict)

def MistimeMonSequence(flags):
    if isComponentAccumulatorCfg():
        return MistimeMonSequenceCfg(flags)
    else:
        return menuSequenceCAToGlobalWrapper(MistimeMonSequenceCfg, flags)
 

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class MonitorChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        monTypeList = self.chainPart.get('monType')
        if not monTypeList:
            raise RuntimeError('No monType defined in chain ' + self.chainName)
        if len(monTypeList) > 1:
            raise RuntimeError('Multiple monType values defined in chain ' + self.chainName)
        monType = monTypeList[0]

        if monType == 'timeburner':
            chainSteps.append(self.getTimeBurnerStep(flags))
        elif monType == 'l1topodebug':
            chainSteps.append(self.getL1TopoOnlineMonitorStep(flags))
        elif monType == 'mistimemonj400':
            chainSteps.append(self.getMistimeMonStep(flags))
        else:
            raise RuntimeError('Unexpected monType '+monType+' in MonitorChainConfiguration')

        return self.buildChain(chainSteps)

    # --------------------
    # TimeBurner configuration
    # --------------------
    def getTimeBurnerStep(self, flags):
        return self.getStep(flags,1,'TimeBurner',[timeBurnerSequence])

    # --------------------
    # L1TopoOnlineMonitor configuration
    # --------------------
    def getL1TopoOnlineMonitorStep(self, flags):
        isLegacy = 'isLegacyL1' in self.chainPart and 'legacy' in self.chainPart['isLegacyL1']
        sequenceCfg = L1TopoLegacyOnlineMonitorSequenceCfg if isLegacy else L1TopoPhase1OnlineMonitorSequenceCfg
        return self.getStep(flags,1,'L1TopoOnlineMonitor',[sequenceCfg])

    # --------------------
    # MistTimeMon configuration
    # --------------------
    def getMistimeMonStep(self, flags):
        return self.getStep(flags,1,'MistimeMon',[MistimeMonSequence])
