#!/bin/bash
#
# art-description: ART for standalone tau reconstruction
# art-type: grid
# art-include: master/Athena
# art-output: myAOD.pool.root
# art-output: NTUP_PHYSVAL.root
# art-output: rootcomp.root
# art-output: *.log
# art-output: *.ps

NEVENTS=500
REF_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/tauRec/reference/q431"

# run the reconstruction
Reco_tf.py --maxEvents=${NEVENTS} --AMI=q431
echo "art-result: $? Reconstrution"

# compare the AOD file
art.py compare ref --entries 200 --mode=semi-detailed --order-trees --diff-root myAOD.pool.root ${REF_DIR}/myAOD.pool.root >> AOD_diff_root.log 2>&1
echo "art-result: $? diff-root"

# run the physics validation
Reco_tf.py --maxEvents=${NEVENTS} --validationFlags 'noExample,doTau' --inputAODFile=myAOD.pool.root  --outputNTUP_PHYSVALFile=NTUP_PHYSVAL.root
echo "art-result: $? PhysVal"

# compare the histograms
rootcomp.py NTUP_PHYSVAL.root ${REF_DIR}/NTUP_PHYSVAL.root >> rootcomp.log 2>&1
echo "art-result: $? rootcomp"
