#!/bin/bash

##################################################################
# Charge Calibration using conventional method
# Usage : sh Calib.sh
# Detail : README or Note(link)
##################################################################

ITSNAME="[CALIB TOOL]"
START=$SECONDS
#echo "${ITSNAME} Setup the ATLAS software environment"
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'
setupATLAS

asetup master,latest,Athena
pushd ../../../../../../build
cmake ../athena/Projects/WorkDir
make
source ./x86_64-centos7-gcc11-opt/setup.sh
popd
DURATION=$((SECONDS-START))
echo "Compilation took ${DURATION} seconds"
echo ""
echo ""
echo "${ITSNAME} Create reference file from DB"
START=$SECONDS
AtlCoolConsole.py COOLOFL_PIXEL/CONDBR2 > PixCalib-DATA-RUN2-UPD4-21.log << EOF
usetag PixCalib-DATA-RUN2-UPD4-21
more /PIXEL/PixCalib
exit
EOF
#Following line to be activated after testing:
#MakeReferenceFile PixCalib-DATA-RUN2-UPD4-21
DURATION=$((SECONDS-START))
echo "Preparing DB reference took ${DURATION} seconds"
echo 
echo ""
echo ""

echo "${ITSNAME} Start calibration"
START=$SECONDS
root -b -q PixelCalib.C > output.dat
DURATION=$((SECONDS-START))
echo ""
echo ""
echo "${ITSNAME} Finish calibration"
echo "Running calibration took ${DURATION} seconds"

echo "${ITSNAME} Fix calibration result"
START=$SECONDS
python3 recover.py output.dat output_recover.dat
DURATION=$((SECONDS-START))
echo ""
echo ""
echo "${ITSNAME} Finish fixing the calibration result"
echo "Running recovery took ${DURATION} seconds"

echo "${ITSNAME} Finish Calibration!"
