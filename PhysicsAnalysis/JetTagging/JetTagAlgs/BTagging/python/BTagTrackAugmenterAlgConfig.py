# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg


def BTagTrackAugmenterAlgCfg(flags, TrackCollection = 'InDetTrackParticles', PrimaryVertexCollectionName = 'PrimaryVertices', prefix=None, **options):

    acc = ComponentAccumulator()
    # Minimal configuration
    # @TODO why is options re-initialised to an empty dict ?
    options = {}
    options['name'] = ('BTagTrackAugmenter').lower() + PrimaryVertexCollectionName + TrackCollection
    options['TrackContainer'] = TrackCollection
    options['PrimaryVertexContainer'] = PrimaryVertexCollectionName
    if 'TrackToVertexIPEstimator' not in  options :
        options.setdefault('TrackToVertexIPEstimator',acc.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags, 'TrkToVxIPEstimator') ))
    if 'Extrapolator' not in options :
        options.setdefault('Extrapolator', acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    if prefix is not None:
        options['prefix'] = prefix

    # -- create the track augmenter algorithm
    acc.addEventAlgo(CompFactory.Analysis.BTagTrackAugmenterAlg(**options))

    return acc
