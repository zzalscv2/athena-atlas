#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

run = ExecStep.ExecStep('athena')
run.type = 'other'
run.executable = 'runHLT_standalone_newJO.py'
run.input = 'ttbar' # TODO restore to once HI MC has supercells 'pbpb'
run.args += ' --filesInput='+Input.get_input(run.input).paths[0]
run.args += ' Trigger.triggerMenuSetup="Dev_HI_run3_v1_TriggerValidation_prescale"'
run.args += ' Trigger.doRuntimeNaviVal=True'
run.args += ' Trigger.L1.doAlfaCtpin=True'
run.prmon = False


test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_v1DevHI_build.new'

import sys
sys.exit(test.run())
