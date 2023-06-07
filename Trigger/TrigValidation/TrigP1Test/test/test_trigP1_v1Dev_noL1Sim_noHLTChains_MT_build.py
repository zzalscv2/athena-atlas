#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Test running only L1 result decoding with forks=2, threads=2, concurrent_events=2
# art-type: build                                                                  
# art-include: master/Athena
# art-include: 23.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data'
ex.forks = 2
ex.threads = 2
ex.concurrent_events = 2
ex.args = '-c "setMenu=\'Dev_pp_run3_v1\';doL1Sim=False;doEmptyMenu=True;forceEnableAllChains=True"'
ex.args += ' --dump-config-reload'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Skip ZeroCounts check because empty menu has no counts
test.check_steps.remove(test.get_step("ZeroCounts"))

import sys
sys.exit(test.run())
