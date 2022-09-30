# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# test the whole AFP calibration loop

# prerequisite - have AODs from AFP_Calibration stream
# cd /afs/cern.ch/work/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD
# asetup Athena,master,latest,here
# for inputfile in `ls -d -1 /afs/cern.ch/work/p/pbalek/public/data17_13TeV.00338480.calibration_AFP.daq.RAW/*`
# 	do python ~/AFP/reco_master_Athena/athena/ForwardDetectors/ForwardRec/python/AFPRecConfig.py --filesInput=${inputfile}
# 	mv AOD.pool.root `echo $inputfile | sed -e 's/daq.RAW/AOD/g' -e 's/data$/root/'`
# done

# move to temp folder
cd /tmp/${USER}
setupATLAS
asetup Athena,master,latest,here

# setup this branch, e.g.
cd /afs/cern.ch/work/p/pbalek/public/afp_calibration_athena/build
# cmake ../athena/Projects/WorkDir; make 
source x86_64-centos7-gcc11-opt/setup.sh
cd /tmp/${USER}


# step 1 - produce histograms
mkdir input1; cd input1
athena AFP_Calibration/AFP_PixelHistoFiller.py --filesInput="/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-1._0007.root"
# now we have output1/AFP_PixelHistoFiller.root
cd ..

mkdir input2; cd input2
athena AFP_Calibration/AFP_PixelHistoFiller.py --filesInput="/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-2._0002.root,/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-2._0005.root"
# now we have output2/AFP_PixelHistoFiller.root
cd ..

mkdir input3; cd input3
athena AFP_Calibration/AFP_PixelHistoFiller.py --filesInput="/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.calibration_AFP.AOD/data17_13TeV.00338480.calibration_AFP.AOD._lb0000._SFO-3._0003.root"
# now we have output3/AFP_PixelHistoFiller.root
cd ..


# step 2 - merge and identify dead/noisy pixels
hadd -f AFP_PixelHistoFiller.root input*/AFP_PixelHistoFiller.root
run_AFP_PixelIdentifier
# now we have AFP_PixelIdentifier.root - that is our output
