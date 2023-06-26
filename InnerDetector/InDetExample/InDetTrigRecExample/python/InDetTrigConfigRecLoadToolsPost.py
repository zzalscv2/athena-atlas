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

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper

from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
InDetTrigTrackSummaryTool = CAtoLegacyPublicToolWrapper(InDetTrigTrackSummaryToolCfg)


# Shared hit computation in the TrackParticleCreatorTool is disabled for consistency with the previous 2022 config
# This can be enabled with
# DoSharedSiHits = InDetTrigFlags.doSharedHits(),
# AssociationMapName = "TrigInDetPRDtoTrackMap"

from TrkConfig.TrkParticleCreatorConfig import InDetTrigParticleCreatorToolCfg,InDetTrigParticleCreatorToolTRTPidCfg
InDetTrigParticleCreatorTool = CAtoLegacyPublicToolWrapper(InDetTrigParticleCreatorToolCfg)
InDetTrigParticleCreatorToolTRTPid = CAtoLegacyPublicToolWrapper(InDetTrigParticleCreatorToolTRTPidCfg)


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



                            

