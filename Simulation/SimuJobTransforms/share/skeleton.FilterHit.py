from __future__ import print_function

import traceback

from AthenaCommon.Logging import logging
filterHitLog = logging.getLogger('FilterHIT')

filterHitLog.info( '****************** STARTING HIT FILTERING *****************' )

filterHitLog.info( '**** Transformation run arguments' )
filterHitLog.info( str(runArgs) )

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#==============================================================
# Job definition parameters:
#==============================================================
from AthenaCommon.AthenaCommonFlags import jobproperties
#Jobs should stop if an include fails.
if hasattr(runArgs,"IgnoreConfigError"):
    jobproperties.AthenaCommonFlags.AllowIgnoreConfigError=runArgs.IgnoreConfigError
else:
    jobproperties.AthenaCommonFlags.AllowIgnoreConfigError=False

include("SimuJobTransforms/CommonSkeletonJobOptions.py")

# Get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp
# Number of events to be processed (default is 10)
theApp.EvtMax = jobproperties.AthenaCommonFlags.EvtMax.get_Value()

#--------------------------------------------------------------
# Peek at input to configure DetFlags
#--------------------------------------------------------------
if not hasattr(runArgs,"inputHITSFile"):
    if not hasattr(runArgs,"inputHitsFile"):
        raise RuntimeError("No inputHITSFile provided.")

from SimuJobTransforms.HitsFilePeeker import HitsFilePeeker
peekInfo = HitsFilePeeker(runArgs, filterHitLog)

#==============================================================
# Job Configuration parameters:
#==============================================================
## Pre-exec
if hasattr(runArgs,"preExec"):
    filterHitLog.info("transform pre-exec")
    for cmd in runArgs.preExec:
        filterHitLog.info(cmd)
        exec(cmd)

## Pre-include
if hasattr(runArgs,"preInclude"):
    for fragment in runArgs.preInclude:
        include(fragment)

#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
from AthenaCommon.AppMgr import ServiceMgr
from AthenaPoolCnvSvc.AthenaPoolCnvSvcConf import AthenaPoolCnvSvc
ServiceMgr += AthenaPoolCnvSvc()

from PyJobTransforms.trfUtils import releaseIsOlderThan
if releaseIsOlderThan(17,6):
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes = [ "DEFAULT_BUFFERSIZE = '2048'" ]

import AthenaPoolCnvSvc.ReadAthenaPool

from CLIDComps.CLIDCompsConf import ClassIDSvc
ServiceMgr += ClassIDSvc()
include( "PartPropSvc/PartPropSvc.py" )

from AthenaCommon.GlobalFlags import jobproperties
jobproperties.Global.DetGeo.set_Value_and_Lock( 'atlas' )
jobproperties.Global.Luminosity.set_Value_and_Lock( 'zero' )
jobproperties.Global.DataSource.set_Value_and_Lock( 'geant4' )

if 'DetFlags' in dir():
    filterHitLog.warning("DetFlags already defined! This means DetFlags should have been fully configured already..")
    #if you configure one detflag, you're responsible for configuring them all!
    DetFlags.Print()
else :
    from AthenaCommon.DetFlags import DetFlags
    #hacks to reproduce the sub-set of DetFlags left on by RecExCond/AllDet_detDescr.py
    DetFlags.simulate.all_setOff()
    DetFlags.Print()

#--------------------------------------------------------------
# Setup Input
#--------------------------------------------------------------
globalflags.InputFormat.set_Value_and_Lock('pool')
if hasattr(runArgs,"inputHITSFile"):
    athenaCommonFlags.PoolHitsInput.set_Value_and_Lock( runArgs.inputHITSFile )
else:
    athenaCommonFlags.PoolHitsInput.set_Value_and_Lock( runArgs.inputHitsFile )

ServiceMgr.EventSelector.InputCollections = athenaCommonFlags.PoolHitsInput()
theApp.EvtSel = "EventSelector"
# Number of input events to be skipped
if hasattr(runArgs,"skipEvents"):
    ServiceMgr.EventSelector.SkipEvents = athenaCommonFlags.SkipEvents()

#--------------------------------------------------------------
# Setup Output
#--------------------------------------------------------------
if hasattr(runArgs,"outputHITS_FILTFile"):
    athenaCommonFlags.PoolHitsOutput=runArgs.outputHITS_FILTFile
elif hasattr(runArgs,"outputHits_FILTFile"):
    athenaCommonFlags.PoolHitsOutput=runArgs.outputHits_FILTFile
elif hasattr(runArgs,"outputHitsFile"):
    athenaCommonFlags.PoolHitsOutput=runArgs.outputHitsFile
