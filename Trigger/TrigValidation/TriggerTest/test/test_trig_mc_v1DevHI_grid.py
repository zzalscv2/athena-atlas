#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: grid
# art-include: master/Athena
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
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'pbpb'
ex.threads = 4
ex.concurrent_events = 4
ex.max_events = 500
precommand = ''.join([
  "setMenu='Dev_HI_run3_v1_TriggerValidation_prescale';",
  "doWriteBS=False;",
  "doWriteRDOTrigger=True;",
  "from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.IOVDb.GlobalTag='OFLCOND-MC16-SDR-RUN2-09';",
  "ConfigFlags.Trigger.enableL1CaloPhase1=False;"
])
ex.args = '-c "{:s}"'.format(precommand)

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
