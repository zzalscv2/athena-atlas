#!/bin/sh
#
# art-description: Athena runs tracking reconstruction from a RAW data22 file
# art-type: grid
# art-athena-mt: 8
# art-include: master/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python $Athena_DIR/python/RecExRecoTest/TrackingReco_RAW_data22_13p6TeV.py | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

