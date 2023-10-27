if not 'vp1InputFiles' in dir(): vp1InputFiles = []
if not 'vp1CfgFiles' in dir(): vp1CfgFiles = []
if not 'vp1ID' in dir(): vp1ID=True
if not 'vp1Calo' in dir(): vp1Calo=True
if not 'vp1Muon' in dir(): vp1Muon=True
if not 'vp1LUCID' in dir(): vp1LUCID=False
if not 'vp1ALFA' in dir(): vp1ALFA=False
if not 'vp1AFP' in dir(): vp1AFP=False
if not 'vp1ForwardRegion' in dir(): vp1ForwardRegion=False
if not 'vp1ZDC' in dir(): vp1ZDC=False
if not 'vp1Extrapolator' in dir(): vp1Extrapolator=True
# if not 'vp1Fitter' in dir(): vp1Fitter=False
if not 'vp1Fatras' in dir(): vp1Fatras=False
if not 'vp1FatrasMuon' in dir(): vp1FatrasMuon=(vp1Fatras and vp1Muon)
if not 'vp1FatrasCalo' in dir(): vp1FatrasCalo=(vp1Fatras and vp1Calo)
if not 'vp1FatrasTruthKey' in dir(): vp1FatrasTruthKey=""
if not 'vp1Online' in dir(): vp1Online=False
if not 'vp1Mc' in dir(): vp1Mc=False
if not 'vp1Multinp' in dir(): vp1Multinp=False
if not 'vp1Multinpsrc' in dir(): vp1Multinpsrc=""
if not 'vp1Multinpcpy' in dir(): vp1Multinpcpy=""
if not 'vp1MultiAvailableSrcDirs' in dir(): vp1MultiAvailableSrcDirs = []
if not 'vp1Batch' in dir(): vp1Batch=False
if not 'vp1BatchAllEvents' in dir(): vp1BatchAllEvents=False
if not 'vp1BatchNEvents' in dir(): vp1BatchNEvents=0
if not 'vp1BatchOutFolder' in dir(): vp1BatchOutFolder=""
if not 'vp1BatchRandomConfiguration' in dir(): vp1BatchRandomConfiguration=False
if not 'vp1LarHvData' in dir(): vp1LarHvData=False
# if not 'vp1FullToroids' in dir(): vp1FullToroids=False
if not 'vp1CruiseTime' in dir(): vp1CruiseTime=0
if not 'vp1NoSortDBReplicas' in dir(): vp1NoSortDBReplicas=False
if not 'vp1FilterEvents' in dir(): vp1FilterEvents=""
if not 'vp1NoGui' in dir(): vp1NoGui=False
if not 'vp1SpacePoints' in dir(): vp1SpacePoints=False
if not 'vp1Cavern' in dir(): vp1Cavern=False
if not 'vp1NoAutoConf' in dir(): vp1NoAutoConf=False
if not 'vp1Trig' in dir(): vp1Trig=False
if not 'vp1MuonAGDDFiles' in dir(): vp1MuonAGDDFiles=[]
if not 'vp1MuonAGDD2GeoSwitches' in dir(): vp1MuonAGDD2GeoSwitches=[]
if not 'vp1NSWAGDDFiles' in dir(): vp1NSWAGDDFiles=[]
if not 'vp1MuonLayout' in dir(): vp1MuonLayout=""

def vp1CfgErr(s): printfunc ("VP1 CONFIGURATION ERROR: %s" % s)

if (vp1Fatras and not vp1ID):
    vp1CfgErr("Fatras can not be enabled without inner detector. Turning off Fatras.")
    vp1Fatras=False
if (vp1FatrasMuon and not vp1Fatras):
    vp1CfgErr("FatrasMuon can not be enabled without Fatras enabled. Turning off FatrasMuon.")
    vp1FatrasMuon=False
