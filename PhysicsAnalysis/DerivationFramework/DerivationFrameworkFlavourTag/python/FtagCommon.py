# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#====================================================================
# FtagCommon.py
# This is to define common objects used by PHYSVAL, FTAG1 and FTAG2.
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo
from DerivationFrameworkMCTruth.MCTruthCommon import addTruth3ContentToSlimmerTool
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
from DerivationFrameworkFlavourTag import FtagBaseContent

# Import common items used in PHYSVAL, FTAG1 and FTAG2 from FtagBaseContent
PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary = FtagBaseContent.PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary
PHYSVAL_FTAG1_FTAG2_ExtraVariables = FtagBaseContent.PHYSVAL_FTAG1_FTAG2_ExtraVariables
PHYSVAL_FTAG1_FTAG2_StaticContent = FtagBaseContent.PHYSVAL_FTAG1_FTAG2_StaticContent

## Common functions used in PHYSVAL, FTAG1 and FTAG2
def add_static_content_to_SlimmingHelper(SlimmingHelper):
    SlimmingHelper.StaticContent = PHYSVAL_FTAG1_FTAG2_StaticContent

def add_truth_to_SlimmingHelper(SlimmingHelper):
    if DerivationFrameworkIsMonteCarlo:
        SlimmingHelper.AppendToDictionary = PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary
        addTruth3ContentToSlimmerTool(SlimmingHelper)
        SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm']

def add_ExtraVariables_to_SlimmingHelper(SlimmingHelper):
    SlimmingHelper.ExtraVariables += PHYSVAL_FTAG1_FTAG2_ExtraVariables
    SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

## Common function used in FTAG1 and FTAG2
def trigger_setup(SlimmingHelper, option=''):
    SlimmingHelper.IncludeTriggerNavigation = False
    SlimmingHelper.IncludeJetTriggerContent = False
    SlimmingHelper.IncludeMuonTriggerContent = False
    SlimmingHelper.IncludeEGammaTriggerContent = False
    SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    SlimmingHelper.IncludeTauTriggerContent = False
    SlimmingHelper.IncludeEtMissTriggerContent = False
    SlimmingHelper.IncludeBJetTriggerContent = False
    SlimmingHelper.IncludeBPhysTriggerContent = False
    SlimmingHelper.IncludeMinBiasTriggerContent = False
    if option == 'FTAG2':
        SlimmingHelper.IncludeMuonTriggerContent = True
        SlimmingHelper.IncludeEGammaTriggerContent = True
        SlimmingHelper.IncludeBJetTriggerContent = True
        SlimmingHelper.IncludeBPhysTriggerContent = True



