
## Algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

## Output printout level
ServiceMgr.MessageSvc.OutputLevel = INFO

from PerfMonComps.PerfMonFlags import jobproperties
jobproperties.PerfMonFlags.doMonitoring = True

## Detector flags
from AthenaCommon.DetFlags import DetFlags
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()
DetFlags.Truth_setOn()

## AthenaCommon flags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#athenaCommonFlags.PoolHitsOutput.set_Off()
athenaCommonFlags.PoolHitsOutput = "test.HITS.pool.root"

if 'EvtMax' not in dir():
    EvtMax=2000
athenaCommonFlags.EvtMax = EvtMax

## Set global conditions tag
from AthenaCommon.GlobalFlags import globalflags
globalflags.ConditionsTag = "OFLCOND-RUN12-SDR-19"
#globalflags.DetDescrVersion = 'ATLAS-R2-2015-03-01-00'

## Simulation flags
athenaCommonFlags.PoolEvgenInput.set_Off()
athenaCommonFlags.SkipEvents.set_Off()
from G4AtlasApps.SimFlags import simFlags
simFlags.load_atlas_flags()
simFlags.SimLayout.set_On()
simFlags.SimLayout='ATLAS-R2-2015-03-01-00_VALIDATION'

## Enable the EtaPhi, VertexSpread and VertexRange checks
simFlags.EventFilter.set_On()

## Use single particle generator
import AthenaCommon.AtlasUnixGeneratorJob
import ParticleGun as PG
pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = simFlags.RandomSeedOffset.get_Value())
pg.sampler.pid = PG.CyclicSeqSampler([-13,13])
pg.sampler.mom = PG.PtEtaMPhiSampler(pt=50000, eta=[-4,4])
job += pg

include("G4AtlasApps/fragment.SimCopyWeights.py")

## Release GeoModel memory once sim is configured
simFlags.ReleaseGeoModel = False


include("G4AtlasApps/G4Atlas.flat.configuration.py")

## Add the G4 sim to the alg sequence after the generator
from AthenaCommon.CfgGetter import getAlgorithm
job += getAlgorithm("G4AtlasAlg",tryDefaultConfigurable=True)


## User algorithms
from AthenaCommon.AppMgr import ServiceMgr
from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc("THistSvc")
#ServiceMgr.THistSvc.Output  = ["atlasTest DATAFILE='atlasTest.muons.histo.root' OPT='RECREATE'"];

ServiceMgr.THistSvc.Output  = ["truth DATAFILE='truth.root' OPT='RECREATE'"];

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

import AthenaCommon.CfgMgr as CfgMgr
job += CfgMgr.G4TestAlg()
from AthenaCommon import CfgGetter
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("TruthTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("EvgenTruthTestTool", checkType=True)] 
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("CaloEntryLayerTestTool", checkType=True)] 
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("MuonEntryLayerTestTool", checkType=True)] 
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("MuonExitLayerTestTool", checkType=True)] 
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("PixelHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("SCT_HitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("TrtHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("EMBHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("EMECHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("FCALHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("HECHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("TileHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools["TileHitsTestTool"].TestMBTS=True
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("MDTHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("RPCHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("CSCHitsTestTool", checkType=True)]
job.G4TestAlg.SimTestTools += [CfgGetter.getPrivateTool("TGCHitsTestTool", checkType=True)]


# Control random number seeds so that daily runs are identical
from AtlasGeant4.AtlasGeant4Conf import G4AtlasAlg
g4AtlasAlg = G4AtlasAlg()
g4AtlasAlg.RandomGenerator = "athena"

## For saving seeds
#from G4AtlasApps import AtlasG4Eng
#pp = AtlasG4Eng.G4Eng.menu_G4RandomNrMenu()
#pp.set_SaveOn()

## Check of memory during the RTT tests
from G4AtlasApps.atlas_utilities import MemorySnooper
job += MemorySnooper()

print ("###MYOUTPUT###")
print (ServiceMgr)
print ("###MYOUTPUT###")
print (job)

from AthenaCommon.ConfigurationShelve import saveToAscii 
saveToAscii("config.txt") 

