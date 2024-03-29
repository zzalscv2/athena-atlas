#==============================================================
#
#
#		This job option runs the G4 simulation
#		of the ATLAS detector and the GeantFollower in ID (and MS)
#		It can be run using athena.py
#
#==============================================================


#--- Algorithm sequence ---------------------------------------
from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

# Conditions sequence for Athena MT
from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
if not hasattr(condSeq, "BeamSpotCondAlg"):
   from BeamSpotConditions.BeamSpotConditionsConf import BeamSpotCondAlg
   condSeq += BeamSpotCondAlg( "BeamSpotCondAlg" )


#topSeq.ContinueEventloopOnFPE = True
from RecExConfig.RecFlags import rec as rec
rec.doFloatingPointException.set_Value_and_Lock(True)
rec.doNameAuditor = True

#--- Output threshold (DEBUG, INFO, WARNING, ERROR, FATAL) ----
#from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.MessageSvc.OutputLevel  = INFO
ServiceMgr.MessageSvc.defaultLimit = 20000

#--- Detector flags -------------------------------------------
from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.GlobalFlags import globalflags

# Code crashes on NystromRK4 stepper NOW take default
#from G4AtlasApps.SimFlags import simFlags
#simFlags.G4Stepper = 'NystromRK4'


#--- Include JobSpecs.py --------------------------------------
#include ('JobSpecs.py')

### pass arguments with athena -c "..." ...jobOptions.py:
if 'myMomentum' not in dir() :
    myMomentum = 25000
#    myMomentum = 50000
#    myMomentum = 100000
#    myMomentum = 500000
#    myMomentum = 10000
#    myMomentum = 5000

if 'myRandomOffset' not in dir() :
    myRandomOffset = 0

if 'myRandomSeed1' not in dir() :
    myRandomSeed1 = 12398190

if 'myRandomSeed2' not in dir() :
    myRandomSeed2 = 820189

if 'myMaxEvent' not in dir() :
    myMaxEvent = 500
    myMaxEvent = 5000
#    myMaxEvent = 1000
    myMaxEvent = 100

if 'myPt' not in dir() :
    myPt = 'p'  # values are 'p' or 'pt'

if 'myGeo' not in dir() :
# release 20
    myGeo = 'ATLAS-R2-2015-03-01-00'
#   this is release 19 like
#    myGeo = 'ATLAS-GEO-20-00-01'
#   this is release 17 from Jochen
#    myGeo = 'ATLAS-GEO-18-01-03'
#    myGeo = 'ATLAS-GEO-18-01-00'
#    myGEO = 'ATLAS-R1-2012-03-02-00'

# Set everything to ATLAS
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()

# the global flags

# select latest Muon lay out on top of GEO-20
from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
#GeoModelSvc.MuonVersionOverride="MuonSpectrometer-R.07.00"
#GeoModelSvc.MuonVersionOverride="MuonSpectrometer-R.06.04"


from IOVDbSvc.CondDB import conddb

if myGeo.split('-')[1] == 'GEO' :
    if myGeo.split('-')[2] > 16 :
        globalflags.ConditionsTag = 'OFLCOND-SDR-BS7T-05-02'
        globalflags.ConditionsTag = 'OFLCOND-MC12-SDR-06'
#        conddb.setGlobalTag("OFLCOND-MC12-SDR-06")
    else :
        globalflags.ConditionsTag = 'OFLCOND-SDR-BS7T-04-15'
    print globalflags.ConditionsTag
elif myGeo.split('-')[1] == 'IBL' :
    globalflags.ConditionsTag = 'OFLCOND-SDR-BS14T-IBL-03'
else :
    globalflags.ConditionsTag = 'OFLCOND-SDR-BS7T-05-02'
#    globalflags.ConditionsTag = 'OFLCOND-SIM-00-00-00'
print globalflags.ConditionsTag

#from AthenaCommon import AthenaCommonFlags
#from BFieldAth.BFieldAthConf import MagFieldAthenaSvc
#ServiceMgr += MagFieldAthenaSvc(SetupCOOL=True, NameOfTheSource='COOL',
#                                UseDCS=not AthenaCommonFlags.isOnline())

#from IOVDbSvc.CondDB import conddb
#conddb.addOverride('/GLOBAL/BField/Map','BFieldMap-FullAsym-09')


DetFlags.Truth_setOn()

# specify your "/tmp/<username>/" directory here (output files may be large!)
outPath = ""  #"/tmp/wlukas/"

#--- AthenaCommon flags ---------------------------------------
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.PoolEvgenInput.set_Off()   ### is this necessary?
###athenaCommonFlags.PoolHitsOutput = outPath + 'Hits.pool.root'
athenaCommonFlags.PoolHitsOutput.set_Off()
athenaCommonFlags.EvtMax =  myMaxEvent

#--- Simulation flags -----------------------------------------
from G4AtlasApps.SimFlags import simFlags
simFlags.load_atlas_flags() # Going to use an ATLAS layout
simFlags.SimLayout = myGeo
simFlags.EventFilter.set_Off()

