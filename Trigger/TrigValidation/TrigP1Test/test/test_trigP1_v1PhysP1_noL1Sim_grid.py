#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the PhysicsP1_pp_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
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

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data'
ex.threads = 4
ex.concurrent_events = 4
precommand = ''.join([
  "setMenu='PhysicsP1_pp_run3_v1_HLTReprocessing_prescale';",
  "doL1Sim=False;",
  "forceEnableAllChains=True;",
  "disableChains=['HLT_cosmic_id_L1MU3V_EMPTY','HLT_cosmic_id_L1MU8VF_EMPTY']", # Temporary workaround for ATR-25459
])
ex.args = f'-c "{precommand}"'  
ex.args += ' --dump-config-reload'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
