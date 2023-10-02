#!/bin/sh
#
# art-description: Athena runs tau reconstruction, using the new job configuration and EMPFlow jets as tau seed
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python $Athena_DIR/python/tauRec/runTauOnly_EMPFlow.py | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log