include("GeneratorUtils/StdEvgenSetup.py")

myMinEta =  0.2
#myMinEta =  0.0
myMaxEta =  1.0

#myMinEta =  0.8
#myMaxEta =  1.8

#myMinEta =  1.8
myMaxEta =  2.8

#myMinEta =  0.2
#myMaxEta =  2.8


#myMinEta =  2.0
#myMaxEta =  2.8
#myMinEta =  0.8
#myMaxEta =  2.0
#myMinEta =  0.2
#myMaxEta =  0.8

myPDG = 13  # 998 = Charged Geantino 999 = neutral Geantino, 13 = Muon

myPDG = 998

#from AthenaCommon.CfgGetter import getAlgorithm
#topSeq += getAlgorithm("BeamEffectsAlg", tryDefaultConfigurable=True)

simFlags.RandomSeedOffset = myRandomOffset

# sept 2014 run ParticleGun
import ParticleGun as PG
pg = PG.ParticleGun(randomStream="SINGLE", randomSeed = simFlags.RandomSeedOffset.get_Value())
#pg.sampler.pid = PG.CyclicSeqSampler([-13,13])
pg.sampler.pid = myPDG
pg.sampler.mom = PG.EEtaMPhiSampler(energy=myMomentum, eta=[myMinEta,myMaxEta])
#pg.sampler.mom = PG.PtEtaMPhiSampler(pt=myMomentum, eta=[myMinEta,myMaxEta])
topSeq += pg

# suppress the enormous amount of MC output
# from TruthExamples.TruthExamplesConf import PrintMC
# PrintMC.VerboseOutput = False

# ToolSvc setup
from AthenaCommon.AppMgr import ToolSvc

# Tracking Geometry
# from AthenaCommon.CfgGetter import getService
# getService("AtlasTrackingGeometrySvc")

from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags
from TrkDetDescrSvc.AtlasTrackingGeometrySvc import AtlasTrackingGeometrySvc

#ToolSvc.AtlasGeometryBuilder.OutputLevel = VERBOSE
#ToolSvc.CaloTrackingGeometryBuilder.OutputLevel = DEBUG
#ToolSvc.MuonTrackingGeometryBuilder.OutputLevel = DEBUG
#ToolSvc.InDetTrackingGeometryBuilder.OutputLevel = DEBUG

#ServiceMgr.AtlasTrackingGeometrySvc.OutputLevel = VERBOSE

#DetFlags.simulate.Truth_setOn()   ### deprecated!


#--------------------------------------------------------------
# Assign the TrackingGeometry to the Algorithm
#--------------------------------------------------------------

# the layer material inspector
from TrkDetDescrTestTools.TrkDetDescrTestToolsConf import Trk__LayerMaterialInspector
LayerMaterialInspector = Trk__LayerMaterialInspector(name= 'LayerMaterialInspector')
LayerMaterialInspector.OutputLevel = INFO
ToolSvc += LayerMaterialInspector



# the tracking volume displayer
from TrkDetDescrTestTools.TrkDetDescrTestToolsConf import Trk__TrackingVolumeDisplayer
TrackingVolumeDisplayer = Trk__TrackingVolumeDisplayer(name= 'TrackingVolumeDisplayer')
TrackingVolumeDisplayer.TrackingVolumeOutputFile = 'TrackingVolumes-'+TrkDetFlags.MaterialMagicTag()+'.C'
TrackingVolumeDisplayer.LayerOutputFile          = 'Layers-'+TrkDetFlags.MaterialMagicTag()+'.C'
TrackingVolumeDisplayer.SurfaceOutputFile        = 'Surfaces-'+TrkDetFlags.MaterialMagicTag()+'.C'
ToolSvc += TrackingVolumeDisplayer


# PROPAGATOR DEFAULTS --------------------------------------------------------------------------------------

from TrkExTools.TrkExToolsConf import Trk__EnergyLossUpdator
AtlasEnergyLossUpdater = Trk__EnergyLossUpdator(name="AtlasEnergyLossUpdator")
ToolSvc  += AtlasEnergyLossUpdater
ToolSvc.AtlasEnergyLossUpdator.DetailedEloss = True
TestEnergyLossUpdater  += AtlasEnergyLossUpdater


TestPropagators  = []

from TrkExRungeKuttaPropagator.TrkExRungeKuttaPropagatorConf import Trk__RungeKuttaPropagator as Propagator
TestPropagator = Propagator(name = 'TestPropagator')
ToolSvc += TestPropagator

TestPropagators += [ TestPropagator ]

from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator as STEP_Propagator
TestSTEP_Propagator = STEP_Propagator(name = 'TestSTEP_Propagator')
ToolSvc += TestSTEP_Propagator
TestSTEP_Propagator.DetailedEloss = True

TestPropagators += [TestSTEP_Propagator]

# UPDATOR DEFAULTS -----------------------------------------------------------------------------------------

TestUpdators    = []

