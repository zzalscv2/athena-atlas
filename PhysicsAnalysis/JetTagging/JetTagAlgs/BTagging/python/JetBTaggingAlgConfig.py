# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from BTagging.BTagToolConfig import BTagToolCfg
from BTagging.BTagLightSecVertexingConfig import BTagLightSecVtxToolCfg


def JetBTaggingAlgCfg(ConfigFlags,
                      BTaggingCollection,
                      JetCollection,
                      JetColNoJetsSuffix,
                      PrimaryVertexCollectionName,
                      TaggerList,
                      Tracks,
                      Muons,
                      VxSecVertexInfoNameList,
                      secVtxFinderxAODBaseNameList,
                      secVtxFinderTrackNameList,
                      OutgoingTracks="BTagTrackToJetAssociator",
                      OutgoingMuons="Muons"):

    SetupScheme = ConfigFlags.BTagging.databaseScheme

    acc = ComponentAccumulator()

    options = {}
    options['BTagTool'] = acc.popToolsAndMerge(BTagToolCfg(
        ConfigFlags, TaggerList, PrimaryVertexCollectionName, SetupScheme))

    # setup the secondary vertexing tool
    options['BTagSecVertexing'] = acc.popToolsAndMerge(BTagLightSecVtxToolCfg(ConfigFlags, 'LightSecVx'+ConfigFlags.BTagging.GeneralToolSuffix, JetCollection, VxSecVertexInfoNameList, secVtxFinderxAODBaseNameList, secVtxFinderTrackNameList, PrimaryVertexCollectionName))

    # Set remaining options
    options['JetCollectionName'] = JetCollection
    options['IncomingTracks'] = Tracks
    options['OutgoingTracks'] = OutgoingTracks
    options['IncomingMuons'] = Muons
    options['OutgoingMuons'] = OutgoingMuons
    options['JetCalibrationName'] = (
        ConfigFlags.BTagging.forcedCalibrationChannel or JetColNoJetsSuffix
    )
    options['BTagSVCollectionName'] = BTaggingCollection + 'SecVtx'
    options['BTagJFVtxCollectionName'] = BTaggingCollection + 'JFVtx'

    if ConfigFlags.BTagging.RunFlipTaggers is True:
        options['BTagSVFlipCollectionName'] = BTaggingCollection + 'SecVtxFlip'
        options['BTagJFVtxFlipCollectionName'] = BTaggingCollection + 'JFVtxFlip'

    options['BTaggingLinkName'] = options['JetCollectionName'] + '.btaggingLink'
    options['BTaggingCollectionName'] = BTaggingCollection
    options['JetLinkName'] = options['BTaggingCollectionName'] + '.jetLink'
    options['name'] = (options['BTaggingCollectionName'] + "_" + options['JetCollectionName'] + ConfigFlags.BTagging.GeneralToolSuffix).lower()

    # -- create main BTagging algorithm
    acc.addEventAlgo(CompFactory.Analysis.JetBTaggingAlg(**options))

    return acc
