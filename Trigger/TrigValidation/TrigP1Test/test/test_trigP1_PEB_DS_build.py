#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of partial event building and data scouting
# art-type: build                                                                  
# art-include: main/Athena
# art-include: 23.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigP1Test.TrigP1TestSteps import TrigBSDumpGrepStep

output_name_base = 'output.test_trigP1_PEB_DS'

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TrigP1Test.PEBDSTest.run'
ex.input = 'data'
ex.args = '-o ' + output_name_base

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Step checking if there is at least one event in each of the streams defined in this test
all_streams = [
  'physics_Main',
  'physics_TestPEBOne',
  'calibration_TestPEBTwo',
  'physics_TestPEBThree',
  'calibration_TestPEBFour',
  'physics_ElectronDSTest',
  'physics_ElectronDSPEBTest',
]
for stream_name in all_streams:
  stream_count_step = TrigBSDumpGrepStep('StreamCount_'+stream_name)
  stream_count_step.args += '--stag'
  stream_count_step.file_name_base = output_name_base
  stream_count_step.regex = stream_name
  if 'PEB' in stream_name or 'DS' in stream_name:
    stream_count_step.regex += '.*Partial Event Building'
  else:
    stream_count_step.regex += '.*Full Event Building'
  test.check_steps.append(stream_count_step)

import sys
sys.exit(test.run())