if (vp1FatrasMuon and not vp1Muon):
    vp1CfgErr("FatrasMuon can not be enabled without muon systems. Turning off FatrasMuon.")
    vp1FatrasMuon=False
if (vp1FatrasCalo and not vp1Fatras):
    vp1CfgErr("FatrasCalo can not be enabled without Fatras enabled. Turning off FatrasCalo.")
    vp1FatrasCalo=False
if (vp1FatrasCalo and not vp1Calo):
    vp1CfgErr("FatrasCalo can not be enabled without calo systems. Turning off FatrasCalo.")
    vp1FatrasCalo=False
if ( vp1FatrasTruthKey != "" and not vp1Fatras ):
    vp1CfgErr("FatrasTruthKey set but Fatras not enabled. Unsetting FatrasTruthKey.")
    vp1FatrasTruthKey=""

printfunc ("*** VP1 NOTE *** setting COIN_GLXGLUE env vars to make screenshots working remotely...")
printfunc ("*** VP1 NOTE *** COIN_GLXGLUE_NO_GLX13_PBUFFERS=1 - " + "COIN_GLXGLUE_NO_PBUFFERS=1")
os.putenv("COIN_GLXGLUE_NO_GLX13_PBUFFERS","1")
os.putenv("COIN_GLXGLUE_NO_PBUFFERS","1")



#Set VP1ATLREL integer so we can do e.g. "if VP1ATLREL>=160002:"
include('VP1Algs/vp1FragmentConstructVP1ATLREL.py')

#Disable FPE checks :-(
from RecExConfig.RecFlags import rec
rec.doFloatingPointException.set_Value_and_Lock(False)

if (vp1InputFiles == []):
    # Create an event selector:
    import AthenaCommon.AtlasUnixGeneratorJob
    # Configure GlobalFlags
    from AthenaCommon.GlobalFlags import globalflags
    globalflags.DetGeo.set_Value_and_Lock('atlas')
    globalflags.DataSource.set_Value_and_Lock('geant4')

    # Set geometry version
    if (not "DetDescrVersion" in dir()):
        #DetDescrVersion = "ATLAS-R2-2016-01-00-01" # default Run 2 geometry
        DetDescrVersion = "ATLAS-R3S-2021-03-02-00" # default Run 3 geometry, with symmetric NSW

    globalflags.DetDescrVersion = DetDescrVersion
    
    # Set conditions tag
    if not 'vp1GlobCond' in dir():
        vp1GlobCond="OFLCOND-SDR-BS7T-05-14"
    from IOVDbSvc.CondDB import conddb
    conddb.setGlobalTag(vp1GlobCond)

    ### NEW FOR REL. >= 22

    # Conditions Service for reading conditions data in serial and MT Athena
    from IOVSvc.IOVSvcConf import CondSvc
    svcMgr += CondSvc()

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")

    # Conditions data access infrastructure for serial and MT Athena
    from IOVSvc.IOVSvcConf import CondInputLoader
    condSeq += CondInputLoader( "CondInputLoader")

    import StoreGate.StoreGateConf as StoreGateConf
    svcMgr += StoreGateConf.StoreGateSvc("ConditionStore")

