#!/usr/bin/env python
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of the RDOtoRDOTrigger transform with threads=1
# art-type: build
# art-include: master/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

preExec = ';'.join([
  'setMenu=\'LS2_v1_TriggerValidation_mc_prescale\'',
  'from TriggerJobOpts.TriggerFlags import TriggerFlags',
  'TriggerFlags.AODEDMSet.set_Value_and_Lock(\\\"AODFULL\\\")',
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

# Add a step comparing counts in the log against reference
refcomp = CheckSteps.RegTestStep("CountRefComp")
refcomp.input_base_name = 'athena.merged'
refcomp.regex = r'TrigSignatureMoniMT\s*INFO\sHLT_.*|TrigSignatureMoniMT\s*INFO\s-- #[0-9]+ (Events|Features).*'
refcomp.reference = 'TrigAnalysisTest/ref_RDOtoRDOTrig_v1Dev_build.ref'
refcomp.required = True # Final exit code depends on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.LogMergeStep, refcomp)

import sys
sys.exit(test.run())
