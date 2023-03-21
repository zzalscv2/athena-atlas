#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

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
run.prmon = False


# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
