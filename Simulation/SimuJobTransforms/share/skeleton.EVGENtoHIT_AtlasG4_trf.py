## Include common skeleton
include("SimuJobTransforms/skeleton.EVGENtoHIT.py")

if hasattr(runArgs, 'inputEVNTFile'):
    raise RuntimeError("inputEVNTFile argument is not valid for AtlasG4_trf. Please use inputEvgenFile instead.")

if hasattr(runArgs, 'useISF') and runArgs.useISF:
    raise RuntimeError("Unsupported configuration! If you want to run with useISF=True, please use Sim_tf.py!")

## Get the logger
from AthenaCommon.Logging import *
atlasG4log = logging.getLogger('AtlasG4')
atlasG4log.info('****************** STARTING ATLASG4 ******************')

## Simulation flags need to be imported first
from G4AtlasApps.SimFlags import SimFlags, simFlags #FIXME drop import of SimFlags rather than simFlags asap
simFlags.load_atlas_flags()


## Set simulation geometry tag
if hasattr(runArgs, 'geometryVersion'):
    simFlags.SimLayout.set_Value_and_Lock(runArgs.geometryVersion)
    atlasG4log.debug('SimLayout set to %s' % simFlags.SimLayout)
else:
    raise RuntimeError("No geometryVersion provided.")

## AthenaCommon flags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
# Jobs should stop if an include fails.
if hasattr(runArgs, "IgnoreConfigError"):
    athenaCommonFlags.AllowIgnoreConfigError = runArgs.IgnoreConfigError
else:
    athenaCommonFlags.AllowIgnoreConfigError = False

## Input Files
if hasattr(runArgs, "inputFile"):
    athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputFile )
from AthenaCommon.BeamFlags import jobproperties
if hasattr(runArgs, "inputEvgenFile"):
    globalflags.InputFormat.set_Value_and_Lock('pool')
    athenaCommonFlags.PoolEvgenInput.set_Value_and_Lock( runArgs.inputEvgenFile )
    # We don't expect both inputFile and inputEvgenFile to be specified
    athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputEvgenFile )
elif hasattr(runArgs, "inputEvgen_StoppedFile"):
    globalflags.InputFormat.set_Value_and_Lock('pool')
    athenaCommonFlags.PoolEvgenInput.set_Value_and_Lock( runArgs.inputEvgen_StoppedFile )
    # We don't expect both inputFile and inputEvgenFile to be specified
    athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputEvgen_StoppedFile )
elif jobproperties.Beam.beamType.get_Value() == 'cosmics':
    atlasG4log.debug('No inputEvgenFile provided. OK, as performing cosmics simulation.')
    athenaCommonFlags.PoolEvgenInput.set_Off()
else:
    atlasG4log.info('No inputEvgenFile provided. Assuming that you are running a generator on the fly.')
    athenaCommonFlags.PoolEvgenInput.set_Off()

## Handle cosmics configs
if jobproperties.Beam.beamType.get_Value() == 'cosmics':
    simFlags.load_cosmics_flags()
    if hasattr(runArgs, "inputEvgenFile"):
        if simFlags.CosmicFilterVolumeName.statusOn and simFlags.CosmicFilterVolumeName.get_Value() != "Muon":
            atlasG4log.warning("Filtering was already done. Using CosmicFilterVolumeName=Muon rather than "
                               "provided value (%s)" % str(runArgs.CosmicFilterVolumeName))
        simFlags.CosmicFilterVolumeName = "Muon"
    else:
        simFlags.CosmicFilterVolumeName = getattr(runArgs, "CosmicFilterVolume", "InnerDetector")
        simFlags.CosmicFilterVolumeName2 = getattr(runArgs, "CosmicFilterVolume2", "NONE")
        simFlags.CosmicPtSlice = getattr(runArgs, "CosmicPtSlice", "NONE")


## Output hits file config
if hasattr(runArgs, "outputHITSFile"):
    athenaCommonFlags.PoolHitsOutput.set_Value_and_Lock( runArgs.outputHITSFile )
elif hasattr(runArgs, "outputHitsFile"):
    athenaCommonFlags.PoolHitsOutput.set_Value_and_Lock( runArgs.outputHitsFile )
else:
    if hasattr(runArgs, "outputEvgen_StoppedFile"):
        simFlags.StoppedParticleFile.set_Value_and_Lock( runArgs.outputEvgen_StoppedFile )
    #raise RuntimeError("No outputHitsFile provided.")
    atlasG4log.info('No outputHitsFile provided. This simulation job will not write out any HITS file.')
    athenaCommonFlags.PoolHitsOutput = ""
    athenaCommonFlags.PoolHitsOutput.statusOn = False


