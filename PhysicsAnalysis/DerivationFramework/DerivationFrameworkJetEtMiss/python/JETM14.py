# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM14.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM14TriggerSkimmingToolCfg(ConfigFlags):
    """Configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    singleMuTriggers = TriggerLists.single_mu_Trig(ConfigFlags)

    JETM14TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "JETM14TrigSkimmingTool1",
                                                                                  TriggerListOR          = singleMuTriggers)

    acc.addPublicTool(JETM14TrigSkimmingTool, primary=True)

    return acc

def JETM14StringSkimmingToolCfg(ConfigFlags):
    """Configure the string skimming tool"""

    acc = ComponentAccumulator()

    cutExpression = "(count(Muons.DFCommonMuonPassPreselection && Muons.pt > (20*GeV) && abs(Muons.eta) < 2.7) ) >= 2"

    JETM14StringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM14StringSkimmingTool",
                                                                                      expression = cutExpression)

    acc.addPublicTool(JETM14StringSkimmingTool, primary=True)

    return(acc)


# Main algorithm config
def JETM14KernelCfg(ConfigFlags, name='JETM14Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM14"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTools = []
    skimmingTools.append(acc.getPrimaryAndMerge(JETM14TriggerSkimmingToolCfg(ConfigFlags)))
    skimmingTools.append(acc.getPrimaryAndMerge(JETM14StringSkimmingToolCfg(ConfigFlags)))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Derivation kernel:
    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import LooseMETTriggerDerivationKernelCfg
    acc.merge(LooseMETTriggerDerivationKernelCfg(ConfigFlags, name="JETM14Kernel", skimmingTools = skimmingTools, StreamName = 'StreamDAOD_JETM14'))

    return acc


def JETM14Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM14TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Skimming, thinning, augmentation
    acc.merge(JETM14KernelCfg(ConfigFlags, name="JETM14Kernel", StreamName = 'StreamDAOD_JETM14', TriggerListsHelper = JETM14TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM14SlimmingHelper = SlimmingHelper("JETM14SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import addMETTriggerDerivationContent
    addMETTriggerDerivationContent(JETM14SlimmingHelper, isLoose=True)

    # Trigger content
    JETM14SlimmingHelper.IncludeTriggerNavigation = False
    JETM14SlimmingHelper.IncludeJetTriggerContent = False
    JETM14SlimmingHelper.IncludeMuonTriggerContent = False
    JETM14SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM14SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM14SlimmingHelper.IncludeTauTriggerContent = False
    JETM14SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM14SlimmingHelper.IncludeBJetTriggerContent = False
    JETM14SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM14SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM14ItemList = JETM14SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_JETM14", ItemList=JETM14ItemList, AcceptAlgs=["JETM14Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_JETM14", AcceptAlgs=["JETM14Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

