# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.BTagConfig import BTagAlgsCfg, GetTaggerTrainingMap
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg


def FtagJetCollectionsCfg(cfgFlags, jet_cols, pv_cols=None):
    """
    Return a component accumulator which runs tagging in derivations.
    Configures several jet collections at once.
    """

    if pv_cols is None:
        pv_cols = ['PrimaryVertices'] * len(jet_cols)
    if len(pv_cols) != len(jet_cols):
        raise ValueError('PV collection length is not the same as Jets')

    acc = ComponentAccumulator()
    from JetTagCalibration.JetTagCalibConfig import JetTagCalibCfg
    acc.merge(JetTagCalibCfg(cfgFlags))

    if 'AntiKt4EMTopoJets' in jet_cols:
        acc.merge(
            RenameInputContainerEmTopoHacksCfg('oldAODVersion')
        )

    if 'AntiKt4EMPFlowJets' in jet_cols and cfgFlags.BTagging.Trackless:
        acc.merge(
            RenameInputContainerEmPflowHacksCfg('tracklessAODVersion')
        )
    
    #Treating decoration of large-R jets as a special case
    largeRJetCollection = 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets'
    
    if largeRJetCollection in jet_cols:
        jet_col_name_without_Jets = largeRJetCollection.replace('Jets','')
        nnFiles = GetTaggerTrainingMap(cfgFlags, jet_col_name_without_Jets)

        acc.merge(BTagLargeRDecoration(cfgFlags, nnFiles, largeRJetCollection))
        jet_cols.remove(largeRJetCollection)

    for jet_col, pv_col in zip(jet_cols, pv_cols):
        acc.merge(
            getFtagComponent(cfgFlags, jet_col, pv_col)
        )

    return(acc)    

def BTagLargeRDecoration(cfgFlags, nnFiles, jet_name='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets'):

    # Doesn't need to be configurable at the moment
    trackContainer = 'GhostTrack'
    primaryVertexContainer = 'PrimaryVertices'
    variableRemapping = {'BTagTrackToJetAssociator': trackContainer}

    acc = ComponentAccumulator()

    acc.merge(BTagTrackAugmenterAlgCfg(
        cfgFlags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName=primaryVertexContainer,
    ))

    for nnFile in nnFiles:
        # ugly string parsing to get the tagger name
        tagger_name = nnFile.split('/')[-3]

        acc.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.JetTagDecoratorAlg(
                f'{jet_name}{tagger_name}JetTagAlg',
                container=jet_name,
                constituentContainer=trackContainer,
                decorator=CompFactory.FlavorTagDiscriminants.GNNTool(
                    tagger_name,
                    nnFile=nnFile,
                    variableRemapping=variableRemapping,
                    trackLinkType='IPARTICLE'
                ),
            )
        )

    return acc



def getFtagComponent(cfgFlags, jet_col, pv_col):
    """
    Return a component accumulator which runs tagging on a single jet collection.
    """ 

    jet_col_name_without_Jets = jet_col.replace('Jets','')
    track_collection = 'InDetTrackParticles'
    input_muons = 'Muons'
    if cfgFlags.BTagging.Pseudotrack:
        track_collection = 'InDetPseudoTrackParticles'
        input_muons = None

    acc = ComponentAccumulator()
    acc.merge(BTagTrackAugmenterAlgCfg(
        cfgFlags,
        TrackCollection=track_collection,
        PrimaryVertexCollectionName=pv_col,
    ))

    # decorate tracks with leptonID
    acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TrackLeptonDecoratorAlg(
        'TrackLeptonDecoratorAlg',
        trackContainer=track_collection,
    ))

    # decorate detailed truth info
    if cfgFlags.Input.isMC:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
            InDetTrackTruthOriginToolCfg,
        )
        trackTruthOriginTool = acc.popToolsAndMerge(InDetTrackTruthOriginToolCfg(cfgFlags))

        acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TruthParticleDecoratorAlg(
            'TruthParticleDecoratorAlg',
            trackTruthOriginTool=trackTruthOriginTool
        ))
        acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TrackTruthDecoratorAlg(
            'TrackTruthDecoratorAlg',
            trackContainer=track_collection,
            trackTruthOriginTool=trackTruthOriginTool
        ))

    # schedule tagging algorithms
    acc.merge(BTagAlgsCfg(
        inputFlags=cfgFlags,
        JetCollection=jet_col_name_without_Jets,
        nnList=GetTaggerTrainingMap(cfgFlags, jet_col_name_without_Jets),
        trackCollection=track_collection,
        primaryVertices=pv_col,
        muons=input_muons,
        renameTrackJets=True,
        AddedJetSuffix='Jets',
    ))

    return acc


