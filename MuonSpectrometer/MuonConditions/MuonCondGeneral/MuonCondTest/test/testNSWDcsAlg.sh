#!/bin/bash

#### Simple script to test the reproducibility of the database read-in of the NSWDcsAlg
primary_script="python -m MuonCondTest.NswDcsAlgTest -i /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/ESD/data23_cos.00448208.express_express.recon.ESD.x721/73events.data23_cos.00448208.express_express.recon.ESD.x721._lb0003._SFO-ALL._0001.1"

sides="A C"
techs="MMG MMD STG"
datas="HV"

### Create first the reference file on the file
${primary_script} --LogName reference

### Then repeat a couple of times 
for x in $(seq 0 1 3);do
    echo "Start reproducibility trial ${x}"
    ${primary_script}  --LogName test 2>&1  > /dev/null
    status_code=$?
    if [ ${status_code} -ne 0 ]; then
       exit 1
    fi
    for s in ${sides}; do
        for t in ${techs}; do
            for d in ${datas}; do
                echo "Check if ${s} ${t} ${d} differs in trial ${x}"
                diff reference_${d}_${t}_${s}.txt test_${d}_${t}_${s}.txt
                status_code=$?
                if [ ${status_code} -ne 0 ]; then
                    echo "Difference comparison failed ${status_code}"
                    exit 1
                fi
            done
        done
    done
done