else:
    raise RuntimeError("No outputHitsFile provided.")

from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
try:
    Stream1 = AthenaPoolOutputStream( "StreamHITS", athenaCommonFlags.PoolHitsOutput(), noTag=not peekInfo["xAODEventInfoPresent"] )
except:
    Stream1 = AthenaPoolOutputStream( "StreamHITS", "DidNotSetOutputName.root", noTag=not peekInfo["xAODEventInfoPresent"] )
# The next line is an example on how to exclude clid's if they are causing a  problem
#Stream1.ExcludeList = ['6421#*']

# Set AutoFlush to 1 as per ATLASSIM-4274
# These outputs are meant to be read randomly
from AthenaPoolCnvSvc import PoolAttributeHelper as pah
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ pah.setTreeAutoFlush( Stream1.OutputFile , "CollectionTree",  1 ) ]
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ pah.setTreeAutoFlush( Stream1.OutputFile , "POOLContainer", 1 ) ]
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ pah.setTreeAutoFlush( Stream1.OutputFile , "POOLContainerForm", 1 ) ]

#--------------------------------------------------------------
# Specify collections for output HIT files, as not all are required.
#--------------------------------------------------------------

#Truth
Stream1.ItemList=["McEventCollection#TruthEvent", # mc truth (hepmc)
                  "JetCollection#*", # Truth jets reconstructed (optionally) during evgen
                  "TrackRecordCollection#MuonEntryLayer"] # others not used in pileup
#                  "TrackRecordCollection#MuonExitLayer", # not used in pileup
#                  "TrackRecordCollection#CaloEntryLayer"] # not used in pileup

# event info
if not peekInfo["xAODEventInfoPresent"]:
    Stream1.ItemList += ["EventInfo#*"]
else:
    Stream1.ItemList += ["xAOD::EventInfo#EventInfo",
                         "xAOD::EventAuxInfo#EventInfoAux.",
                         "xAOD::EventInfoContainer#*",
                         "xAOD::EventInfoAuxContainer#*"]

# Deal with "new" truth jet collections properly
from PyJobTransforms.trfUtils import releaseIsOlderThan
if releaseIsOlderThan(20,0):
    #Hack to maintain compatibility of G4AtlasApps trunk with
    #19.2.X.Y after EDM changes in release 20.0.0.
    Stream1.ItemList += ["xAOD::JetContainer_v1#*",
                         "xAOD::JetAuxContainer_v1#*"]
else:
    Stream1.ItemList +=  ["xAOD::JetContainer#AntiKt4TruthJets",
                          "xAOD::AuxContainerBase!#AntiKt4TruthJetsAux.-constituentLinks.-constituentWeights",
                          "xAOD::JetContainer#AntiKt6TruthJets",
                          "xAOD::AuxContainerBase!#AntiKt6TruthJetsAux.-constituentLinks.-constituentWeights"]

# pile-up truth particles
Stream1.ItemList += ["xAOD::TruthParticleContainer#TruthPileupParticles",
                     "xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."]

#BLM
#Stream1.ItemList += ["SiHitCollection#BLMHits"] # not used in digi
#BCM
Stream1.ItemList += ["SiHitCollection#BCMHits"]
#Pixels
Stream1.ItemList += ["SiHitCollection#PixelHits"]
#SCT
Stream1.ItemList += ["SiHitCollection#SCT_Hits"]
#TRT
Stream1.ItemList += ["TRTUncompressedHitCollection#TRTUncompressedHits"]
#LAr
Stream1.ItemList += ["LArHitContainer#LArHitEMB"]
Stream1.ItemList += ["LArHitContainer#LArHitEMEC"]
Stream1.ItemList += ["LArHitContainer#LArHitHEC"]
Stream1.ItemList += ["LArHitContainer#LArHitFCAL"]
#Tile
Stream1.ItemList += ["TileHitVector#TileHitVec"]

if hasattr(DetFlags.detdescr, 'HGTD_on') and DetFlags.detdescr.HGTD_on():
    Stream1.ItemList += ["LArHitContainer#LArHitHGTD"]
else:
    Stream1.ItemList += ["TileHitVector#MBTSHits"]
#Calo Calibration Hits - not required, so leave out to save space
#Stream1.ItemList += ["CaloCalibrationHitContainer#LArCalibrationHitActive",
#                     "CaloCalibrationHitContainer#LArCalibrationHitDeadMaterial",
#                     "CaloCalibrationHitContainer#LArCalibrationHitInactive",
#                     "CaloCalibrationHitContainer#TileCalibHitActiveCell",
#                     "CaloCalibrationHitContainer#TileCalibHitInactiveCell",
#                     "CaloCalibrationHitContainer#TileCalibHitDeadMaterial" ]
#CSC
if DetFlags.detdescr.CSC_on():
    Stream1.ItemList+=["CSCSimHitCollection#CSC_Hits"]
