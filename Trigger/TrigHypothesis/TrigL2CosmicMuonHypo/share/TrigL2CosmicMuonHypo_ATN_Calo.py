
# define slices to run...
doEmpty=True
doStandaloneEmpty=True
doCaloEmpty=True
doRPCEmpty=True
doTGCEmpty=True
doTileRODMu=True
doTileLookForMu=True
doEgamma=True
doAllTeEgamma=True

# other settings...
doM5L1Menu=True
#generateOnlineMenu=False
testCosmic=True
isOnline=False
doM4Data=True
doM3Data=True
isRealData=True
doM4L1Menu=True
doEnableCosmicMuonAccessInEF=True
doSerialization=True
T2CaloFakeLVL1InLVL2=True
doDBConfig=False
OutputLevel=INFO
HLTOutputLevel=DEBUG
forceLVL2Accept=False
FakeLVL1=False


# define input data
BSRDOInput=["/castor/cern.ch/user/a/acerri/ATNTests/daq.m5_combined.0028940.Default.L1TT-b00000110.LB0000.SFO-4._0209.data_5evts"]
include('TriggerRelease/runHLT_standalone.py')
