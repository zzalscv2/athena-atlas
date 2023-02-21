#!/bin/sh
#
# art-description: Run mc20e MC setup, with APF, without pileup.
# art-output: log.*
# art-type: grid
# art-athena-mt: 8
# art-include: master/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA --multithreaded --inputHITSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/user.ladamczy/user.ladamczy.mc15_13TeV.860102.SuperChicPy8_gg_jj_CEP_70_new.evgen.HITS.e8419.v1_EXT0/user.ladamczy.28711500.EXT0._000009.HITS.pool.root  --maxEvents=500 --conditionsTag="OFLCOND-MC16-SDR-RUN2-09" --geometryVersion="ATLAS-R2-2016-01-00-01" --preInclude 'Campaigns.MC20e' --maxEvents=300 --outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --steering 'doRDO_TRIG' --triggerConfig 'RDOtoRDOTrigger=MCRECO:DBF:TRIGGERDBMC:2233,87,314'

RES=$?
echo "art-result: $RES Reco"
