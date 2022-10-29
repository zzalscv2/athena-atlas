#See: https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/SoftwareTutorialxAODAnalysisInCMake for more details about anything here

testFile = 'myAOD.pool.root'

#override next line on command line with: --filesInput=XXX
jps.AthenaCommonFlags.FilesInput = [testFile] 

#Specify AccessMode (read mode) ... ClassAccess is good default for xAOD
jps.AthenaCommonFlags.AccessMode = "ClassAccess" 

jps.AthenaCommonFlags.HistOutputs = ["ANALYSIS:ZdcNtuple.outputs.root"]
svcMgr.THistSvc.MaxFileSize=-1 #speeds up jobs that output lots of histograms

# Create the algorithm's configuration.
from AnaAlgorithm.DualUseConfig import createAlgorithm
alg = createAlgorithm ( 'ZdcNtuple', 'AnalysisAlg' )

# later on we'll add some configuration options for our algorithm that go here
alg.zdcConfig = "LHCf2022"
alg.zdcOnly = True
alg.useGRL = False
alg.zdcCalib = False
alg.reprocZdc = False
alg.enableOutputTree = True
alg.enableOutputSamples = True
alg.enableTrigger = True

# Add our algorithm to the main alg sequence
athAlgSeq += alg

# limit the number of events (for testing purposes)
theApp.EvtMax = 1000

# optional include for reducing printout from athena
#include("AthAnalysisBaseComps/SuppressLogging.py")


