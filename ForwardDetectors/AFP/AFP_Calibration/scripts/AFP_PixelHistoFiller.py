#!/usr/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

"""
The AFP Calibration Loop, filling the histograms, usage e.g.:
    athena AFP_Calibration/AFP_PixelHistoFiller.py --filesInput="/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-1._0007.root"
"""


# it's important to have "POOLAccess" here. With "AthenaAccess" or others, the EventContext won't be filled properly
jps.AthenaCommonFlags.AccessMode = "POOLAccess" 

# overwrite with --evtMax
jps.AthenaCommonFlags.EvtMax=-1
	
# overwrite with --filesInput
jps.AthenaCommonFlags.FilesInput = ["/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-1._0007.root"]
	
algo = CfgMgr.AFP_PixelHistoFiller("AFP_PixelHistoFiller")
athAlgSeq += algo

