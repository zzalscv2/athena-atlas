#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'ttbar' # TODO restore to once HI MC has supercells 'pbpb'
ex.threads = 1
precommand = ''.join([
  "setMenu='Dev_HI_run3_v1_TriggerValidation_prescale';",
  "doWriteBS=False;",
  "doWriteRDOTrigger=True;",
  "from AthenaConfiguration.AllConfigFlags import ConfigFlags;",
  "ConfigFlags.Trigger.L1.doAlfaCtpin=True;",
  # TODO restore/fix once switching back to HI data "from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.IOVDb.GlobalTag='OFLCOND-MC16-SDR-RUN2-09'"
])
ex.args = '-c "{:s}"'.format(precommand)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
