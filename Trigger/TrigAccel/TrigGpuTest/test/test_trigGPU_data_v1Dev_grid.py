#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger BS->RDO_TRIG athena test of the Dev_pp_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data'
ex.threads = 1
ex.concurrent_events = 1
ex.max_events = 2000
ex.args = '-c "setMenu=\'Dev_pp_run3_v1_TriggerValidation_prescale\';doWriteBS=False;doL1Sim=True;doWriteRDOTrigger=True;doRuntimeNaviVal=True;from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.Trigger.InDetTracking.doGPU=True"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
