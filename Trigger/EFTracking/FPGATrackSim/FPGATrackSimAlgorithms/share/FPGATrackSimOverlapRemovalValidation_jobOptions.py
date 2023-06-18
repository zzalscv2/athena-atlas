# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
#
#   FPGATrackSimOverlapRemovalValAlg job options file
#   @file FPGATrackSimAlgorithms/share/FPGATrackSimOverlapRemovalValidation_jobOptions.py
#   @author Zhaoyuan.Cui@cern.ch
#   @date Dec. 4, 2020
#   @brief jobOptions for overlap removal validation algorithm
#==============================================================

import os
import glob

from FPGATrackSimConfig.formatMessageSvc import formatMessageSvc
formatMessageSvc()

import FPGATrackSimConfig.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig
import FPGATrackSimBanks.FPGATrackSimBankConfig as FPGATrackSimBankConfig
import FPGATrackSimConfig.FPGATrackSimConfigCompInit as FPGATrackSimConfig

tags = FPGATrackSimTagConfig.getTags(stage='bank')
map_tag = tags['map']
bank_tag = tags['bank']
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)
BankSvc = FPGATrackSimBankConfig.addBankSvc(map_tag, bank_tag)

from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from AthenaCommon.AppMgr import ToolSvc, ServiceMgr
from AthenaCommon.Constants import DEBUG

from GaudiSvc.GaudiSvcConf import THistSvc
# THistSvc configuration
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += ["MONITOROUT DATAFILE='OR_monitor_out_${FPGATrackSimORSampleType}_${FPGATrackSimORAlgo}_grouping-${FPGATrackSimORnhg}.root' OPT='RECREATE'"]

# InputFile
FPGATrackSimInputFile=[]
if 'FPGATrackSimInputFile' in os.environ :
     for ex in os.environ['FPGATrackSimInputFile'].split(','):
         files=glob.glob(ex)
         FPGATrackSimInputFile += files
else :
     FPGATrackSimInputFile = ["httsim_loghits_wra.OUT.root"]
print("Input file:  ",  FPGATrackSimInputFile)

ES = FPGATrackSimConfig.addEvtSelSvc(map_tag)
ES.sampleType=os.environ["FPGATrackSimORSampleType"]

# Read in file
from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimOutputHeaderTool
FPGATrackSimReadInput = FPGATrackSimOutputHeaderTool("FPGATrackSimReadInput", OutputLevel = DEBUG)
FPGATrackSimReadInput.InFileName=FPGATrackSimInputFile
FPGATrackSimReadInput.RWstatus="READ"
ToolSvc += FPGATrackSimReadInput

# OR tools
from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimOverlapRemovalTool
ORtool = FPGATrackSimOverlapRemovalTool(OutputLevel = DEBUG)
ORtool.ORAlgo = os.environ["FPGATrackSimORAlgo"]
ORtool.NumOfHitPerGrouping = int(os.environ["FPGATrackSimORnhg"])
ToolSvc += ORtool

from FPGATrackSimMonitor.FPGATrackSimMonitorConf import FPGATrackSimOverlapRemovalMonitorTool
ORmonitor = FPGATrackSimOverlapRemovalMonitorTool(OutputLevel = DEBUG)
ORmonitor.BarcodeFracCut = 0.5
ToolSvc += ORmonitor

from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimOverlapRemovalValAlg
theAlg = FPGATrackSimOverlapRemovalValAlg(OutputLevel = DEBUG);
theAlg.InputTool = FPGATrackSimReadInput
theAlg.OverlapRemoval = ORtool
theAlg.OverlapRemovalMonitor = ORmonitor

theJob +=theAlg
############################################################
