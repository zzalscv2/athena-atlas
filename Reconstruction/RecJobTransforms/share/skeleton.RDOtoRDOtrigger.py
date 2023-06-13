# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Transform skeleton for RDO -> RDO_TRIG (running trigger and adding its output to the RDO file).
# This is only a wrapper interfacing transform arguments into the main job options file
# TriggerJobOpts/runHLT_standalone.py
#

from AthenaCommon.Include import include
from AthenaCommon.Logging import logging
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs

log = logging.getLogger('skeleton.RDOtoRDOtrigger')

ConfigFlags = flags  # backwards compatibility

##################################################
# Ensure AthenaMT is used as running Trigger requires the MT scheduler
##################################################
from AthenaCommon.ConcurrencyFlags import jobproperties as cfjp
if cfjp.ConcurrencyFlags.NumThreads() == 0 and flags.Concurrency.NumThreads == 0:
    raise RuntimeError('RDOtoRDOTrigger requires AthenaMT, but serial Athena was used. Please use threads=1 or more')

##################################################
# Options read by runHLT_standalone, can be overwritten in runArgs/preExec/preInclude
##################################################
doWriteRDOTrigger = True
doWriteBS         = False

##################################################
# Parse runArgs
##################################################
if 'runArgs' not in globals():
    raise RuntimeError('runArgs not defined')

ra = globals()['runArgs']
def getFromRunArgs(propName, failIfMissing=True):
    if not hasattr(ra, propName):
        if failIfMissing:
            raise RuntimeError(propName + ' not defined in runArgs')
        else:
            return None
    return getattr(ra, propName)

# Input/Output
athenaCommonFlags.FilesInput = getFromRunArgs('inputRDOFile')
flags.Input.Files = getFromRunArgs('inputRDOFile')
flags.Output.RDOFileName = getFromRunArgs('outputRDO_TRIGFile')

# Max/skip events
athenaCommonFlags.EvtMax = getFromRunArgs('maxEvents', False) or -1
athenaCommonFlags.SkipEvents = getFromRunArgs('skipEvents', False) or 0

#conditions/geometry setup for runHLT_standalone
try:
    flags.IOVDb.GlobalTag = getFromRunArgs("conditionsTag")
except RuntimeError:
    pass  # don't set the flag if not specified

try:
    flags.GeoModel.AtlasVersion = getFromRunArgs("geometryVersion")
except RuntimeError:
    pass  # don't set the flag if not specified

# PerfMon setup (ATR-25439)
setPerfmonFlagsFromRunArgs(flags, ra)

##################################################
# Parse preExec / preInclude
##################################################
preExec = getFromRunArgs('preExec', False)
if preExec:
    log.info('Executing transform preExec')
    for cmd in preExec:
        log.info(cmd)
        exec(cmd)

preInclude = getFromRunArgs('preInclude', False)
if preInclude:
    log.info('Executing transform preInclude')
    for fragment in preInclude:
        if '/' not in fragment:
            log.warning('Trying to use CA-based preInclude, trying to fallback to legacy equivalent')
            fragment = f"{fragment.replace('.', '/')}.py"
        include(fragment)

##################################################
# Include the main job options
##################################################
include("TriggerJobOpts/runHLT_standalone.py")

##################################################
# Include PerfMon configuration (ATR-25439)
##################################################
# Translate old concurrency flags to new for PerfMon
# - do that only here as runHLT_standalone must be independent of these flags
flagsForPerfMon = flags.clone()
flagsForPerfMon.Concurrency.NumProcs = cfjp.ConcurrencyFlags.NumProcs()
flagsForPerfMon.Concurrency.NumThreads = cfjp.ConcurrencyFlags.NumThreads()
flagsForPerfMon.Concurrency.NumConcurrentEvents = cfjp.ConcurrencyFlags.NumConcurrentEvents()
flagsForPerfMon.lock()
CAtoGlobalWrapper(PerfMonMTSvcCfg, flagsForPerfMon)

##################################################
# Parse postExec / postInclude
##################################################
postExec = getFromRunArgs('postExec', failIfMissing=False)
if postExec:
    log.info('Executing transform postExec')
    for cmd in postExec:
        log.info(cmd)
        exec(cmd)

postInclude = getFromRunArgs('postInclude', failIfMissing=False)
if postInclude:
    log.info('Executing transform postInclude')
    for fragment in postInclude:
        if '/' not in fragment:
            log.warning('Trying to use CA-based preInclude, trying to fallback to legacy equivalent')
            fragment = f"{fragment.replace('.', '/')}.py"
        include(fragment)
