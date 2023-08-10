#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: grid
# art-include: master/Athena
# art-architecture: '#&nvidia'
# If you create a grid version, check art-output in existing grid tests.
# art-athena-mt: 4
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

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

# Generate configuration run file
run = ExecStep.ExecStep('athena')
run.type = 'other'
run.input = 'Single_mu_Run4'
run.executable = 'runHLT_standalone_newJO.py'
run.args += ' --filesInput='+Input.get_input(run.input).paths[0]
run.args += ' Trigger.triggerMenuSetup="Dev_pp_run3_v1"'
run.args += ' Trigger.doRuntimeNaviVal=True'
run.args += ' ITk.doTruth=False'
run.args += ' Tracking.doTruth=False'
run.args += ' Trigger.enableL1CaloPhase1=False'
run.args += ' Trigger.InDetTracking.doGPU=True'
run.prmon = False


# The full test configuration
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
