#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Cosmic_run3_v1 menu
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'mc_cosmics'
ex.threads = 1
precommand = ''.join([
  "setMenu='Cosmic_run3_v1';", 
  "doCosmics=True;",
  "doWriteBS=False;",
  "doWriteRDOTrigger=True;",
  "from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.Trigger.enableL1CaloPhase1=False;",
])
ex.args = '-c "{:s}"'.format(precommand)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
