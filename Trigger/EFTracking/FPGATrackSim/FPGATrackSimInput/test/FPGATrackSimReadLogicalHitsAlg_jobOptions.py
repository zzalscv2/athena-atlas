# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
#
# FPGATrackSimWrapper job options file
#
#==============================================================


from AthenaCommon.AppMgr import ToolSvc
import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig

tags = FPGATrackSimTagConfig.getTags(stage='map')
map_tag = tags['map']
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)

from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()


import glob
import os
from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

#input
InputFPGATrackSimLogHitFile = []
if 'InputFPGATrackSimLogHitFile' in os.environ:
    for ex in os.environ['InputFPGATrackSimLogHitFile'].split(','):
        files = glob.glob(ex)
        if files:
            InputFPGATrackSimLogHitFile += files
        else:
            InputFPGATrackSimLogHitFile += [ex]
else :
    InputFPGATrackSimLogHitFile = ["httsim_loghits_wrap.OUT.root"]

msg = logging.getLogger('FPGATrackSimReadLogicalHitsAlg')
msg.info("Input file %r",  InputFPGATrackSimLogHitFile)


from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimOutputHeaderTool
FPGATrackSimReadInput = FPGATrackSimOutputHeaderTool("FPGATrackSimReadInput", OutputLevel = DEBUG)
FPGATrackSimReadInput.InFileName=InputFPGATrackSimLogHitFile
FPGATrackSimReadInput.RWstatus="READ"
ToolSvc += FPGATrackSimReadInput

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimReadLogicalHitsAlg
wrapper = FPGATrackSimReadLogicalHitsAlg(OutputLevel = DEBUG)
wrapper.InputTool  = FPGATrackSimReadInput

theJob += wrapper

###############################################################
