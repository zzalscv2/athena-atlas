#import AthenaCommon.AtlasUnixGeneratorJob
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
# Number of events to be processed (default is 10)
theApp.EvtMax = 3
#--------------------------------------------------------------
# Algorithms Private Options
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = INFO

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
from Pythia8_i.Pythia8_iConf import Pythia8_i


topSequence+=Pythia8_i()
topSequence.Pythia8_i.useRndmGenSvc = True
topSequence.Pythia8_i.UserProcess = "qqbar2emu"

#topSequence.Pythia8_i.Commands += ['Random:setSeed = on']
#topSequence.Pythia8_i.Commands += ['Random:seed = 123456789']

#topSequence.Pythia8_i.CollisionEnergy = 10000

from TruthExamples.TruthExamplesConf import DumpMC
topSequence += DumpMC()


