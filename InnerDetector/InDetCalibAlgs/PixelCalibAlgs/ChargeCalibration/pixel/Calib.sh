#!/bin/bash

##################################################################
# Charge Calibration using conventional method
# Usage : sh Calib.sh
# Detail : README or Note(link)
##################################################################

ITSNAME="[CALIB TOOL]"

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
echo ""
echo ""

echo "${ITSNAME} Create reference file from DB"
AtlCoolConsole.py COOLOFL_PIXEL/CONDBR2 > PixCalib-DATA-RUN2-UPD4-21.log << EOF
usetag PixCalib-DATA-RUN2-UPD4-21
more /PIXEL/PixCalib
exit
EOF
echo ""
echo ""

echo "${ITSNAME} Start calibration"
root -b -q PixelCalib.C > output.dat
echo ""
echo ""
echo "${ITSNAME} Finish calibration"

echo "${ITSNAME} Fix calibration result"
python3 recover.py output.dat output_recover.dat
echo ""
echo ""
echo "${ITSNAME} Finish to fix calibration result"


echo "${ITSNAME} Finish Calibration!"
