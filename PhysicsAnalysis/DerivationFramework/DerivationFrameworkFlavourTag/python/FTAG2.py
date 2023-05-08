# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_FTAG2.py
# This defines DAOD_FTAG2, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG2 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def FTAG2KernelCfg(ConfigFlags, name='FTAG2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import JetTrackParticleThinningCfg, MuonTrackParticleThinningCfg


    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 18*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 18*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 2 && count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'
    FTAG2LeptonSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG2LeptonSkimmingTool",
            expression = lepton_skimming_expression )
    acc.addPublicTool(FTAG2LeptonSkimmingTool)


    # TrackParticles associated with small-R jets
    track_selection_string = "InDetTrackParticles.numberOfSCTHits + InDetTrackParticles.numberOfPixelHits > 1"
    
    FTAG2Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(ConfigFlags,
        name            = "FTAG2Akt4PFlowJetTPThinningTool",
        StreamName      = kwargs['StreamName'],
        JetKey   = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        TrackSelectionString = track_selection_string,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    FTAG2AktVRJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(ConfigFlags,
        name            = "FTAG2AktVRJetTPThinningTool",
        StreamName      = kwargs['StreamName'],
        JetKey  = "AntiKtVR30Rmax4Rmin02PV0TrackJets",
        SelectionString = 'AntiKtVR30Rmax4Rmin02PV0TrackJets.pt > 7*GeV',
        TrackSelectionString = track_selection_string,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    FTAG2MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "FTAG2MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [
            FTAG2MuonTPThinningTool,
            FTAG2Akt4PFlowJetTPThinningTool,
            FTAG2AktVRJetTPThinningTool,
            ]
    skimmingTools = [
            FTAG2LeptonSkimmingTool,
            ]

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))       
    return acc


def FTAG2Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG2TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Common augmentations
    acc.merge(FTAG2KernelCfg(ConfigFlags, name="FTAG2Kernel", StreamName = 'StreamDAOD_FTAG2', TriggerListsHelper = FTAG2TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG2SlimmingHelper = SlimmingHelper("FTAG2SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    FTAG2SlimmingHelper.SmartCollections = [
            "Electrons",
            "Muons",
            "PrimaryVertices",
            "InDetTrackParticles",
            "AntiKt4EMPFlowJets",
            "BTagging_AntiKt4EMPFlow",
            "AntiKtVR30Rmax4Rmin02PV0TrackJets",
            "BTagging_AntiKtVR30Rmax4Rmin02Track",
            "MET_Baseline_AntiKt4EMPFlow",
            ]

    FTAG2SlimmingHelper.AllVariables = [
            "EventInfo",
            "PrimaryVertices",
            "InDetTrackParticles",
            "BTagging_AntiKt4EMPFlow",
            "BTagging_AntiKtVR30Rmax4Rmin02Track",
            "BTagging_AntiKt4EMPFlowJFVtx",
            "BTagging_AntiKt4EMPFlowSecVtx",
            "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
            ]

    from DerivationFrameworkFlavourTag import FtagBaseContent

    # Static content
    FtagBaseContent.add_static_content_to_SlimmingHelper(FTAG2SlimmingHelper)

    # Add truth containers
    if ConfigFlags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG2SlimmingHelper)

    # Add ExtraVariables
    FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG2SlimmingHelper)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG2SlimmingHelper, 'FTAG2')
    FtagBaseContent.trigger_matching(FTAG2SlimmingHelper, FTAG2TriggerListsHelper, ConfigFlags)


    # Output stream    
    FTAG2ItemList = FTAG2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_FTAG2", ItemList=FTAG2ItemList, AcceptAlgs=["FTAG2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_FTAG2", AcceptAlgs=["FTAG2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

