# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Skeleton top job options for RDO merging
#

#import glob, os, re
import traceback

from AthenaCommon.Logging import logging
merRDOLog = logging.getLogger('MergeRDOS')

merRDOLog.info( '****************** STARTING RDO MERGING *****************' )

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#==============================================================
# Job definition parameters:
#==============================================================
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#Jobs should stop if an include fails.
athenaCommonFlags.AllowIgnoreConfigError.set_Value_and_Lock(False)
if hasattr(runArgs,"inputRDOFile"): athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputRDOFile )
else: raise RuntimeError("No inputRDOFile provided.")
from RecJobTransforms.RDOFilePeeker import RDOFilePeeker
RDOFilePeeker(runArgs, merRDOLog)

# Common athena flags
if hasattr(runArgs, "skipEvents"):
  athenaCommonFlags.SkipEvents.set_Value_and_Lock(runArgs.skipEvents)
if hasattr(runArgs, "maxEvents"):
  athenaCommonFlags.EvtMax.set_Value_and_Lock(runArgs.maxEvents)

# Digitization flags
from Digitization.DigitizationFlags import digitizationFlags
if hasattr(runArgs, "PileUpPresampling"):
  merRDOLog.info("Doing pile-up presampling")
  digitizationFlags.PileUpPresampling = runArgs.PileUpPresampling

#from AthenaCommon.GlobalFlags import globalflags
#if hasattr(runArgs,"geometryVersion"): globalflags.DetDescrVersion.set_Value_and_Lock( runArgs.geometryVersion )
#if hasattr(runArgs,"conditionsTag"): globalflags.ConditionsTag.set_Value_and_Lock( runArgs.conditionsTag )

## Pre-exec
if hasattr(runArgs,"preExec"):
    merRDOLog.info("transform pre-exec")
    for cmd in runArgs.preExec:
        merRDOLog.info(cmd)
        exec(cmd)

## Pre-include
if hasattr(runArgs,"preInclude"):
    for fragment in runArgs.preInclude:
        include(fragment)

## Configure specific sub-detectors
if hasattr(runArgs,"LucidOn") or hasattr(runArgs,"ALFAOn") or hasattr(runArgs,"ZDCOn") or hasattr(runArgs,"AFPOn") or hasattr(runArgs,"FwdRegionOn"):
    if not 'DetFlags' in dir():
        # If you configure one detflag, you're responsible for configuring them all!
        from AthenaCommon.DetFlags import DetFlags
        DetFlags.all_setOn()
        DetFlags.ALFA_setOff() #Default for now
        DetFlags.ZDC_setOff() #Default for now
        checkAFPOff = getattr(DetFlags, 'AFP_setOff', None)
        if checkAFPOff is not None:
            checkAFPOff() #Default for now
        checkFwdRegionOff = getattr(DetFlags, 'FwdRegion_setOff', None)
        if checkFwdRegionOff is not None:
            checkFwdRegionOff() #Default for now

    if hasattr(runArgs,"LucidOn"):
        if not runArgs.LucidOn:
            DetFlags.Lucid_setOff()
    if hasattr(runArgs,"ALFAOn"):
        if runArgs.ALFAOn:
            DetFlags.ALFA_setOn()
    if hasattr(runArgs,"ZDCOn"):
        if runArgs.ZDCOn:
            DetFlags.ZDC_setOn()
    if hasattr(runArgs, "AFPOn"):
        if runArgs.AFPOn:
            checkAFPOn = getattr(DetFlags, 'AFP_setOn', None)
            if checkAFPOn is not None:
                checkAFPOn()
            else:
                merRDOLog.warning('The AFP DetFlag is not supported in this release')
    if hasattr(runArgs, "FwdRegionOn"):
        if runArgs.FwdRegionOn:
            checkFwdRegionOn = getattr(DetFlags, 'FwdRegion_setOn', None)
            if checkFwdRegionOn is not None:
                checkFwdRegionOn()
            else:
                merRDOLog.warning('The FwdRegion DetFlag is not supported in this release')


