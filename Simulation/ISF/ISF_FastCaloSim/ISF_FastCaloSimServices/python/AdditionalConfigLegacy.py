# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Tools configurations for ISF_FastCaloSimServices
KG Tan, 04/12/2012
"""

from AthenaCommon import CfgMgr
from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags


def getPunchThroughTool(name="ISF_PunchThroughTool", **kwargs):
    kwargs.setdefault("FilenameLookupTable"     , ISF_FastCaloSimFlags.PunchThroughParamsInputFilename())
    kwargs.setdefault("FilenameInverseCdf"      , ISF_FastCaloSimFlags.PunchThroughParamsInverseCdfFilename())
    kwargs.setdefault("FilenameInversePca"      , ISF_FastCaloSimFlags.PunchThroughParamsInversePcaFilename())
    kwargs.setdefault("EnergyFactor"            , [ 0.98,  0.831, 0.896, 0.652, 0.717, 1., 0.877, 0.858, 0.919 ]    )
    kwargs.setdefault("DoAntiParticles"         , [ 0,   1,    0,     1,     1,     0,   0,    0,    0 ]    )
    kwargs.setdefault("PunchThroughInitiators"  , [ 211, 321, 311, 310, 130, 2212, 2112]        )
    kwargs.setdefault("InitiatorsMinEnergy"     , [ 65536, 65536, 65536, 65536, 65536, 65536, 65536]                                         )
    kwargs.setdefault("InitiatorsEtaRange"      , [ -3.2,   3.2 ]                               )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("CorrelatedParticle"      , []    )
    kwargs.setdefault("FullCorrelationEnergy"   , [ 100000., 100000., 100000., 100000.,      0., 100000., 100000., 100000., 100000.]    )
    kwargs.setdefault("MinEnergy"               , [   938.3,   135.6,     50.,     50.,   105.7,   939.6, 493.7,   497.6,   497.6 ]    )
    kwargs.setdefault("MaxNumParticles"         , [      -1,      -1,      -1,      -1,      -1,    -1,     -1,     -1,     -1 ]    )
    from AthenaCommon.CfgGetter import getService
    kwargs.setdefault("EnvelopeDefSvc"          , getService('AtlasGeometry_EnvelopeDefSvc')         )
    kwargs.setdefault("BeamPipeRadius"          , 500.                                               )
    return CfgMgr.ISF__PunchThroughTool(name, **kwargs )


def getPunchThroughClassifier(name="ISF_PunchThroughClassifier", **kwargs):
    kwargs.setdefault("ScalerConfigFileName"     , ISF_FastCaloSimFlags.PunchThroughClassifierScalerFilename() )
    kwargs.setdefault("NetworkConfigFileName"     , ISF_FastCaloSimFlags.PunchThroughClassifierNetworkFilename() )
    kwargs.setdefault("CalibratorConfigFileName"    , ISF_FastCaloSimFlags.PunchThroughClassifierCalibratorFilename())
    return CfgMgr.ISF__PunchThroughClassifier(name, **kwargs )


def getEmptyCellBuilderTool(name="ISF_EmptyCellBuilderTool", **kwargs):
    return CfgMgr.EmptyCellBuilderTool(name, **kwargs )


def getNIMatEffUpdator(name="ISF_NIMatEffUpdator", **kwargs):
    return CfgMgr.Trk__NIMatEffUpdator(name, **kwargs )


def getNIPropagator(name="ISF_NIPropagator", **kwargs):
    kwargs.setdefault("MaterialEffects" , False )
    return CfgMgr.Trk__STEP_Propagator(name, **kwargs )


def getNINavigator(name="ISF_NINavigator", **kwargs):
    from TrkExTools.TimedExtrapolator import getNINavigator as navigatorConfig
    return navigatorConfig(name, **kwargs)


def getNITimedExtrapolator(name="ISF_NITimedExtrapolator", **kwargs):
    kwargs.setdefault("MaterialEffectsUpdators", ['ISF_NIMatEffUpdator'])
    kwargs.setdefault("ApplyMaterialEffects", False)
    kwargs.setdefault("STEP_Propagator", 'ISF_NIPropagator')
    kwargs.setdefault("Navigator", 'ISF_NINavigator')
    return CfgMgr.Trk__TimedExtrapolator(name, **kwargs)


def getTimedExtrapolator(name="TimedExtrapolator", **kwargs):
    kwargs.setdefault("MaterialEffectsUpdators", ['ISF_NIMatEffUpdator'])
    kwargs.setdefault("ApplyMaterialEffects", False)
    kwargs.setdefault("STEP_Propagator", 'ISF_NIPropagator')
    kwargs.setdefault("Navigator", 'ISF_NINavigator')
    return CfgMgr.Trk__TimedExtrapolator(name, **kwargs)


def getFastHitConvertTool(name="ISF_FastHitConvertTool", **kwargs):
    from ISF_Algorithms.collection_merger_helpers import generate_mergeable_collection_name
    # Suffix which the FastCaloSim HIT collection will receive
    collectionSuffix = '_FastCaloSim'
    region = 'CALO'

    caloRegionList = ['EMB', 'EMEC', 'FCAL', 'HEC']
    for caloRegion in caloRegionList:
        bareCollectionName = f'LArHit{caloRegion}'
        inputProperty = f'LAr{caloRegion}Hits'
        # Generates a mergeable collection name for different CALO regions
        hitContainerName = generate_mergeable_collection_name(bareCollectionName, collectionSuffix, inputProperty, region)
        kwargs.setdefault(f'{caloRegion.lower()}HitContainername', hitContainerName)

    # For tile
    hitContainerName = generate_mergeable_collection_name('TileHitVec', collectionSuffix, 'TileHits', region)
    kwargs.setdefault('tileHitContainername', hitContainerName)

    from FastCaloSimHit.FastCaloSimHitConf import FastHitConvertTool
    return FastHitConvertTool(name,**kwargs)


def getCaloCellContainerFinalizerTool(name="ISF_CaloCellContainerFinalizerTool", **kwargs):
    from CaloRec.CaloRecConf import CaloCellContainerFinalizerTool
    return CaloCellContainerFinalizerTool(name, **kwargs )


def getCaloCellContainerFCSFinalizerTool(name="ISF_CaloCellContainerFCSFinalizerTool", **kwargs):
    from FastCaloSim.FastCaloSimConf import CaloCellContainerFCSFinalizerTool
    return CaloCellContainerFCSFinalizerTool(name, **kwargs )


def getFastCaloSimV2Tool(name="ISF_FastCaloSimV2Tool", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags

    kwargs.setdefault("CaloCellsOutputName"              , ISF_FastCaloSimFlags.CaloCellsName()   )

    kwargs.setdefault("CaloCellMakerTools_setup"         , [ 'ISF_EmptyCellBuilderTool' ] )
    kwargs.setdefault("CaloCellMakerTools_release"       , [ 'ISF_CaloCellContainerFCSFinalizerTool',
                                                           'ISF_FastHitConvertTool' ])
    kwargs.setdefault("FastCaloSimCaloExtrapolation"     , 'FastCaloSimCaloExtrapolation')

    kwargs.setdefault("ParamSvc", "ISF_FastCaloSimV2ParamSvc")
    # register the FastCaloSim random number streams
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomStream"                     , ISF_FastCaloSimFlags.RandomStreamName())
    kwargs.setdefault("RandomSvc"                        , simFlags.RandomSvcMT())
    kwargs.setdefault("PunchThroughTool"                 , 'ISF_PunchThroughTool')

    kwargs.setdefault("ParticleTruthSvc"                 , simFlags.TruthStrategy.TruthServiceName() )
    return CfgMgr.ISF__FastCaloSimV2Tool(name, **kwargs )
