# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
#
# FPGATrackSimWrapper job options file
#
#==============================================================


from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from PyJobTransformsCore.runargs import RunArguments
runArgs = RunArguments()

from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

import os
import glob

#--------------------------------------------------------------
# FPGATrackSim Includes
#--------------------------------------------------------------

import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig

tags = FPGATrackSimTagConfig.getTags(stage='map')
map_tag = tags['map']
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)

#--------------------------------------------------------------
# Arguments
#--------------------------------------------------------------

InputFPGATrackSimRawHitFile = []
if 'InputFPGATrackSimRawHitFile' in os.environ:
    for ex in os.environ['InputFPGATrackSimRawHitFile'].split(','):
        files = glob.glob(ex)
        if files:
            InputFPGATrackSimRawHitFile += files
        else:
            InputFPGATrackSimRawHitFile += [ex]
else:
    InputFPGATrackSimRawHitFile = ["httsim_rawhits_wrap.root"]

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
msg = logging.getLogger('FPGATrackSimRawToLogicalHitsWrapperAlg')
msg.info("OutFPGATrackSim file %r ",  OutputFPGATrackSimRawHitFile)



#--------------------------------------------------------------
# Create the components
#--------------------------------------------------------------

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimInputHeaderTool, FPGATrackSimOutputHeaderTool
FPGATrackSimReadInput = FPGATrackSimInputHeaderTool(OutputLevel = DEBUG)
FPGATrackSimReadInput.InFileName=InputFPGATrackSimRawHitFile
FPGATrackSimReadInput.RWstatus="READ"
#FPGATrackSimReadInput.ReadTruthTracks = True
ToolSvc += FPGATrackSimReadInput

FPGATrackSimWriteOutput = FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutput", OutputLevel = DEBUG)
FPGATrackSimWriteOutput.InFileName=OutputFPGATrackSimRawHitFile
FPGATrackSimWriteOutput.RWstatus="RECREATE"
ToolSvc += FPGATrackSimWriteOutput



from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimRawToLogicalHitsTool
FPGATrackSimRawLogic = FPGATrackSimRawToLogicalHitsTool(OutputLevel = DEBUG)
FPGATrackSimRawLogic.SaveOptional = 2
ToolSvc += FPGATrackSimRawLogic



from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimLogicalHitsWrapperAlg
wrapper = FPGATrackSimLogicalHitsWrapperAlg(OutputLevel = DEBUG)

wrapper.InputTool = FPGATrackSimReadInput
wrapper.OutputTool = FPGATrackSimWriteOutput
wrapper.RawToLogicalHitsTool = FPGATrackSimRawLogic
wrapper.Clustering = True
theJob += wrapper

###############################################################