## Write out runArgs configuration
atlasG4log.info( '**** Transformation run arguments' )
atlasG4log.info( str(runArgs) )


#==============================================================
# Job Configuration parameters:
#==============================================================
## Pre-exec
if hasattr(runArgs, "preExec"):
    atlasG4log.info("transform pre-exec")
    for cmd in runArgs.preExec:
        atlasG4log.info(cmd)
        exec(cmd)

## Pre-include
if hasattr(runArgs, "preInclude"):
    for fragment in runArgs.preInclude:
        include(fragment)

# Avoid command line preInclude for stopped particles
if hasattr(runArgs, "inputEvgen_StoppedFile"):
    include('SimulationJobOptions/preInclude.ReadStoppedParticles.py')

include( "AthenaCommon/MemTraceInclude.py" )


## Select detectors
if 'DetFlags' not in dir():
    ## If you configure one det flag, you're responsible for configuring them all!
    from AthenaCommon.DetFlags import DetFlags
    DetFlags.all_setOn()
DetFlags.LVL1_setOff() # LVL1 is not part of G4 sim
DetFlags.Truth_setOn()
DetFlags.Forward_setOff() # Forward dets are off by default
## TODO Move repeated syntax into a separate function
if hasattr(runArgs, "AFPOn"):
    if runArgs.AFPOn:
        checkAFP = getattr(DetFlags, 'AFP_setOn', None)
        if checkAFP is not None:
            checkAFP()
        else:
            atlasG4log.warning( 'AFP Simulation is not supported in this release' )
if hasattr(runArgs, "FwdRegionOn"):
    if runArgs.FwdRegionOn:
        checkFwdRegion = getattr(DetFlags, 'FwdRegion_setOn', None)
        if checkFwdRegion is not None:
            checkFwdRegion()
        else:
            atlasG4log.warning( 'FwdRegion Simulation is not supported in this release' )
if hasattr(runArgs, "LucidOn"):
    if runArgs.LucidOn:
        DetFlags.Lucid_setOn()
if hasattr(runArgs, "ALFAOn"):
    if runArgs.ALFAOn:
        DetFlags.ALFA_setOn()
if hasattr(runArgs, "ZDCOn"):
    if runArgs.ZDCOn:
        DetFlags.ZDC_setOn()

if DetFlags.Forward_on():
    checkFwdRegion = getattr(DetFlags, 'FwdRegion_on', None)
    checkAFP = getattr(DetFlags, 'AFP_on', None)

    if (checkFwdRegion is not None and checkFwdRegion()) or DetFlags.ZDC_on() or DetFlags.ALFA_on() or (checkAFP is not None and checkAFP()):
        ## Do not filter high eta particles
        if simFlags.EventFilter.statusOn:
            simFlags.EventFilter.get_Value()['EtaPhiFilters'] = False
        ## ForwardTransport is applied to particles hitting BeamPipe::SectionF46
        DetFlags.bpipe_setOn()

    if checkFwdRegion is not None and checkFwdRegion():
        # Do full simulation rather than beam transport
        simFlags.ForwardDetectors = 1
        atlasG4log.info( 'FwdRegion switched on, so will run Full Simulation of the Forward Region rather than Forward Transport.' )
    elif DetFlags.ZDC_on() or DetFlags.ALFA_on() or (checkAFP is not None and checkAFP()):
        atlasG4log.info( 'Temoporary Measure: Switching off Magnetic Field for Forward Detector simulation.' )
        simFlags.MagneticField.set_Off()
        ## Use the ForwardTransport package to do the beam transport
        atlasG4log.info( 'FwdRegion switched off, so will run Full Simulation of the Forward Region rather than Forward Transport.' )
        simFlags.ForwardDetectors = 2

## Set the PhysicsList
if hasattr(runArgs, 'physicsList'):
    simFlags.PhysicsList = runArgs.physicsList


## Random seed
if hasattr(runArgs, "randomSeed"):
    simFlags.RandomSeedOffset = int(runArgs.randomSeed)
else:
    atlasG4log.warning('randomSeed not set')
## Don't use the SeedsG4 override
simFlags.SeedsG4.set_Off()


## Set the Run Number (if required)
if hasattr(runArgs,"DataRunNumber"):
    if runArgs.DataRunNumber>0:
        atlasG4log.info( 'Overriding run number to be: %s ', runArgs.DataRunNumber )
        simFlags.RunNumber=runArgs.DataRunNumber
