#!/bin/bash

#### Simple script to test the reproducibility of the database read-in of the NSWCondAlg
primary_script="python -m MuonCondTest.NswCondAlgTest"

sides="MMA MMC STGCA STGCC"
conv="TDO PDO"
 ### Create first the reference file on the file
${primary_script} --isMC --LogName reference
 ### Then repeat a couple of times 
 for x in $(seq 0 1 3);do
    echo "Start reproducibility trial ${x}"
    ${primary_script} --isMC  --LogName test 2>&1  > /dev/null
    status_code=$?
    if [ ${status_code} -ne 0 ]; then
       exit 1
    fi
    for s in ${sides}; do
        for c in ${conv}; do
            echo "Check if ${s} ${c} differs in trial ${x}"
            diff reference_${c}_${s}.txt test_${c}_${s}.txt
            status_code=$?
            if [ ${status_code} -ne 0 ]; then
                echo "Difference comparison failed ${status_code}"
                exit 1
            fi
        done
    done
 done


#### now lets run on data. 
${primary_script} --LogName data_ref
status_code = $?
if [ ${status_code} -ne 0 ]; then
   exit 1
fi

${primary_script} --LogName data_test
status_code = $?
if [ ${status_code} -ne 0 ]; then
   exit 1
fi

diff data_ref_T0.txt data_test_T0.txt
status_code = $?
if [ ${status_code} -ne 0 ]; then
   echo "Difference comparison failed ${status_code}"
   exit 1
fi





