#!/bin/sh

python -c 'from MuonConfig.MuonRdoDecodeConfig import muonRdoDecodeTestData; muonRdoDecodeTestData()'# generate pickle
status=$?
if [ ${status} -ne 0 ] 
then
    echo "ERROR in configuration generation stage, stopping"
    exit -1
else
    echo
    echo "JOs reading stage finished, launching CARunner from pickle file"
    echo 
    CARunner.py MuonRdoDecode.pkl --evtMax 20 
fi