elif hasattr(runArgs,'jobNumber'):
    if runArgs.jobNumber>=0:
        atlasG4log.info( 'Using job number '+str(runArgs.jobNumber)+' to derive run number.' )
        simFlags.RunNumber = simFlags.RunDict.GetRunNumber( runArgs.jobNumber )
        atlasG4log.info( 'Set run number based on dictionary to '+str(simFlags.RunNumber) )

## Handle cosmics track record
from AthenaCommon.BeamFlags import jobproperties
if jobproperties.Beam.beamType.get_Value() == 'cosmics':
    if hasattr(runArgs, "inputEvgenFile"):
        simFlags.ReadTR = athenaCommonFlags.PoolEvgenInput()[0]
    else:
        if hasattr(runArgs, "outputTrackRecordFile"):
            simFlags.WriteTR = runArgs.outputTrackRecordFile
        elif hasattr(runArgs, "outputEvgenFile"):
            simFlags.WriteTR = runArgs.outputEvgenFile
        include( 'CosmicGenerator/jobOptions_ConfigCosmicProd.py' )


## Add filters for non-cosmics simulation
## FIXME: This block should be moved out of the skeleton into job options.
if jobproperties.Beam.beamType.get_Value() != 'cosmics':
    if simFlags.CavernCuts:
        simFlags.EventFilter.set_Off()
        # Make a bigger world volume for cavern bg
        simFlags.WorldZRange.set_Value(26050)
    else:
        simFlags.EventFilter.set_On()


from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

## Set Overall per-Algorithm time-limit on the AlgSequence
topSeq.TimeOut = 43200 * Units.s

try:
    from RecAlgs.RecAlgsConf import TimingAlg
    topSeq+=TimingAlg("SimTimerBegin", TimingObjOutputName = "EVNTtoHITS_timings")
except:
    atlasG4log.warning('Could not add TimingAlg, no timing info will be written out.')

## Add G4 alg to alg sequence
from G4AtlasApps.PyG4Atlas import PyG4AtlasAlg
topSeq += PyG4AtlasAlg()

from PyJobTransforms.trfUtils import releaseIsOlderThan
if releaseIsOlderThan(17,6):
    ## Random number configuration
    from G4AtlasAlg.G4AtlasAlgConf import G4AtlasAlg
    g4AtlasAlg = G4AtlasAlg()
    g4AtlasAlg.RandomGenerator = "athena"
    g4AtlasAlg.AtRndmGenSvc = simFlags.RandomSvc.get_Value()

## Add AMITag MetaData to TagInfoMgr
if hasattr(runArgs, 'AMITag'):
    if runArgs.AMITag != "NONE":
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr.TagInfoMgr.ExtraTagValuePairs += ["AMITag", runArgs.AMITag]


## Set firstEvent for cosmics jobs
if jobproperties.Beam.beamType.get_Value() == 'cosmics':
    if hasattr(runArgs, "firstEvent"):
        #print runArgs.firstEvent
        svcMgr.EventSelector.FirstEvent = runArgs.firstEvent
    else:
        svcMgr.EventSelector.FirstEvent = 0


## Increase max RDO output file size to 10 GB
## NB. We use 10GB since Athena complains that 15GB files are not supported
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
svcMgr.AthenaPoolCnvSvc.MaxFileSizes = [ "10000000000" ]


## Post-include
if hasattr(runArgs, "postInclude"):
    for fragment in runArgs.postInclude:
        include(fragment)

# Avoid command line postInclude for stopped particles
if hasattr(runArgs, "outputEvgen_StoppedFile"):
    include('SimulationJobOptions/postInclude.StoppedParticleWrite.py')

## Post-exec
if hasattr(runArgs, "postExec"):
    atlasG4log.info("transform post-exec")
    for cmd in runArgs.postExec:
        atlasG4log.info(cmd)
        exec(cmd)


## Always enable the looper killer, unless it's been disabled
if not hasattr(runArgs, "enableLooperKiller") or runArgs.enableLooperKiller:
    def use_looperkiller():
        from G4AtlasApps import PyG4Atlas, AtlasG4Eng
        lkAction = PyG4Atlas.UserAction('G4UserActions', 'LooperKiller', ['BeginOfRun', 'EndOfRun', 'BeginOfEvent', 'EndOfEvent', 'Step'])
        AtlasG4Eng.G4Eng.menu_UserActions.add_UserAction(lkAction)
    simFlags.InitFunctions.add_function("postInit", use_looperkiller)
else:
    atlasG4log.warning("The looper killer will NOT be run in this job.")
