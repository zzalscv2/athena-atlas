# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_FTAG1.py
# This defines DAOD_FTAG1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG1 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Main algorithm config
def FTAG1KernelCfg(ConfigFlags, name='FTAG1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG1"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))


    # Finally the kernel itself
    thinningTools = []
    
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))       
    return acc


def FTAG1Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG1TriggerListsHelper = TriggerListsHelper()

    # Common augmentations
    acc.merge(FTAG1KernelCfg(ConfigFlags, name="FTAG1Kernel", StreamName = 'StreamDAOD_FTAG1', TriggerListsHelper = FTAG1TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG1SlimmingHelper = SlimmingHelper("FTAG1SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)
    
    FTAG1SlimmingHelper.SmartCollections = [
                                           "Electrons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMPFlowJets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "AntiKt10UFOCSSKJets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                          ]
    FTAG1SlimmingHelper.AllVariables = [
            "EventInfo",
            "PrimaryVertices",
            "InDetTrackParticles",
            "InDetLargeD0TrackParticles",
            "BTagging_AntiKt4EMPFlow",
            "BTagging_AntiKtVR30Rmax4Rmin02Track",
            "BTagging_AntiKt4EMPFlowJFVtx",
            "BTagging_AntiKt4EMPFlowSecVtx",
            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
            "UFOCSSK",
            "TruthParticles",
            "TruthVertices",
            "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
            ]

    if ConfigFlags.BTagging.Pseudotrack:
        FTAG1SlimmingHelper.AllVariables += [ "InDetPseudoTrackParticles" ]

    if ConfigFlags.BTagging.Trackless:
        FTAG1SlimmingHelper.AllVariables += [
                "JetAssociatedPixelClusters",
                "JetAssociatedSCTClusters",
                ]

    from DerivationFrameworkFlavourTag import FtagBaseContent

    # Static content
    FtagBaseContent.add_static_content_to_SlimmingHelper(FTAG1SlimmingHelper)

    # Add truth containers
    if ConfigFlags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG1SlimmingHelper)

    # Add ExtraVariables
    FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG1SlimmingHelper)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG1SlimmingHelper)


    # Output stream    
    FTAG1ItemList = FTAG1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_FTAG1", ItemList=FTAG1ItemList, AcceptAlgs=["FTAG1Kernel"]))

    return acc

