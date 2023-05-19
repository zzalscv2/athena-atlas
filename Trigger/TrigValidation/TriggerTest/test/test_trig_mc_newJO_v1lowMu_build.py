#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the lowMu menu
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

import sys
from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

ex = ExecStep.ExecStep('athena')
ex.type = 'other'
ex.executable = 'runHLT_standalone_newJO.py'
ex.input = 'minbias'
ex.args += ' --filesInput='+Input.get_input(ex.input).paths[0]
ex.args += ' Trigger.triggerMenuSetup="PhysicsP1_pp_lowMu_run3_v1"'
ex.args += ' Trigger.disableChains=[]'  # Do not disable any chains
ex.args += ' Trigger.doRuntimeNaviVal=True'
ex.args += ' Trigger.L1.doAlfaCtpin=True'
ex.args += ' IOVDb.GlobalTag="OFLCOND-MC21-SDR-RUN3-07"'
ex.prmon = False

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_v1lowMu_build.new'
sys.exit(test.run())
