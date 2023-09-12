# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# -------------------------------------
# Common data and MC overlay skeleton
# -------------------------------------
# flake8 hack
overlayArgs = runArgs  # noqa F821

from AthenaCommon.Logging import logging
logOverlay = logging.getLogger('Overlay')
logOverlay.info('****************** STARTING OVERLAY *****************')

logOverlay.info('**** Transformation run arguments')
logOverlay.info(str(overlayArgs))

# PerfMon
from PerfMonComps.PerfMonFlags import jobproperties as pm
pm.PerfMonFlags.doFastMonMT = (overlayArgs.perfmon == 'fastmonmt')
pm.PerfMonFlags.doFullMonMT = (overlayArgs.perfmon == 'fullmonmt')
pm.PerfMonFlags.OutputJSON  = "perfmonmt_Overlay.json"
include( "PerfMonComps/PerfMonMTSvc_jobOptions.py" ) # noqa F821

# Pre-exec
if hasattr(overlayArgs, 'preExec') and overlayArgs.preExec != 'NONE':
    for cmd in overlayArgs.preExec:
        exec(cmd)

# Pre-include
if hasattr(overlayArgs, 'preInclude'):
    for fragment in overlayArgs.preInclude:
        if '/' not in fragment:
            logOverlay.warning('Trying to use CA-based preInclude, trying to fallback to legacy equivalent')
            fragment = f"{fragment.replace('.', '/')}.py"
        include(fragment)  # noqa F821

# ----------------------------
# Set flags from job options
# ----------------------------
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from Digitization.DigitizationFlags import digitizationFlags
from OverlayCommonAlgs.OverlayFlags import overlayFlags

hasRDO_BKGInput = hasattr(overlayArgs, 'inputRDO_BKGFile')
hasBS_SKIMInput = hasattr(overlayArgs, 'inputBS_SKIMFile')

if hasRDO_BKGInput and hasBS_SKIMInput:
    raise RuntimeError('Both RDO_BKG and BS_SKIM are defined')
if not hasRDO_BKGInput and not hasBS_SKIMInput:
    raise RuntimeError('Define one of RDO_BKG and BS_SKIM file types')

# Set overlay specifics
globalflags.isOverlay.set_Value_and_Lock(True)
# Force MT mode
overlayFlags.isOverlayMT.set_Value_and_Lock(True)
if hasRDO_BKGInput:
    logOverlay.info('Running MC+MC overlay')
    overlayFlags.isDataOverlay.set_Value_and_Lock(False)
    globalflags.DataSource.set_Value_and_Lock('geant4')
else:
    logOverlay.info('Running MC+data overlay')
    overlayFlags.isDataOverlay.set_Value_and_Lock(True)
    globalflags.DataSource.set_Value_and_Lock('data')
    globalflags.DetGeo = 'commis'  # TODO: why?
    globalflags.InputFormat.set_Value_and_Lock('bytestream')

    from AthenaCommon.BeamFlags import jobproperties
    jobproperties.Beam.beamType.set_Value_and_Lock('collisions')

# Common athena flags
if hasattr(overlayArgs, 'skipEvents'):
    athenaCommonFlags.SkipEvents.set_Value_and_Lock(overlayArgs.skipEvents)
if hasattr(overlayArgs, 'skipSecondaryEvents'):
    overlayFlags.SkipSecondaryEvents.set_Value_and_Lock(overlayArgs.skipSecondaryEvents)
if hasattr(overlayArgs, 'maxEvents'):
    athenaCommonFlags.EvtMax.set_Value_and_Lock(overlayArgs.maxEvents)

if hasattr(overlayArgs, 'inputHITSFile'):
    if not overlayFlags.isDataOverlay(): athenaCommonFlags.FilesInput.set_Value_and_Lock(overlayArgs.inputHITSFile)
    athenaCommonFlags.PoolHitsInput.set_Value_and_Lock(overlayArgs.inputHITSFile)
else:
    raise RuntimeError('No input HITS file defined')

if hasattr(overlayArgs, 'inputRDO_BKGFile'):
    athenaCommonFlags.PoolRDOInput.set_Value_and_Lock(overlayArgs.inputRDO_BKGFile)
elif hasattr(overlayArgs, 'inputBS_SKIMFile'):
    athenaCommonFlags.FilesInput.set_Value_and_Lock(overlayArgs.inputBS_SKIMFile)

if hasattr(overlayArgs, 'outputRDOFile'):
    athenaCommonFlags.PoolRDOOutput.set_Value_and_Lock(overlayArgs.outputRDOFile)
else:
    raise RuntimeError('No output RDO file defined')

if hasattr(overlayArgs, 'outputRDO_SGNLFile'):
    overlayFlags.signalOnlyOutput.set_Value_and_Lock(True)
    overlayFlags.PoolSignalRDOOutput.set_Value_and_Lock(
        overlayArgs.outputRDO_SGNLFile)
else:
    overlayFlags.signalOnlyOutput.set_Value_and_Lock(False)

