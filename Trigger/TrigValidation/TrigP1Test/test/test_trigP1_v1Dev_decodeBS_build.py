#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# art-description: Runs athenaHLT writing BS output and then runs BS decoding
# art-type: build
# art-include: master/Athena
# art-include: 22.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

##################################################
# Helper functions to build the test steps
##################################################
from TrigP1Test.TrigP1TestSteps import filterBS, decodeBS

##################################################
# Test definition
##################################################

# Run standard athenaHLT BS->BS job
writeBS = ExecStep.ExecStep("WriteBS")
writeBS.type = 'athenaHLT'
writeBS.job_options = 'TriggerJobOpts/runHLT_standalone.py'
writeBS.input = 'data'
writeBS.max_events = 50
precommand = ''.join([
  "setMenu='Dev_pp_run3_v1_TriggerValidation_prescale';",
  "doL1Sim=True;",
  "rewriteLVL1=True;",
  "doRuntimeNaviVal=True;",
])
writeBS.args = '-c "{:s}"'.format(precommand)
writeBS.args += ' -o output'
writeBS.args += ' --dump-config-reload'

# Extract and decode physics_Main
filterMain = filterBS("Main")
decodeMain = decodeBS("Main")

# Extract and decode calibration_CostMonitoring
filterCost = filterBS("CostMonitoring")
decodeCost = decodeBS("CostMonitoring")
decodeCost.args += ' -c "ModuleID=1"'

# Test definition with updated CheckFile inputs
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [writeBS, filterMain, decodeMain, filterCost, decodeCost]
test.check_steps = CheckSteps.default_check_steps(test)
test.get_step('CheckFile').input_file = 'ESD.pool.root,ESD.Module1.pool.root'

# Overwrite default MessageCount settings
# We are trying to lower the limits step by step
# Ultimately there should be no per-event messages
msgcount = test.get_step("MessageCount")
msgcount.thresholds = {
  'WARNING': 500,  # Remaining warnings are mostly from ATLASRECTS-3866
  'INFO': 500,
  'other': 50
}
msgcount.required = True # make the test exit code depend on this step

import sys
sys.exit(test.run())