#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
from AthenaCommon.AppMgr import ServiceMgr
from AthenaPoolCnvSvc.AthenaPoolCnvSvcConf import AthenaPoolCnvSvc
ServiceMgr += AthenaPoolCnvSvc()

# Still the right setting?
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes = [ "DEFAULT_BUFFERSIZE = '2048'" ]

import AthenaPoolCnvSvc.ReadAthenaPool

from CLIDComps.CLIDCompsConf import ClassIDSvc
ServiceMgr += ClassIDSvc()
include( "PartPropSvc/PartPropSvc.py" )

# load all possible converters for EventCheck
GeoModelSvc = Service( "GeoModelSvc" )
GeoModelSvc.IgnoreTagDifference=True

# set up all detector description stuff
include( "RecExCond/AllDet_detDescr.py" )
from AthenaCommon.DetFlags import DetFlags
DetFlags.Print()

# muon ID
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
if not hasattr(ServiceMgr, "MuonIdHelperSvc"):
    from AthenaCommon.CfgGetter import getService
    ServiceMgr += getService("MuonIdHelperSvc")

# PixelLorentzAngleSvc and SCTLorentzAngleSvc
from InDetRecExample.InDetJobProperties import InDetFlags
include("InDetRecExample/InDetRecConditionsAccess.py")
include("InDetRecExample/InDetRecCabling.py")


#--------------------------------------------------------------
# Setup Input
#--------------------------------------------------------------
ServiceMgr.EventSelector.InputCollections = athenaCommonFlags.FilesInput()

# Check collection type
try:
  ServiceMgr.EventSelector.CollectionType = CollType
except:
  merRDOLog.info("Reading from file")

SkipEvents=0
ServiceMgr.EventSelector.SkipEvents = SkipEvents

#--------------------------------------------------------------
# Setup Output
#--------------------------------------------------------------
if hasattr(runArgs, "outputRDO_MRGFile"):
  outputFile = runArgs.outputRDO_MRGFile
else:
  outputFile = "DidNotSetOutputName.root"

if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
  from OverlayCommonAlgs.OverlayFlags import overlayFlags
  eventInfoKey = overlayFlags.bkgPrefix() + "EventInfo"
else:
  eventInfoKey = "EventInfo"

from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
StreamRDO = AthenaPoolOutputStream( "StreamRDO", outputFile, asAlg=True, eventInfoKey=eventInfoKey )
StreamRDO.TakeItemsFromInput=TRUE;
# The next line is an example on how to exclude clid's if they are causing a  problem
#StreamRDO.ExcludeList = ['6421#*']

# Look for lists of filter algorithms
try:
  StreamRDO.AcceptAlgs = AcceptList
except:
  merRDOLog.info("No accept algs indicated in AcceptList")
try:
  StreamRDO.RequireAlgs = RequireList
except:
  merRDOLog.info("No accept algs indicated in RequireList")
try:
  StreamRDO.VetoAlgs = VetoList
except:
  merRDOLog.info("No accept algs indicated in VetoList")

# Perfmon
from PerfMonComps.PerfMonFlags import jobproperties as pmon_properties
pmon_properties.PerfMonFlags.doMonitoring=True
pmon_properties.PerfMonFlags.doSemiDetailedMonitoring=True
pmon_properties.PerfMonFlags.OutputFile = "ntuple_RDOMerge"

MessageSvc = ServiceMgr.MessageSvc
MessageSvc.OutputLevel = INFO

StreamRDO.ExtendProvenanceRecord = False

ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" + outputFile + "'; COMPRESSION_ALGORITHM = '2'" ]
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" + outputFile + "'; COMPRESSION_LEVEL = '1'" ]

## Post-include
if hasattr(runArgs,"postInclude"):
    for fragment in runArgs.postInclude:
        include(fragment)

## Post-exec
if hasattr(runArgs,"postExec"):
    merRDOLog.info("transform post-exec")
    for cmd in runArgs.postExec:
        merRDOLog.info(cmd)
        exec(cmd)

#--------------------------------------------------------------
