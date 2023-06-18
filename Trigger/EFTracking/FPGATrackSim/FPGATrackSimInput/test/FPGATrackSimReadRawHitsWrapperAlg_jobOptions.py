# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
#
# FPGATrackSimWrapper job options file
#
#==============================================================
import os
import glob

import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig

tags = FPGATrackSimTagConfig.getTags(stage='map')
map_tag = tags['map']
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)

from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG


#input
InputFPGATrackSimRawHitFile = []
if 'InputFPGATrackSimRawHitFile' in os.environ:
    for ex in os.environ['InputFPGATrackSimRawHitFile'].split(','):
        files = glob.glob(ex)
        if files:
            InputFPGATrackSimRawHitFile += files
        else:
            InputFPGATrackSimRawHitFile += [ex]
else :
    InputFPGATrackSimRawHitFile = ["httsim_rawhits_wrap.root"]

msg = logging.getLogger('FPGATrackSimReadRawHitsWrapperAlg')
msg.info("Input file %r",  InputFPGATrackSimRawHitFile)

#output
OutputFPGATrackSimRawHitFile = []
if 'OutputFPGATrackSimRawHitFile' in os.environ :
     for ex in os.environ['OutputFPGATrackSimRawHitFile'].split(','):
        files = glob.glob(ex)
        if files:
            OutputFPGATrackSimRawHitFile += files
        else:
            OutputFPGATrackSimRawHitFile += [ex]
else :
     OutputFPGATrackSimRawHitFile = ["httsim_rawhits_wrap.OUT.root"]
msg.info("OutFPGATrackSim file %r",  OutputFPGATrackSimRawHitFile)



from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimInputHeaderTool
FPGATrackSimReadInput = FPGATrackSimInputHeaderTool("FPGATrackSimReadInput", OutputLevel = DEBUG)
FPGATrackSimReadInput.InFileName=InputFPGATrackSimRawHitFile
FPGATrackSimReadInput.RWstatus="READ"
#FPGATrackSimReadInput.ReadTruthTracks = True
ToolSvc += FPGATrackSimReadInput

FPGATrackSimWriteOutput = FPGATrackSimInputHeaderTool("FPGATrackSimWriteInput", OutputLevel = DEBUG)
FPGATrackSimWriteOutput.InFileName=OutputFPGATrackSimRawHitFile
FPGATrackSimWriteOutput.RWstatus="RECREATE"
ToolSvc += FPGATrackSimWriteOutput

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimRawHitsWrapperAlg
wrapper = FPGATrackSimRawHitsWrapperAlg(OutputLevel = DEBUG)

wrapper.InputTool = FPGATrackSimReadInput
wrapper.OutputTool = FPGATrackSimWriteOutput

theJob += wrapper

###############################################################
