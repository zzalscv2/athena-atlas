#!/bin/bash
#
# art-description: q221 reconstruction ART on MC in multithread mode
# art-type: grid
# art-athena-mt: 4
# art-include: main/Athena
# art-output: myAOD.pool.root
# art-output: NTUP_PHYSVAL.root
# art-output: rootcomp.root
# art-output: *.log
# art-output: *.ps
# art-output: dcube
# art-html: dcube

NEVENTS=200
REF_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/tauRec/reference/q221"

export ATHENA_CORE_NUMBER=4

# run the reconstruction
Reco_tf.py --maxEvents ${NEVENTS} --AMI q221 --multithreaded True
echo "art-result: $? Reconstrution"

# compare the AOD file
#art.py compare ref --entries ${NEVENTS} --mode semi-detailed --order-trees --branches-of-interest Tau* DiTau* InDetTrackParticles* CaloCalTopoClusters* AntiKt4LCTopoJets* --diff-root myAOD.pool.root ${REF_DIR}/myAOD.pool.root >> AOD_diff_root.log 2>&1
acmd.py diff-root myAOD.pool.root ${REF_DIR}/myAOD.pool.root --entries ${NEVENTS} --mode semi-detailed --order-trees --branches-of-interest Tau* DiTau* InDetTrackParticles* CaloCalTopoClusters* AntiKt4LCTopoJets* >> AOD_diff_root.log 2>&1
echo "art-result: $? diff-root"

# run the physics validation
Reco_tf.py --maxEvents ${NEVENTS} --validationFlags 'noExample,doTau' --inputAODFile myAOD.pool.root  --outputNTUP_PHYSVALFile NTUP_PHYSVAL.root
echo "art-result: $? PhysVal"

# compare the histograms
rootcomp.py NTUP_PHYSVAL.root ${REF_DIR}/NTUP_PHYSVAL.root >> rootcomp.log 2>&1
echo "art-result: $? rootcomp"

# run dcube
INPUT_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/tauRec/input"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    --plot --output dcube \
    --config ${INPUT_DIR}/config_mc.xml \
    --reference ${REF_DIR}/NTUP_PHYSVAL.root \
    NTUP_PHYSVAL.root
echo "art-result: $? dcube"