from TrkExTools.TrkExToolsConf import Trk__MaterialEffectsUpdator as MaterialEffectsUpdator
TestMaterialEffectsUpdator = MaterialEffectsUpdator(name = 'TestMaterialEffectsUpdator')
ToolSvc += TestMaterialEffectsUpdator
if myPDG == 998 :
 TestMaterialEffectsUpdator.EnergyLoss           = False
 TestMaterialEffectsUpdator.MultipleScattering   = False

TestUpdators    += [ TestMaterialEffectsUpdator ]

TestMaterialEffectsUpdatorLandau = MaterialEffectsUpdator(name = 'TestMaterialEffectsUpdatorLandau')
TestMaterialEffectsUpdatorLandau.LandauMode           = True
ToolSvc += TestMaterialEffectsUpdatorLandau
if myPDG == 998 :
 TestMaterialEffectsUpdatorLandau.EnergyLoss           = False
 TestMaterialEffectsUpdatorLandau.MultipleScattering   = False

##TestUpdators    += [ TestMaterialEffectsUpdatorLandau ]

# the UNIQUE NAVIGATOR ( === UNIQUE GEOMETRY) --------------------------------------------------------------
from TrkExTools.TrkExToolsConf import Trk__Navigator
TestNavigator = Trk__Navigator(name = 'TestNavigator')
TestNavigator.TrackingGeometrySvc = "Trk::TrackingGeometrySvc/AtlasTrackingGeometrySvc"
ToolSvc += TestNavigator

# CONFIGURE PROPAGATORS/UPDATORS ACCORDING TO GEOMETRY SIGNATURE

TestSubPropagators = []
TestSubUpdators = []

# -------------------- set it depending on the geometry ----------------------------------------------------
# default for ID is (Rk,Mat)
TestSubPropagators += [ TestPropagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]

# default for Calo is (Rk,MatLandau)
TestSubPropagators += [ TestPropagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]

TestSubPropagators += [ TestPropagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]

# default for MS is (STEP,Mat)
TestSubPropagators += [ TestSTEP_Propagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]

TestSubPropagators += [ TestSTEP_Propagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]

TestSubPropagators += [ TestPropagator.name() ]
TestSubUpdators    += [ TestMaterialEffectsUpdator.name() ]
# ----------------------------------------------------------------------------------------------------------

# call the base class constructor
from TrkExTools.TrkExToolsConf import Trk__Extrapolator
TestExtrapolator = Trk__Extrapolator('TestExtrapolator',\
                           Navigator = TestNavigator,\
                           MaterialEffectsUpdators = TestUpdators,\
                           Propagators = TestPropagators,\
                           EnergyLossUpdater = TestEnergyLossUpdater,\
                           STEP_Propagator = TestSTEP_Propagator.name(),\
                           SubPropagators = TestSubPropagators,\
                           SubMEUpdators = TestSubUpdators)
ToolSvc += TestExtrapolator

from TrkG4UserActions.TrkG4UserActionsConf import Trk__GeantFollowerHelper
GeantFollowerHelper = Trk__GeantFollowerHelper(name="GeantFollowerHelper")
GeantFollowerHelper.Extrapolator             = TestExtrapolator
GeantFollowerHelper.ExtrapolateDirectly      = True
GeantFollowerHelper.ExtrapolateIncrementally = True
GeantFollowerHelper.OutputLevel = VERBOSE
ToolSvc += GeantFollowerHelper

SimFlags.OptionalUserActionList.addAction('G4UA::GeantFollowerTool',['Step','Event','Run'])

############### The output collection #######################

from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
## --- commit interval (test)
#ServiceMgr.AthenaPoolCnvSvc.OutputLevel = DEBUG

from AthenaCommon.AppMgr import ServiceMgr
if not hasattr(ServiceMgr, 'THistSvc'):
        from GaudiSvc.GaudiSvcConf import THistSvc
        ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += [ "val DATAFILE='GeantFollowing.root' TYPE='ROOT' OPT='RECREATE'" ]

##############################################################


include("G4AtlasApps/G4Atlas.flat.configuration.py")

from AthenaCommon.CfgGetter import getAlgorithm
topSeq += getAlgorithm("BeamEffectsAlg", tryDefaultConfigurable=True)

## Populate alg sequence
topSeq += getAlgorithm("G4AtlasAlg",tryDefaultConfigurable=True)

#AlgSequence("Streams").StreamHITS.ItemList                  = ['EventInfo#*']

ToolSvc.MuonTrackingGeometryBuilder.BlendInertMaterial = False
TestSTEP_Propagator.Straggling = False

#TestExtrapolator.OutputLevel = VERBOSE

if myPDG == 998 :
  TestSTEP_Propagator.MultipleScattering = False
  TestSTEP_Propagator.EnergyLoss = False

from AthenaCommon.ConfigurationShelve import saveToAscii
saveToAscii("config.txt")
#--- End jobOptions GeantFollowing_jobOptions.py file  ------------------------------