#MDT
Stream1.ItemList+=["MDTSimHitCollection#MDT_Hits"]
#RPC
Stream1.ItemList+=["RPCSimHitCollection#RPC_Hits"]
#TGC
Stream1.ItemList+=["TGCSimHitCollection#TGC_Hits"]
#STGC
if DetFlags.detdescr.sTGC_on():
    Stream1.ItemList+=["sTGCSimHitCollection#sTGC_Hits"]
#MM
if DetFlags.detdescr.MM_on():
    Stream1.ItemList+=["MMSimHitCollection#MM_Hits"]


#--------------------------------------------------------------
# the Tile, LAr and Calo detector description package
#--------------------------------------------------------------
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

include( "CaloIdCnv/CaloIdCnv_joboptions.py" )
include( "TileIdCnv/TileIdCnv_jobOptions.py" )
include( "LArDetDescr/LArDetDescr_joboptions.py" )
try:
    import MagFieldServices.SetupField
except:
    filterHitLog.warning("C++ magnetic field not available in this release.")
    include( "BFieldAth/BFieldAth_jobOptions.py" )
#--------------------------------------------------------------
# LarHit filter algorithm
#--------------------------------------------------------------
from AthenaCommon.CfgGetter import getAlgorithm
LArHitFilter = getAlgorithm("LArHitFilter")
topSequence += LArHitFilter
try:
    from SGComps import AddressRemappingSvc
    AddressRemappingSvc.addInputRename("LArHitContainer","LArHitEMB" ,"LArHitEMBOLD")
    AddressRemappingSvc.addInputRename("LArHitContainer","LArHitEMEC","LArHitEMECOLD")
    AddressRemappingSvc.addInputRename("LArHitContainer","LArHitFCAL","LArHitFCALOLD")
    AddressRemappingSvc.addInputRename("LArHitContainer","LArHitHEC" ,"LArHitHECOLD")

except:
    pass
