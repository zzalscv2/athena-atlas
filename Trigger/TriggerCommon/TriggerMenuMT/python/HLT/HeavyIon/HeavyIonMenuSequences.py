# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.CFElements import seqAND

def HIFwdGapMenuSequence():

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

  return MenuSequence(Sequence    = theSequence,
                      Maker       = HIFwdGapInputMakerAlg,
                      Hypo        = theHIFwdGapHypo,
                      HypoToolGen = TrigHIFwdGapHypoToolFromDict)

