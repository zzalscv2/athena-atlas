# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#-----------------------------------------------------------------------------
# Athena imports
#-----------------------------------------------------------------------------
from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from AthenaCommon.AppMgr import ToolSvc

import AthenaCommon.Configurable as Configurable
from AthenaCommon.Constants import INFO
Configurable.log.setLevel(INFO)

#--------------------------------------------------------------
# FPGATrackSim Includes
#--------------------------------------------------------------


from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimDetectorTool, FPGATrackSimDumpDetStatusAlgo

import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig

tags = FPGATrackSimTagConfig.getTags(stage='map')
map_tag = tags['map']
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)

FPGATrackSimDet = FPGATrackSimDetectorTool()
ToolSvc += FPGATrackSimDet

FPGATrackSimDumpCond = FPGATrackSimDumpDetStatusAlgo("FPGATrackSimDumpDetStatusAlgo")
FPGATrackSimDumpCond.DumpGlobalToLocalMap = True
theJob += FPGATrackSimDumpCond
