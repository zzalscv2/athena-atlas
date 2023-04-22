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
run.input = 'ttbar'
run.executable = 'runHLT_standalone_newJO.py'
run.args += ' --filesInput='+Input.get_input(run.input).paths[0]
run.args += ' Trigger.triggerMenuSetup="Dev_pp_run3_v1"'
run.args += ' Trigger.doRuntimeNaviVal=True'
run.args += ' Output.doWriteRDO=False' #TODO enable once fixes issue with missing containers
run.prmon = False


# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_build.new'

# Change RegTest pattern
#regtest = test.get_step('RegTest')
#regtest.regex = r'TrigSignatureMoniMT\s*INFO\sHLT_.*|TrigSignatureMoniMT\s*INFO\s-- #[0-9]+ (Events|Features).*'

import sys
sys.exit(test.run())