else:
    if(vp1NoAutoConf):
        # AutoConfiguration disabled
        import AthenaCommon.AtlasUnixStandardJob

        if(vp1Online):
            from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
            athenaCommonFlags.isOnline = True

        from AthenaCommon.GlobalFlags import globalflags
        globalflags.DetGeo.set_Value_and_Lock('atlas')
        if(vp1Mc):
            globalflags.DataSource.set_Value_and_Lock('geant4')
        else:
            globalflags.DataSource.set_Value_and_Lock('data')
        globalflags.InputFormat.set_Value_and_Lock('pool')
        globalflags.Luminosity.set_Value_and_Lock('zero')
        from AthenaCommon.BeamFlags import jobproperties
        jobproperties.Beam.beamType="collisions"

        # Set geometry version
        if (not "DetDescrVersion" in dir()):
            #DetDescrVersion = "ATLAS-GEO-10-00-02" # very old tag
            #DetDescrVersion = "ATLAS-R2-2016-01-00-01" # default Run 2 geometry
            DetDescrVersion = "ATLAS-R3S-2021-03-01-00" # default Run 3 geometry, with symmetric NSW
        globalflags.DetDescrVersion = DetDescrVersion
            
        # Set conditions tag
        if not 'vp1GlobCond' in dir():
            if (vp1Mc):
                vp1GlobCond="OFLCOND-SIM-BS7T-02"
            else:
                vp1GlobCond="COMCOND-BLKPST-004-01"

        from IOVDbSvc.CondDB import conddb
        conddb.setGlobalTag(vp1GlobCond)


        ### NEW FOR REL. >= 22

        # Conditions Service for reading conditions data in serial and MT Athena
        from IOVSvc.IOVSvcConf import CondSvc
        svcMgr += CondSvc()

        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")

        # Conditions data access infrastructure for serial and MT Athena
        from IOVSvc.IOVSvcConf import CondInputLoader
        condSeq += CondInputLoader( "CondInputLoader")

        import StoreGate.StoreGateConf as StoreGateConf
        svcMgr += StoreGateConf.StoreGateSvc("ConditionStore")

    else:
        # AutoConfiguration enabled
        from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
        athenaCommonFlags.FilesInput=vp1InputFiles
        
        from RecExConfig.RecFlags import rec
        rec.AutoConfiguration=['everything']
        rec.doAOD.set_Value_and_Lock(False)
        rec.doESD.set_Value_and_Lock(False)

        # Override geometry tag from command line
        if("DetDescrVersion" in dir()):
            from AthenaCommon.GlobalFlags import globalflags
            globalflags.DetDescrVersion = DetDescrVersion
            #globalflags.DetDescrVersion.set_Value_and_Lock(DetDescrVersion)

        athenaCommonFlags.EvtMax=-1
        include('RecExCond/RecExCommon_flags.py')

        # Override global conditions tag from command line
        if('vp1GlobCond' in dir()):
            from IOVDbSvc.CondDB import conddb
            conddb.setGlobalTag(vp1GlobCond)


        ### NEW FOR REL. >= 22

        # Conditions Service for reading conditions data in serial and MT Athena
        from IOVSvc.IOVSvcConf import CondSvc
        svcMgr += CondSvc()

        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")

        # Conditions data access infrastructure for serial and MT Athena
        from IOVSvc.IOVSvcConf import CondInputLoader
        condSeq += CondInputLoader( "CondInputLoader")

        import StoreGate.StoreGateConf as StoreGateConf
        svcMgr += StoreGateConf.StoreGateSvc("ConditionStore")

if vp1FilterEvents:
    import VP1Algs.VP1EvtFilter
    vp1FilterEvents = VP1Algs.VP1EvtFilter.parseFilterString(vp1FilterEvents)
    VP1Algs.VP1EvtFilter.installEventFilter(vp1FilterEvents)

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()


#BATCH-MODE
# If "vp1Batch" is TRUE, then set the corresponding env var. 
# The GUI of VP1 will not be shown, but the config file will be taken
# and in the end a render of the 3D window will be saved as PNG file.
#
# Moreover, if "vp1BatchAllEvents" is TRUE, then all the events 
# in the provided data file will be processed with the same configuration
# file provided by the user.
# 
if vp1Batch:
    os.putenv("VP1_BATCHMODE","1")
if vp1BatchAllEvents:
    os.putenv("VP1_BATCHMODE_ALLEVENTS","1")
if vp1BatchNEvents > 0:
    os.putenv("VP1_BATCHMODE_NEVENTS", str(vp1BatchNEvents) )
if (vp1BatchOutFolder != ""):
    os.putenv("VP1_BATCHMODE_OUT_FOLDER", vp1BatchOutFolder)
