#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
run = ExecStep.ExecStep()
run.type = 'athena'
run.args = '--CA'
run.input = 'Single_mu_Run4'
run.job_options = 'TriggerJobOpts/runHLT.py'
run.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
             'Trigger.doRuntimeNaviVal=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
             'Trigger.enableL1CaloPhase1=False']

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