#--------------------------------------------------------------
# McEventCollection filter algorithm
#--------------------------------------------------------------
if hasattr(runArgs,'TruthReductionScheme'):
    #--------------------------------------------------------------
    # Add aliases for input SimHit Collections (if possible)
    #--------------------------------------------------------------
    try:
        from SGComps import AddressRemappingSvc
        AddressRemappingSvc.addInputRename("McEventCollection","TruthEvent","TruthEventOLD")
        AddressRemappingSvc.addInputRename("SiHitCollection","BCMHits","BCMHitsOLD")
        AddressRemappingSvc.addInputRename("SiHitCollection","PixelHits","PixelHitsOLD")
        AddressRemappingSvc.addInputRename("SiHitCollection","SCT_Hits","SCT_HitsOLD")
        AddressRemappingSvc.addInputRename("TRTUncompressedHitCollection","TRTUncompressedHits","TRTUncompressedHitsOLD")
        AddressRemappingSvc.addInputRename("CSCSimHitCollection","CSC_Hits","CSC_HitsOLD")
        AddressRemappingSvc.addInputRename("MDTSimHitCollection","MDT_Hits","MDT_HitsOLD")
        AddressRemappingSvc.addInputRename("RPCSimHitCollection","RPC_Hits","RPC_HitsOLD")
        AddressRemappingSvc.addInputRename("TGCSimHitCollection","TGC_Hits","TGC_HitsOLD")
        AddressRemappingSvc.addInputRename("sTGCSimHitCollection","sTGC_Hits","sTGC_HitsOLD")
        AddressRemappingSvc.addInputRename("MMSimHitCollection","MM_Hits","MM_HitsOLD")
    except:
        pass

    from McEventCollectionFilter.McEventCollectionFilterConf import McEventCollectionFilter
    McEventCollectionFilter = McEventCollectionFilter("McEventCollectionFilter")
    if runArgs.TruthReductionScheme != 'SingleGenParticle':
        filterHitLog.warning( 'Unknown TruthReductionScheme (' + runArgs.TruthReductionScheme + '). Currently just a dummy value, but please check.' )
    ## here configure the level of Truth reduction required
    topSequence += McEventCollectionFilter
    from AthenaCommon.CfgGetter import getAlgorithm
    if peekInfo["AntiKt4TruthJetsPresent"]:
        topSequence += getAlgorithm("DecoratePileupAntiKt4TruthJets")
    if peekInfo["AntiKt6TruthJetsPresent"]:
        topSequence += getAlgorithm("DecoratePileupAntiKt6TruthJets")
    if peekInfo["PileUpTruthParticlesPresent"]:
        topSequence += getAlgorithm("DecorateTruthPileupParticles")

    if DetFlags.detdescr.TRT_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import TRT_HitsTruthRelink
            topSequence += TRT_HitsTruthRelink("TRT_HitsTruthRelink")
        except:
            filterHitLog.error('Trying to run on upgrade samples (no TRT) with an old tag of McEventCollectionFilter - job will fail.')

    if DetFlags.detdescr.BCM_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import SiliconHitsTruthRelink
            topSequence += SiliconHitsTruthRelink("BCM_HitsTruthRelink", InputHits="BCMHitsOLD", OutputHits="BCMHits")
        except:
            filterHitLog.error('Trying to run on upgrade samples (no BCM) with an old version of McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.pixel_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import SiliconHitsTruthRelink
            topSequence += SiliconHitsTruthRelink("PixelHitsTruthRelink", InputHits="PixelHitsOLD", OutputHits="PixelHits")
        except:
            filterHitLog.error('Trying to run on upgrade samples (no BCM) with an old version of McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.SCT_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import SiliconHitsTruthRelink
            topSequence += SiliconHitsTruthRelink("SCT_HitsTruthRelink", InputHits="SCT_HitsOLD", OutputHits="SCT_Hits")
        except:
            filterHitLog.error('Trying to run on upgrade samples (no BCM) with an old version of McEventCollectionFilter - job will fail.')

    if DetFlags.detdescr.CSC_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import CSC_HitsTruthRelink
            topSequence += CSC_HitsTruthRelink("CSC_HitsTruthRelink")
        except:
            filterHitLog.error('Trying to run on upgrade samples (no CSC) with an old tag of McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.MDT_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import MDT_HitsTruthRelink
            topSequence += MDT_HitsTruthRelink("MDT_HitsTruthRelink")
        except:
            filterHitLog.error('Failed to add MDT Hits to McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.RPC_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import RPC_HitsTruthRelink
            topSequence += RPC_HitsTruthRelink("RPC_HitsTruthRelink")
        except:
            filterHitLog.error('Failed to add RPC Hits to McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.TGC_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import TGC_HitsTruthRelink
            topSequence += TGC_HitsTruthRelink("TGC_HitsTruthRelink")
        except:
            filterHitLog.error('Failed to add TGC Hits to McEventCollectionFilter - job will fail.')
    ## For RUN3 geometries, turn on the NSW technologies.
    if DetFlags.detdescr.sTGC_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import sTGC_HitsTruthRelink
            topSequence += sTGC_HitsTruthRelink("sTGC_HitsTruthRelink")
        except:
            filterHitLog.error('Failed to add sTGC Hits to McEventCollectionFilter - job will fail.')
    if DetFlags.detdescr.MM_on():
        try:
            from McEventCollectionFilter.McEventCollectionFilterConf import MM_HitsTruthRelink
            topSequence += MM_HitsTruthRelink("MM_HitsTruthRelink")
        except:
            filterHitLog.error('Failed to add Micromega Hits to McEventCollectionFilter - job will fail.')


#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#-------------------------------------------------------------
ServiceMgr.MessageSvc.OutputLevel   = INFO
LArHitFilter.OutputLevel        = INFO

#--------------------------------------------------------------
# Ensure IOVDbSvc.GlobalTag is configured
# - protection against bad HITS file metadata
#--------------------------------------------------------------
if not hasattr(ServiceMgr,'IOVDbSvc'):
    from IOVDbSvc.IOVDbSvcConf import IOVDbSvc
    ServiceMgr += IOVDbSvc()
if not hasattr(ServiceMgr.IOVDbSvc, 'GlobalTag') or not ServiceMgr.IOVDbSvc.GlobalTag:
    from AthenaCommon.GlobalFlags import globalflags
    ServiceMgr.IOVDbSvc.GlobalTag = globalflags.ConditionsTag.get_Value()

#--------------------------------------------------------------
# Do the job!
#--------------------------------------------------------------

#Stream1.ExtendProvenanceRecord = False

#--------------------------------------------------------------

## Post-include
if hasattr(runArgs,"postInclude"):
    for fragment in runArgs.postInclude:
        include(fragment)

## Post-exec
if hasattr(runArgs,"postExec"):
    filterHitLog.info("transform post-exec")
    for cmd in runArgs.postExec:
        filterHitLog.info(cmd)
        exec(cmd)
#--------------------------------------------------------------
print(topSequence)
