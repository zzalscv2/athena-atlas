#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Comparison of CA vs legacy counts for the RDOtoRDOTrigger transform with Dev menu
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: expert*.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Step import get_step_from_list

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
])

# Run Legacy menu
exLegacy = ExecStep.ExecStep('Legacy')
exLegacy.type = 'Reco_tf'
exLegacy.input = 'ttbar_stau_bphys'
exLegacy.max_events = 2500
exLegacy.threads = 8
exLegacy.concurrent_events = 8
exLegacy.args = '--outputRDO_TRIGFile=RDO_TRIG_Legacy.pool.root'
exLegacy.args += f' --preExec="all:{preExec};"'

# Rename Legacy histograms
hmLegacy = CheckSteps.RootMergeStep('HistMergeLegacy')
hmLegacy.merged_file = 'expert-monitoring_Legacy.root'
hmLegacy.input_file = 'expert-monitoring.root'

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
exCA.type = 'Reco_tf'
exCA.input = 'ttbar_stau_bphys'
exCA.max_events = 2500
exCA.threads = 8
exCA.concurrent_events = 8
exCA.args = '--outputRDO_TRIGFile=RDO_TRIG_CA.pool.root'
exCA.args += f' --preExec="all:{preExec};"'
exCA.args += ' --CA "all:True"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [exLegacy, hmLegacy, cdLegacy, exCA]
test.check_steps = CheckSteps.default_check_steps(test)

# Rename CA ChainDump
cdCA = get_step_from_list("ChainDump", test.check_steps)
cdCA.args = '--yaml ChainDumpCA.yml'

# Add a step comparing counts from CA vs Legacy
countComp = CheckSteps.ChainCompStep()
countComp.reference = 'ChainDumpLegacy.yml'
countComp.input_file = 'ChainDumpCA.yml'
countComp.explicit_reference = True # Test produces the reference
countComp.required = True # Final exit code doesn't depend on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, countComp)

import sys
sys.exit(test.run())