if vp1BatchRandomConfiguration:
    os.putenv("VP1_BATCHMODE_RANDOMCONFIG","1")



#Detector setup:
from AthenaCommon.DetFlags import DetFlags
# INNER DETECTORS
if (vp1ID): DetFlags.ID_setOn()
else:       DetFlags.ID_setOff()
# CALORIMETERS
if (vp1Calo): DetFlags.Calo_setOn()
else:         DetFlags.Calo_setOff()
# MUON SPECTROMETER
if (vp1Muon):
    DetFlags.Muon_setOn()
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    if not MuonGeometryFlags.hasCSC(): DetFlags.CSC_setOff()
    if not MuonGeometryFlags.hasMM(): DetFlags.MM_setOff()
    if not MuonGeometryFlags.hasSTGC(): DetFlags.sTGC_setOff()
else:         DetFlags.Muon_setOff()
# LUCID
if (vp1LUCID): DetFlags.Lucid_setOn()
else:          DetFlags.Lucid_setOff()
# ZDC
if (vp1ZDC): DetFlags.ZDC_setOn()
else:        DetFlags.ZDC_setOff()
# AFP
if (vp1AFP):  DetFlags.AFP_setOn()
else:         DetFlags.AFP_setOff()
# ALFA
if (vp1ALFA): DetFlags.ALFA_setOn()
else:         DetFlags.ALFA_setOff()
# FORWARDREGION
if (vp1ForwardRegion): DetFlags.FwdRegion_setOn()
else:                  DetFlags.FwdRegion_setOff()
    
DetFlags.Print()

# --- GeoModel
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AthenaCommon.AppMgr import ToolSvc

if vp1Muon and vp1MuonLayout!="":
    printfunc ("*** VP1 NOTE *** You specified custom vp1MuonLayout, using %s as muon geometry"%vp1MuonLayout)
    from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
    GeoModelSvc = GeoModelSvc()
    GeoModelSvc.MuonVersionOverride=vp1MuonLayout

from AthenaCommon.AppMgr import ServiceMgr as svcMgr

if vp1Cavern:
  from CavernInfraGeoModel.CavernInfraGeoModelConf import CavernInfraDetectorTool
  svcMgr.GeoModelSvc.DetectorTools += [ CavernInfraDetectorTool() ]

# --- AGDD2Geo
# Switch it ON if
#  - Muon is ON
#  - Major geometry version is greater than 10
if (vp1Muon):
    
    # it fixes ATLASVPONE-641
    # Details: https://its.cern.ch/jira/browse/ATLASVPONE-641
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    if not hasattr(svcMgr, "MuonIdHelperSvc"):
        from MuonIdHelpers.MuonIdHelpersConfigLegacy import MuonIdHelperSvc
        svcMgr += MuonIdHelperSvc("MuonIdHelperSvc")
    
    from AtlasGeoModel import Agdd2Geo
    if len(vp1MuonAGDDFiles)>0:
        printfunc ("*** VP1 NOTE *** You specified custom vp1MuonAGDDFiles, configuring MuonAGDDTool to read MuonAGDD information from custom file(s) '%s' instead from built-in geometry"%(', '.join(vp1MuonAGDDFiles)))
        if hasattr(svcMgr,"AGDDtoGeoSvc"):
            for b in getattr(svcMgr,"AGDDtoGeoSvc").Builders:
                if b.name()=="MuonSpectrometer":
                    b.ReadAGDD=False
                    b.XMLFiles=vp1MuonAGDDFiles
                    if len(vp1MuonAGDD2GeoSwitches)>0:
                        printfunc ("*** VP1 NOTE *** You specified custom vp1MuonAGDD2GeoSwitches, configuring MuonAGDDTool to build volumes: '%s'"%(', '.join(vp1MuonAGDD2GeoSwitches)))
                        b.Volumes=vp1MuonAGDD2GeoSwitches
                    else:
                        # the default AGDD2GeoSwitches for Run2
                        b.Volumes=["ECT_Toroids",
                                   "BAR_Toroid",
                                   "Feet",
                                   "RailAssembly",
                                   "JFSH_Shield",
                                   "JDSH_Shield",
                                   "JTSH_Shield",
                                   "pp2",
                                   "MBAP_AccessPlatform",
                                   "MBWH_BigWheels",
                                   "SADL_CalorimeterSaddle",
                                   "TBWH_BigWheels",
                                   "TGC3_BigWheels",
                                   "TGC1_BigWheels",
                                   "MDTRail",
                                   "servicesAtZ0",
                                   "HFTruckRail",
                                   "RUN2_Services"]
    
    if len(vp1NSWAGDDFiles)>0:
        printfunc ("*** VP1 NOTE *** You specified custom vp1NSWAGDDFiles, configuring NSWAGDDTool to read NSWAGDD information from custom file(s) '%s' instead from built-in geometry"%(', '.join(vp1NSWAGDDFiles)))
        if hasattr(svcMgr,"AGDDtoGeoSvc"):
            for b in getattr(svcMgr,"AGDDtoGeoSvc").Builders:
                if b.name()=="NewSmallWheel":
                    b.ReadAGDD=False
                    b.XMLFiles=vp1NSWAGDDFiles

