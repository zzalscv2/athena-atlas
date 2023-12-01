
pathena --trf \
"Run3DQTestingDriver.py \
--inputFiles=%IN \
Output.HISTFileName=%OUT.HIST.root \
DQ.Environment=AOD \
DQ.Steering.doHLTMon=True \
DQ.Steering.HLT.doGeneral=False \
DQ.Steering.HLT.doMET=False \
DQ.Steering.HLT.doTau=False \
DQ.Steering.HLT.doBjet=False \
DQ.Steering.HLT.doBphys=False \
DQ.Steering.HLT.doCalo=False \
DQ.Steering.HLT.doEgamma=True \
DQ.Steering.HLT.doJet=False \
DQ.Steering.HLT.doMinBias=False \
DQ.Steering.HLT.doMuon=False \
DQ.Steering.HLT.doTau=False \
DQ.Steering.HLT.doInDet=False \
DQ.Steering.Muon.doAlignMon=False \
DQ.Steering.Muon.doCombinedMon=False \
DQ.Steering.Muon.doPhysicsMon=False \
DQ.Steering.Muon.doRawMon=False \
DQ.Steering.Muon.doSegmentMon=False \
DQ.Steering.Muon.doTrackMon=False \
--dqOffByDefault \
--evtMax 100" \
--nFilesPerJob=1 \
--nJobs 10 \
--nCore 1 \
--mergeOutput \
--inDS $1 \
--outDS user.eegidiop.$1.a0
