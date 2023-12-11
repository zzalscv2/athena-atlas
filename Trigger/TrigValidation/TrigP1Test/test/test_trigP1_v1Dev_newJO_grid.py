#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu in CA compared to legacy
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
from TrigValTools.TrigValSteering.Step import get_step_from_list

# Specify trigger menu once here:
triggermenu = 'Dev_pp_run3_v1_TriggerValidation_prescale'

# Run Legacy menu
exLegacy = ExecStep.ExecStep('Legacy')
exLegacy.type = 'athenaHLT'
exLegacy.job_options = 'TriggerJobOpts/runHLT_standalone.py'
exLegacy.input = 'data'
exLegacy.threads = 4
exLegacy.concurrent_events = 4
exLegacy.args += f' -c "setMenu=\'{triggermenu}\';doL1Sim=True;"'
exLegacy.args += ' --dump-config-reload'

# Merge Legacy histograms
# To be able to create chain dump
hmLegacy = CheckSteps.RootMergeStep('HistMergeLegacy')
hmLegacy.merged_file = 'expert-monitoring_Legacy.root'
hmLegacy.input_file = 'athenaHLT_workers/*/expert-monitoring.root expert-monitoring.root'

# ChainDump to create Legacy reference
cdLegacy = ExecStep.ExecStep('ChainDumpLegacy')
cdLegacy.type = 'other'
cdLegacy.executable = 'chainDump.py'
cdLegacy.input = ''
cdLegacy.args = '-f expert-monitoring_Legacy.root --yaml ChainDumpLegacy.yml'
cdLegacy.auto_report_result = False
cdLegacy.prmon = False

# Run CA menu
exCA = ExecStep.ExecStep('CA')
exCA.type = 'athenaHLT'
exCA.job_options = 'TriggerJobOpts.runHLT'
exCA.input = 'data'
exCA.threads = 4
exCA.concurrent_events = 4
exCA.flags = [f'Trigger.triggerMenuSetup=\"{triggermenu}\"',
            'Trigger.doLVL1=True']

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [exLegacy, hmLegacy, cdLegacy, exCA]
test.check_steps = CheckSteps.default_check_steps(test)

# Rename CA ChainDump
cdCA = get_step_from_list("ChainDump", test.check_steps)
cdCA.args = '--yaml ChainDumpCA.yml'

# Add a step comparing counts from CA against the Legacy
countComp = CheckSteps.ChainCompStep()
countComp.reference = 'ChainDumpLegacy.yml'
countComp.input_file = 'ChainDumpCA.yml'
countComp.explicit_reference = True # Test produces the reference
countComp.required = True # Final exit code doesn't depend on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, countComp)

import sys
sys.exit(test.run())