#MagneticField:
import MagFieldServices.SetupField

# Fatras
if (vp1Fatras):
    from FatrasExample.FatrasJobProperties import FatrasFlags
    FatrasFlags.SingleTrackSimulation.set_Value(vp1InputFiles==[])
    from FastSimulationConfig.FastSimulationFlags import jobproperties
    jobproperties.FastSimulation.doFatrasID.set_Value(True)
    jobproperties.FastSimulation.doFatrasMuon.set_Value(vp1FatrasMuon)
    FatrasFlags.FastCaloSim.set_Value(vp1FatrasCalo)


if (vp1InputFiles != []):
    import AthenaPoolCnvSvc.ReadAthenaPool

    if (vp1ID):
        if (vp1Fatras and vp1FatrasTruthKey!=""):
            from FatrasExample.FatrasKeys import FatrasKeyFlags
            FatrasKeyFlags.InputMcEventCollection.set_Value(vp1FatrasTruthKey)
    
    if (vp1Trig):
        from TriggerJobOpts.TriggerConfigGetter import TriggerConfigGetter
        TriggerConfigGetter()
    
    if (vp1Calo):
        from LArConditionsCommon import LArAlignable

        #
        # Following is needed to display Tile Pulse shapes on ESD files
        #
        from TileConditions.TileInfoConfigurator import TileInfoConfigurator
        tileInfoConfigurator = TileInfoConfigurator()

    if (vp1Muon):
        include( "AmdcAth/AmdcAth_jobOptions.py" )

    include( "PartPropSvc/PartPropSvc.py" )

    if(vp1NoAutoConf):
        svcMgr.EventSelector.InputCollections=vp1InputFiles
    else:
        svcMgr.EventSelector.InputCollections=athenaCommonFlags.FilesInput()
        
    if 'skipEvents' in dir():
        svcMgr.EventSelector.SkipEvents=skipEvents
else:
    vp1Extrapolator = False
    
#watch LAr HV:
if ( vp1LarHvData ):
    from time import time
    svcMgr.EventSelector.InitialTimeStamp  = int (time())
    svcMgr.EventSelector.TimeStampInterval = 1
    include('LArDetDescr/LArDetDescr_joboptions.py')
    from LArConditionsCommon import LArHVDB
#------------

if (vp1Fatras):
    include( "FatrasExample/Fatras_jobOptions.py" )

