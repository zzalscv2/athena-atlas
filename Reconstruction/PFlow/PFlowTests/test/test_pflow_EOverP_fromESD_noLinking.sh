#!/bin/sh
#
# art-description: Athena runs EOverP ESD pflow reconstruction, PFO linking off, thinning off.
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-athena-mt: 8
# art-output: AOD.pool.root
# art-output: log_FE_validation.txt

export ATHENA_CORE_NUMBER=8 # set number of cores used in multithread to 8.

python $Athena_DIR/python/eflowRec/PFRunESDtoAOD_mc20e_eOverP.py | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log
