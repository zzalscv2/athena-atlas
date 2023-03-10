# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

""" InDetTrigConfigRecLoadToolsPost
    various tools for postprocessing
"""

__author__ = "J. Masik"
__version__= "$Revision: 1.2 $"
__doc__    = "InDetTrigConfigRecLoadToolsPost"


# common things
from AthenaCommon.AppMgr import ToolSvc
from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
from InDetTrigRecExample.ConfiguredNewTrackingTrigCuts import EFIDTrackingCuts
InDetTrigCutValues = EFIDTrackingCuts

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper

from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTestPixelLayerToolInner
from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
InDetTrigTrackSummaryTool = CAtoLegacyPublicToolWrapper(InDetTrigTrackSummaryToolCfg)


# Shared hit computation in the TrackParticleCreatorTool is disabled for consistency with the previous 2022 config
# This can be enabled with
# DoSharedSiHits = InDetTrigFlags.doSharedHits(),
# AssociationMapName = "TrigInDetPRDtoTrackMap"

from TrkParticleCreator.TrkParticleCreatorConf import Trk__TrackParticleCreatorTool
InDetTrigParticleCreatorTool = \
    Trk__TrackParticleCreatorTool( name = "InDetTrigParticleCreatorTool",
                                   TrackSummaryTool = InDetTrigTrackSummaryTool,
                                   KeepParameters = False,
                                   DoSharedSiHits = False
                                   #ForceTrackSummaryUpdate = False,
                                   )

ToolSvc += InDetTrigParticleCreatorTool
if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigParticleCreatorTool)

InDetTrigParticleCreatorToolWithSummary = \
    Trk__TrackParticleCreatorTool( name = "InDetTrigParticleCreatorToolWithSummary",
                                   TrackSummaryTool = InDetTrigTrackSummaryTool,
                                   TestPixelLayerTool = InDetTrigTestPixelLayerToolInner,
                                   KeepParameters = True,
                                   ComputeAdditionalInfo = True,
                                   DoSharedSiHits = False
                                   #ForceTrackSummaryUpdate = True,
                                   )

ToolSvc += InDetTrigParticleCreatorToolWithSummary
if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigParticleCreatorToolWithSummary)

InDetTrigTRT_ElectronPidTool = None
from AthenaCommon.DetFlags import DetFlags
if DetFlags.haveRIO.TRT_on() :
    from TrigInDetConfig.InDetTrigCollectionKeys import TrigTRTKeys
    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTRTCalDbTool

    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTRTStrawStatusSummaryTool
    from TRT_ElectronPidTools.TRT_ElectronPidToolsConf import InDet__TRT_ElectronPidToolRun2,InDet__TRT_LocalOccupancy,TRT_ToT_dEdx
    InDetTrigTRT_LocalOccupancy = InDet__TRT_LocalOccupancy(name ="InDetTrig_TRT_LocalOccupancy",
                                                            isTrigger = True,
                                                            TRT_DriftCircleCollection = TrigTRTKeys.DriftCircles,
                                                            TRTCalDbTool = InDetTRTCalDbTool)
    ToolSvc += InDetTrigTRT_LocalOccupancy

    InDetTrigTRT_ToT_dEdx = TRT_ToT_dEdx(name = "InDetTrig_TRT_ToT_dEdx",
                                         AssociationTool = ToolSvc.InDetTrigPrdAssociationTool,
                                         TRTStrawSummaryTool = InDetTrigTRTStrawStatusSummaryTool,
                                         TRT_LocalOccupancyTool = InDetTrigTRT_LocalOccupancy)
    ToolSvc += InDetTrigTRT_ToT_dEdx

    InDetTrigTRT_ElectronPidTool = InDet__TRT_ElectronPidToolRun2(name   = "InDetTrigTRT_ElectronPidTool",
                                                                  TRT_LocalOccupancyTool = InDetTrigTRT_LocalOccupancy,
                                                                  TRTStrawSummaryTool= InDetTrigTRTStrawStatusSummaryTool,
                                                                  TRT_ToT_dEdx_Tool = InDetTrigTRT_ToT_dEdx,
                                                                  MinimumTrackPtForNNPid = 2000., # default 2 GeV
                                                                  CalculateNNPid = InDetTrigFlags.doTRTPIDNN() )

    ToolSvc += InDetTrigTRT_ElectronPidTool
    if (InDetTrigFlags.doPrintConfigurables()):
        print (     InDetTrigTRT_ElectronPidTool)

InDetTrigParticleCreatorToolWithSummaryTRTPid = \
    Trk__TrackParticleCreatorTool( name = "InDetTrigParticleCreatorToolWithSummaryTRTPid",
                                   TrackSummaryTool = InDetTrigTrackSummaryTool,
                                   TestPixelLayerTool = InDetTrigTestPixelLayerToolInner,
                                   KeepParameters = True,
                                   ComputeAdditionalInfo = True,
                                   TRT_ElectronPidTool   = InDetTrigTRT_ElectronPidTool,
                                   DoSharedSiHits = False
                                   #ForceTrackSummaryUpdate = True,
                                   )

ToolSvc += InDetTrigParticleCreatorToolWithSummaryTRTPid
if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigParticleCreatorToolWithSummaryTRTPid)

from InDetRecExample.TrackingCommon import makePublicTool,makeName
@makePublicTool
def getInDetTrigFullLinearizedTrackFactory(name='InDetFullLinearizedTrackFactory', **kwargs) :
    the_name                    = makeName( name, kwargs)
    if 'Extrapolator' not in kwargs :
      from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigExtrapolator
      kwargs.setdefault('Extrapolator', InDetTrigExtrapolator) # @TODO AtlasExtrapolator ?

    from TrkVertexFitterUtils.TrkVertexFitterUtilsConf import Trk__FullLinearizedTrackFactory
    return Trk__FullLinearizedTrackFactory(the_name, **kwargs)

@makePublicTool
def getInDetTrigTrackToVertexIPEstimator(name='InDetTrackToVertexIPEstimator', **kwargs) :
    the_name                    = makeName( name, kwargs)
    if 'Extrapolator' not in kwargs :
      from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigExtrapolator
      kwargs.setdefault('Extrapolator', InDetTrigExtrapolator) # @TODO AtlasExtrapolator ?
    if 'LinearizedTrackFactory' not in kwargs :
        kwargs.setdefault('LinearizedTrackFactory', getInDetTrigFullLinearizedTrackFactory() )
    from TrkVertexFitterUtils.TrkVertexFitterUtilsConf import Trk__TrackToVertexIPEstimator
    return Trk__TrackToVertexIPEstimator( the_name, **kwargs)


# from TrkVertexFitterUtils.TrkVertexFitterUtilsConf import Trk__DetAnnealingMaker
# InDetTrigAnnealingMaker = Trk__DetAnnealingMaker(name = "InDetTrigTrkAnnealingMaker")
# #TODO this setting is different in the R3 legacy
# InDetTrigAnnealingMaker.SetOfTemperatures = [64.,16.,4.,2.,1.5,1.] # not default
# ToolSvc += InDetTrigAnnealingMaker

                            

