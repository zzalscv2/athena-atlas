#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of the PhysicsP1_pp_run3_v1 menu, then running BS decoding follows the athenaHLT process
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, PyStep

##################################################
# Helper functions to build the test steps
##################################################
from TrigP1Test.TrigP1TestSteps import filterBS, decodeBS, check_hlt_properties

##################################################
# Test definition
##################################################

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data'
ex.args = '-c "setMenu=\'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale\';doL1Sim=True;'
ex.args += ';'.join(['flags.Trigger.L1MuonSim.NSWVetoMode=False',
                     'flags.Trigger.L1MuonSim.doMMTrigger=False',
                     'flags.Trigger.L1MuonSim.doPadTrigger=False',
                     'flags.Trigger.L1MuonSim.doStripTrigger=False']) + '"'
ex.args += ' -o output'
ex.args += ' --dump-config-reload'

# Extract and decode physics_Main
filterMain = filterBS("Main")
decodeMain = decodeBS("Main")

# Extract and decode calibration_CostMonitoring
filterCost = filterBS("CostMonitoring")
decodeCost = decodeBS("CostMonitoring", moduleID=1)

# Check a few important job options
checkProperties = PyStep.PyStep(check_hlt_properties, name="CheckProperties")
checkProperties.required = True

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex, filterMain, decodeMain, filterCost, decodeCost]
test.check_steps = CheckSteps.default_check_steps(test) + [checkProperties]

import sys
sys.exit(test.run())
