#import AthenaCommon.AtlasUnixGeneratorJob
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
# Number of events to be processed (default is 10)
theApp.EvtMax = 10
#--------------------------------------------------------------
# Algorithms Private Options
#--------------------------------------------------------------

svcMgr.MessageSvc.OutputLevel = INFO

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
from Pythia8_i.Pythia8_iConf import Pythia8_i

topSequence+=Pythia8_i()
topSequence.Pythia8_i.CollisionEnergy = 7000

topSequence.Pythia8_i.LHEFile = "Powheg.ZMu.MC11.events"

topSequence.Pythia8_i.Commands += ['SpaceShower:pTmaxMatch = 2',
                                   'TimeShower:pTmaxMatch  = 1']
topSequence.Pythia8_i.UserHook = "ISRVetoedShower"

from TruthExamples.TruthExamplesConf import DumpMC
topSequence += DumpMC()