# Valerio's magic hacks for emtopo
def RenameInputContainerEmTopoHacksCfg(suffix):
    acc = ComponentAccumulator()

    #Delete BTagging container read from input ESD
    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    AddressRemappingSvc = AddressRemappingSvc("AddressRemappingSvc")
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.BTagTrackToJetAssociator->AntiKt4EMTopoJets.BTagTrackToJetAssociator_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.JFVtx->AntiKt4EMTopoJets.JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.SecVtx->AntiKt4EMTopoJets.SecVtx_' + suffix]

    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.btaggingLink->AntiKt4EMTopoJets.btaggingLink_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingContainer#BTagging_AntiKt4EMTopo->BTagging_AntiKt4EMTopo_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingAuxContainer#BTagging_AntiKt4EMTopoAux.->BTagging_AntiKt4EMTopo_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexContainer#BTagging_AntiKt4EMTopoSecVtx->BTagging_AntiKt4EMTopoSecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexAuxContainer#BTagging_AntiKt4EMTopoSecVtxAux.->BTagging_AntiKt4EMTopoSecVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexContainer#BTagging_AntiKt4EMTopoJFVtx->BTagging_AntiKt4EMTopoJFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexAuxContainer#BTagging_AntiKt4EMTopoJFVtxAux.->BTagging_AntiKt4EMTopoJFVtx_' + suffix+"Aux."]
    acc.addService(AddressRemappingSvc)
    acc.addService(ProxyProviderSvc(ProviderNames = [ "AddressRemappingSvc" ]))
    return acc

# Valerio's magic hacks for pflow
def RenameInputContainerEmPflowHacksCfg(suffix):
    acc = ComponentAccumulator()

    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    AddressRemappingSvc = AddressRemappingSvc("AddressRemappingSvc")
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.BTagTrackToJetAssociator->AntiKt4EMPFlowJets.BTagTrackToJetAssociator_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.JFVtx->AntiKt4EMPFlowJets.JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.SecVtx->AntiKt4EMPFlowJets.SecVtx_' + suffix]

    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.btaggingLink->AntiKt4EMPFlowJets.btaggingLink_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingContainer#BTagging_AntiKt4EMPFlow->BTagging_AntiKt4EMPFlow_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingAuxContainer#BTagging_AntiKt4EMPFlowAux.->BTagging_AntiKt4EMPFlow_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexContainer#BTagging_AntiKt4EMPFlowSecVtx->BTagging_AntiKt4EMPFlowSecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexAuxContainer#BTagging_AntiKt4EMPFlowSecVtxAux.->BTagging_AntiKt4EMPFlowSecVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexContainer#BTagging_AntiKt4EMPFlowJFVtx->BTagging_AntiKt4EMPFlowJFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexAuxContainer#BTagging_AntiKt4EMPFlowJFVtxAux.->BTagging_AntiKt4EMPFlowJFVtx_' + suffix+"Aux."]
    acc.addService(AddressRemappingSvc)
    acc.addService(ProxyProviderSvc(ProviderNames = [ "AddressRemappingSvc" ]))
    return acc
