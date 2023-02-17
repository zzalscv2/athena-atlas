# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.CFElements import seqAND

def HIFwdGapMenuSequence(flags):

  from TriggerMenuMT.HLT.Egamma.TrigEgammaFactories import egammaFSCaloRecoSequence
  egammaFSRecoSequence = egammaFSCaloRecoSequence()

  from DecisionHandling.DecisionHandlingConf import InputMakerForRoI, ViewCreatorInitialROITool
  HIFwdGapInputMakerAlg = InputMakerForRoI('IM_HIFwdGap',
                                           RoIsLink = 'initialRoI',
                                           RoITool = ViewCreatorInitialROITool(),
                                           RoIs='HIFwdGapRoI', # not used in fact
                                           )

  theSequence = seqAND('HIFwdGapSequence', [egammaFSRecoSequence, HIFwdGapInputMakerAlg])

  from TrigHIHypo.TrigHIHypoConf import TrigHIFwdGapHypoAlg
  theHIFwdGapHypo = TrigHIFwdGapHypoAlg()

  from TrigHIHypo.TrigHIFwdGapHypoConfig import TrigHIFwdGapHypoToolFromDict

  return MenuSequence(flags,
                      Sequence    = theSequence,
                      Maker       = HIFwdGapInputMakerAlg,
                      Hypo        = theHIFwdGapHypo,
                      HypoToolGen = TrigHIFwdGapHypoToolFromDict)



def HIFwdGapMenuSequenceCfg(flags):
  from ..Config.MenuComponents import InEventRecoCA, SelectionCA, MenuSequenceCA
  from AthenaConfiguration.ComponentFactory import CompFactory
  from TriggerMenuMT.HLT.Egamma.TrigEgammaFactories import egammaFSHIEventShapeMakerCfg
  from TrigHIHypo.TrigHIFwdGapHypoConfig import TrigHIFwdGapHypoToolFromDict

  recoAcc = InEventRecoCA("HIFwdGapReco")
  recoAcc.mergeReco(egammaFSHIEventShapeMakerCfg(flags))

  selAcc = SelectionCA("HLFwdGapSel")
  selAcc.mergeReco(recoAcc)
  selAcc.addHypoAlgo(CompFactory.TrigHIFwdGapHypoAlg())

  return MenuSequenceCA(flags, selAcc, HypoToolGen = TrigHIFwdGapHypoToolFromDict)
