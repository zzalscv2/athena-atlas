#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetSelectionConfig.py
@author M. Aparo
@date 02-10-2023
@brief CA-based python configurations for selection tools in this package
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def RoiSelectionToolCfg( flags, name="RoiSelectionTool", **kwargs ) :
    '''
    CA-based configuration for the Tool to retrieve and select RoIs 
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "RoiKey",        flags.PhysVal.IDTPM.currentTrkAna.RoiKey )
    kwargs.setdefault( "ChainLeg",      flags.PhysVal.IDTPM.currentTrkAna.ChainLeg )
    kwargs.setdefault( "doTagNProbe",   flags.PhysVal.IDTPM.currentTrkAna.doTagNProbe )
    kwargs.setdefault( "RoiKeyTag",     flags.PhysVal.IDTPM.currentTrkAna.RoiKeyTag )
    kwargs.setdefault( "ChainLegTag",   flags.PhysVal.IDTPM.currentTrkAna.ChainLegTag )
    kwargs.setdefault( "RoiKeyProbe",   flags.PhysVal.IDTPM.currentTrkAna.RoiKeyProbe )
    kwargs.setdefault( "ChainLegProbe", flags.PhysVal.IDTPM.currentTrkAna.ChainLegProbe )

    acc.setPrivateTools( CompFactory.IDTPM.RoiSelectionTool( name, **kwargs ) )
    return acc


def TrackRoiSelectionToolCfg( flags, name="TrackRoiSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    kwargs.setdefault( "TriggerTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )

    acc.setPrivateTools( CompFactory.IDTPM.TrackRoiSelectionTool( name, **kwargs ) )
    return acc


def TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    ## TODO - to be uncommented in later MRs
    ## offline track-object selection
    #from InDetTrackPerfMon.ConfigUtils import getObjectStr
    #if getObjectStr( flags ):
    #    kwargs.setdefault( "DoObjectSelection", True )
    #
    #    if "TrackObjectSelectionTool" not in kwargs:
    #       kwargs.setdefault( "TrackObjectSelectionTool", acc.popToolsAndMerge(
    #           InDetTrackObjectSelectionToolCfg( flags,
    #               name="TrackObjectSelectionTool" + flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    acc.setPrivateTools( CompFactory.IDTPM.TrackQualitySelectionTool( name, **kwargs ) )
    return acc


## TODO - to be uncommented in later MRs
#def InDetTrackObjectSelectionToolCfg( flags, name="TrackObjectSelectionTool", **kwargs ):
#    acc = ComponentAccumulator()
#
#    from InDetTrackPerfMon.ConfigUtils import getObjectStr
#    objectStr = getObjectStr( flags )
#    kwargs.setdefault( "ObjectType",     objectStr )
#    kwargs.setdefault( "ObjectQuality",  flags.PhysVal.IDTPM.currentTrkAna.ObjectQuality )
#    if "Tau" in objectStr:
#        kwargs.setdefault( "TauType",    flags.PhysVal.IDTPM.currentTrkAna.TauType )
#        kwargs.setdefault( "TauNprongs", flags.PhysVal.IDTPM.currentTrkAna.TauNprongs )
#
#    acc.setPrivateTools( CompFactory.IDTPM.InDetTrackObjectSelectionTool( name, **kwargs ) )
#    return acc
