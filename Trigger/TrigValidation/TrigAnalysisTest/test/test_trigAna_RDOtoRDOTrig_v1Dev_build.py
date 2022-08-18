#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of the RDOtoRDOTrigger transform with threads=1
# art-type: build
# art-include: master/Athena
# art-include: 22.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

preExec = ';'.join([
  'setMenu=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
  'from AthenaConfiguration.AllConfigFlags import ConfigFlags',
  'ConfigFlags.Trigger.AODEDMSet=\'AODFULL\'',
])

ex = ExecStep.ExecStep()
ex.type = 'Reco_tf'
ex.input = 'ttbar'
ex.threads = 1
ex.args = '--outputRDO_TRIGFile=RDO_TRIG.pool.root'
ex.args += ' --preExec="all:{:s};"'.format(preExec)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
