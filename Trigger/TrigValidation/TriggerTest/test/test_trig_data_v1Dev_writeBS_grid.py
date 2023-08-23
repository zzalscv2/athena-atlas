#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger BS->BS athena test of the Dev_pp_run3_v1 menu
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

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Step
from TrigValTools.TrigValSteering.Common import find_file

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data'
ex.threads = 4
ex.concurrent_events = 4
ex.args = '-c "setMenu=\'Dev_pp_run3_v1_TriggerValidation_prescale\';doL1Sim=True;doWriteBS=True;doWriteRDOTrigger=False;"'

checkBS = Step.Step("CheckBS")
checkBS.executable = 'trigbs_dumpHLTContentInBS_run3.py'
checkBS.args = ' --l1 --hlt --hltres --stag --sizeSummary'
checkBS.args += ' ' + find_file('*unknown_SingleStream.daq.RAW.*Athena.*.data')
checkBS.timeout = 600  # 10 minutes
checkBS.required = True
checkBS.auto_report_result = True

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = [checkBS] + CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
