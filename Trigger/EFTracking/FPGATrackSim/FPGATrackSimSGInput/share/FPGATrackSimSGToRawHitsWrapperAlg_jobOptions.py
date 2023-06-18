# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
#
# FPGATrackSimWrapper job options file
#
#==============================================================

from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()



if hasattr(runArgs,"outputNTUP_FPGATrackSimIPFile") :
    OutputNTUP_FPGATrackSimIPFile = runArgs.outputNTUP_FPGATrackSimIPFile
else :
    OutputNTUP_FPGATrackSimIPFile = "fpgatracksim_rawhits_wrap.root"
print "Output file", OutputNTUP_FPGATrackSimIPFile

from AthenaCommon.AppMgr import ToolSvc
from FPGATrackSimSGInput.FPGATrackSimSGInputConf import FPGATrackSimSGToRawHitsTool
FPGATrackSimSGInput = FPGATrackSimSGToRawHitsTool( maxEta= 3.2, minPt= 0.8*GeV)
FPGATrackSimSGInput.OutputLevel = DEBUG
FPGATrackSimSGInput.ReadTruthTracks = True
ToolSvc += FPGATrackSimSGInput

from FPGATrackSimSGInput.FPGATrackSimSGInputConf import TrigFPGATrackSimRawHitsWrapperAlg
wrapper = TrigFPGATrackSimRawHitsWrapperAlg(OutputLevel = DEBUG,
                             OutFileName = OutputNTUP_FPGATrackSimIPFile)

wrapper.InputTool = FPGATrackSimSGInput
theJob += wrapper

from AthenaCommon.AppMgr import ServiceMgr
from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc()


print theJob
###############################################################
