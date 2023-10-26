#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu
# art-type: build                                                                  
# art-include: main/Athena
# art-include: 23.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data'
ex.max_events = 50
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"',
            'Trigger.doLVL1=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Add a step comparing counts against legacy reference (from test_trigP1_v1Dev_decodeBS_build)
chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_v1Dev_decodeBS_build.new'
refcomp = CheckSteps.ChainCompStep("CountRefComp")
refcomp.input_file = 'ref_v1Dev_decodeBS_build.new'
refcomp.reference_from_release = True # installed from TrigP1Test/share
refcomp.required = False # Final exit code doesn't depend on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