if hasattr(overlayArgs, 'geometryVersion'):
    globalflags.DetDescrVersion.set_Value_and_Lock(overlayArgs.geometryVersion)
if hasattr(overlayArgs, 'conditionsTag'):
    globalflags.ConditionsTag.set_Value_and_Lock(overlayArgs.conditionsTag)
if hasattr(overlayArgs, 'detectors'):
    overlayDetectors = overlayArgs.detectors
else:
    overlayDetectors = None

# runNumber is MC channel number in reco
if hasattr(overlayArgs, 'runNumber'):
    # always set it in legacy config
    athenaCommonFlags.MCChannelNumber.set_Value(overlayArgs.runNumber)
    logOverlay.info('Got MC channel number %d from runNumber', athenaCommonFlags.MCChannelNumber())

# Digitization flags
if hasattr(overlayArgs, 'digiSeedOffset1'):
    digitizationFlags.rndmSeedOffset1 = int(overlayArgs.digiSeedOffset1)
else:
    logOverlay.warning('digiSeedOffset1 not set')
    digitizationFlags.rndmSeedOffset1 = 1

if hasattr(overlayArgs, 'digiSeedOffset2'):
    digitizationFlags.rndmSeedOffset2 = int(overlayArgs.digiSeedOffset2)
else:
    logOverlay.warning('digiSeedOffset2 not set')
    digitizationFlags.rndmSeedOffset2 = 2

if hasattr(overlayArgs, 'samplingFractionDbTag'):
    digitizationFlags.physicsList = overlayArgs.samplingFractionDbTag
if hasattr(overlayArgs, 'digiRndmSvc'):
    digitizationFlags.rndmSvc = overlayArgs.digiRndmSvc
if hasattr(overlayArgs, 'AddCaloDigi'):
    digitizationFlags.experimentalDigi += ['AddCaloDigi']

# LAr flags
if overlayFlags.isDataOverlay():
    from LArConditionsCommon.LArCondFlags import larCondFlags
    larCondFlags.LArCoolChannelSelection.set_Value_and_Lock('')
    larCondFlags.useShape.set_Value_and_Lock(True)
    larCondFlags.OFCShapeFolder.set_Value_and_Lock('')
    # TODO: put into global tag?
    fSampltag = 'LARElecCalibMCfSampl-G496-19213-'
    if hasattr(overlayArgs, 'fSampltag'):
        logOverlay.warning('fSampltag argument is deprecated, using %s', fSampltag)
    larCondFlags.LArfSamplTag.set_Value_and_Lock(fSampltag + digitizationFlags.physicsList.get_Value())

# TODO: investigate why this is even needed
from RecExConfig.RecFlags import rec
rec.projectName = 'IS_SIMULATION'


# ----------------------------
# Setup detector flags
# ----------------------------
logOverlay.info('================ DetFlags ================ ')
if 'DetFlags' in dir():
    logOverlay.warning(
        'DetFlags already defined! This means DetFlags should have been fully configured already..')
else:
    from OverlayConfiguration.OverlayHelpersLegacy import setupOverlayLegacyDetectorFlags
    DetFlags = setupOverlayLegacyDetectorFlags(overlayDetectors)

if hasattr(overlayArgs, 'triggerConfig') and overlayArgs.triggerConfig == 'NONE':
    DetFlags.LVL1_setOff()

DetFlags.digitize.LVL1_setOff()

from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
if not MuonGeometryFlags.hasCSC():
    DetFlags.CSC_setOff()
if not MuonGeometryFlags.hasSTGC():
    DetFlags.sTGC_setOff()
if not MuonGeometryFlags.hasMM():
    DetFlags.MM_setOff()

# TODO: need to do it better
# DetFlags.makeRIO.all_setOff() # needed for MT TRT conditions
DetFlags.pileup.all_setOff()
if not overlayFlags.isDataOverlay():
    DetFlags.readRDOBS.all_setOff()
    DetFlags.readRIOBS.all_setOff()
    DetFlags.readRIOPool.all_setOff()
DetFlags.readRDOPool.all_setOff()
DetFlags.simulate.all_setOff()
DetFlags.writeBS.all_setOff()
DetFlags.writeRIOPool.all_setOff()

DetFlags.Print()


# -------------------------------------------------------------
# Read Simulation MetaData (unless override flag set to True)
# ------------------------------------------------------------
if 'ALL' in digitizationFlags.overrideMetadata.get_Value():
    logOverlay.info('Skipping input file MetaData check.')
else:
    from EventOverlayJobTransforms.OverlayReadMetaData import readInputFileMetadata
    readInputFileMetadata()

DetFlags.Print()

# -------------------------
# Conditions
# -------------------------
from IOVDbSvc.CondDB import conddb
if hasattr(overlayArgs, 'conditionsTag') and overlayArgs.conditionsTag not in ['', 'NONE']:
    globalflags.ConditionsTag = overlayArgs.conditionsTag
    if len(globalflags.ConditionsTag()) != 0:
        conddb.setGlobalTag(globalflags.ConditionsTag())

# convert flags to ConfigFlags
from AthenaConfiguration.OldFlags2NewFlags import getNewConfigFlags
ConfigFlags = getNewConfigFlags()