if (vp1Calo):
    from AthenaCommon.GlobalFlags import globalflags
    if globalflags.DataSource() == 'data' :
        include ("LArConditionsCommon/LArIdMap_comm_jobOptions.py")
    else:
        include ("LArConditionsCommon/LArConditionsCommon_MC_jobOptions.py")
        include ("LArConditionsCommon/LArIdMap_MC_jobOptions.py")

    from LArCabling.LArCablingAccess import LArOnOffIdMapping
    LArOnOffIdMapping()

#Extrapolator:
if vp1Extrapolator and (vp1ID or vp1Muon):
    # Force tool initializations needed by the extrapolator
    from VP1TrkAuxAlgs.VP1TrkAuxAlgsConf import VP1TrkInitializer
    VP1TrkInitializer = VP1TrkInitializer()
    VP1TrkInitializer.ForceExtrapolatorTools = True
    topSequence += VP1TrkInitializer

    if not vp1Fatras and vp1Mc:
        from TrkDetDescrSvc.AtlasTrackingGeometrySvc import AtlasTrackingGeometrySvc

    from TrkExTools.AtlasExtrapolator import AtlasExtrapolator
    VP1ExtraPolatorName='VP1Extrapolator';
    VP1Extrapolator = AtlasExtrapolator(name=VP1ExtraPolatorName)

    from TrkExEngine.AtlasExtrapolationEngine import AtlasExtrapolationEngine
    ExtrapolationEngine = AtlasExtrapolationEngine(name='Extrapolation', nameprefix='Atlas', ToolOutputLevel=INFO)
    ToolSvc += ExtrapolationEngine

    # Previous config from SetupFitters.py
    # from TrkExTools.TrkExToolsConf import Trk__Extrapolator as Extrapolator
    # VP1Extrapolator = Extrapolator(name='VP1Extrapolator',
    #                                Navigator = VP1Navigator,
    #                                MaterialEffectsUpdators = [ VP1MaterialUpdator ],
    #                                Propagators = [ VP1Propagator ] )

    ToolSvc += VP1Extrapolator
    #We should append to variable instead:
    os.putenv("VP1_JOBCFG_EXTRA_VP1_EXTRAPOLATORS","Trk::Extrapolator/"+VP1ExtraPolatorName)


#On a machine where the hostname does not indicate domain, pool will
#fail if trying to find nearest replica. In any case, dblookup.xml
#needs to be patched when running on real data:
include('VP1Algs/vp1FragmentFixDBReplica.py')

if vp1NoSortDBReplicas:
    PoolSvc = Service( "PoolSvc" )
    PoolSvc.SortReplicas = False


#Finally, the VP1 algorithm itself:
from VP1Algs.VP1AlgsConf import VP1Alg
vp1Alg = VP1Alg()
topSequence += vp1Alg

vp1Alg.NoGui=vp1NoGui

if vp1CruiseTime > 0:
    vp1Alg.InitialCruiseMode = "EVENT"
    vp1Alg.InitialCruiseModePeriod = vp1CruiseTime

vp1Alg.InitiallyLoadedVP1Files = vp1CfgFiles
if (vp1Multinp):
    vp1Alg.MultipleFilesON = True
    vp1Alg.MFSourceDir = vp1Multinpsrc
    vp1Alg.MFLocalCopyDir = vp1Multinpcpy
    vp1Alg.MFAvailableLocalInputDirectories = vp1MultiAvailableSrcDirs

if ( vp1LarHvData ):
    vp1Alg.ExtraInputs += [('LArHVIdMapping', 'ConditionStore+LArHVIdMap'),
                           ('CondAttrListCollection', 'ConditionStore+/LAR/DCS/HV/BARREl/I16'),
                           ('CondAttrListCollection', 'ConditionStore+/LAR/DCS/HV/BARREL/I8'),
                           ]

if (vp1Calo):
    vp1Alg.ExtraInputs += [('LArOnOffIdMapping', 'ConditionStore+LArOnOffIdMap')]


topSequence.TimeOut=0

