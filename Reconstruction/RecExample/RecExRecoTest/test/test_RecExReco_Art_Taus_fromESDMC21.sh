#!/bin/sh
#
# art-description: Athena runs tau reconstruction, using the new job configuration for Run 3, from an ESD file
# art-type: grid
# art-athena-mt: 8
# art-include: master/Athena
# art-output: *.log   

python $Athena_DIR/python/Reconstruction/tauRec/TauConfig.py | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