# -------------------------
# Configuration
# -------------------------
logOverlay.info('================ Configure ================= ')

# Initialise overlay
include('EventOverlayJobTransforms/ConfiguredOverlayMT_jobOptions.py')  # noqa F821

# load the input properly
include('EventOverlayJobTransforms/OverlayInput_jobOptions.py')  # noqa F821

# Always schedule beam spot conditions for overlay
include('Digitization/BeamSpot.py')  # noqa F821

if DetFlags.overlay.Truth_on():
    include('EventOverlayJobTransforms/TruthOverlay_jobOptions.py')  # noqa F821

if DetFlags.overlay.BCM_on() or DetFlags.overlay.Lucid_on():
    include('EventOverlayJobTransforms/BeamOverlay_jobOptions.py')  # noqa F821

if DetFlags.overlay.pixel_on() or DetFlags.overlay.SCT_on() or DetFlags.overlay.TRT_on():
    include('EventOverlayJobTransforms/InnerDetectorOverlay_jobOptions.py')  # noqa F821

if DetFlags.overlay.LAr_on() or DetFlags.overlay.Tile_on():
    include('EventOverlayJobTransforms/CaloOverlay_jobOptions.py')  # noqa F821

ConfigFlags.Common.isOverlay = True
ConfigFlags.lock()
ConfigFlags.dump()
if (MuonGeometryFlags.hasCSC() and DetFlags.overlay.CSC_on()) or \
   (MuonGeometryFlags.hasSTGC() and DetFlags.overlay.sTGC_on()) or \
   (MuonGeometryFlags.hasMM() and DetFlags.overlay.MM_on()) or \
    DetFlags.overlay.MDT_on() or DetFlags.overlay.RPC_on() or DetFlags.overlay.TGC_on():
    from MuonConfig.MuonOverlayConfig import MuonOverlayCfg
    from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
    CAtoGlobalWrapper(MuonOverlayCfg, ConfigFlags)

if DetFlags.overlay.LVL1_on():
    include('EventOverlayJobTransforms/Level1Overlay_jobOptions.py')  # noqa F821

if overlayFlags.doTrackOverlay():
    include('EventOverlayJobTransforms/TrackOverlay_jobOptions.py') # noqa F821

# Run calculation of weight for the beam spot size reweighting
if digitizationFlags.doBeamSpotSizeReweighting() and digitizationFlags.OldBeamSpotZSize() > 0:
    include('Digitization/BeamSpotReweight.py')  # noqa F821

# save the overlay output
include('EventOverlayJobTransforms/OverlayOutput_jobOptions.py')  # noqa F821

# save the signal-only output
if overlayFlags.signalOnlyOutput():
    include('EventOverlayJobTransforms/SignalOutput_jobOptions.py')  # noqa F821


# -------------------------
# Random numbers
# -------------------------
# TODO: move somewhere more appropriate
from AthenaCommon.ConfigurableDb import getConfigurable
from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr += getConfigurable(digitizationFlags.rndmSvc.get_Value())()
digitizationFlags.rndmSeedList.addtoService()
digitizationFlags.rndmSeedList.printSeeds()


# -------------------------
# Logging
# -------------------------
from AthenaCommon.Constants import INFO
ServiceMgr.MessageSvc.OutputLevel = INFO
ServiceMgr.MessageSvc.Format = '% F%45W%S%5W%e%s%7W%R%T %0W%M'

#==========================================================
# Use ZLIB for compression of all temporary outputs
#==========================================================
if '_000' in overlayArgs.outputRDOFile or 'tmp.' in overlayArgs.outputRDOFile:
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" +  athenaCommonFlags.PoolRDOOutput()+ "'; COMPRESSION_ALGORITHM = '1'" ]
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" +  athenaCommonFlags.PoolRDOOutput()+ "'; COMPRESSION_LEVEL = '1'" ]

# Post-include
if hasattr(overlayArgs, 'postInclude'):
    for fragment in overlayArgs.postInclude:
        if '/' not in fragment:
            logOverlay.warning('Trying to use CA-based postInclude, trying to fallback to legacy equivalent')
            fragment = f"{fragment.replace('.', '/')}.py"
        include(fragment)  # noqa F821

# Post-exec
if hasattr(overlayArgs, 'postExec') and overlayArgs.postExec != 'NONE':
    for cmd in overlayArgs.postExec:
        exec(cmd)

# Patch /TagInfo metadata container
# TODO: move somewhere more appropriate
if not overlayFlags.isDataOverlay():
    ServiceMgr.TagInfoMgr.ExtraTagValuePairs.update(
        overlayFlags.extraTagInfoPairs.get_Value())

# Set AMITag in /TagInfo
from PyUtils import AMITagHelper
AMITagHelper.SetAMITag(runArgs=overlayArgs)

# ================================================================
logOverlay.info('\nOverlay: OutputStream = \n' + str(outStream))  # noqa F821
logOverlay.info('\nOverlay: ServiceMgr = \n' + str(ServiceMgr))
