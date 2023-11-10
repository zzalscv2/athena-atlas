#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: PhysicsP1_pp_run3_v1 menu test only dumping options for SMK generation
# art-type: build
# art-include: main/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# as per test_trigP1_v1PhysP1_noL1Sim_dumpConfig__build.py
ex_leg = ExecStep.ExecStep('ConfigLegacy')
ex_leg.type = 'athenaHLT'
ex_leg.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex_leg.input = ''  # No input file needed to generate config
ex_leg.args = '-c "setMenu=\'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale\';"'
ex_leg.args += ' -M --dump-config-exit'
ex_leg.perfmon = False  # Cannot use PerfMon with -M
ex_leg.fpe_auditor = False  # Don't want FPEAuditor in SMK for P1
ex_leg.workdir = 'legacy'

ex_CA = ExecStep.ExecStep('ConfigCA')
ex_CA.type = 'athenaHLT'
ex_CA.job_options = 'TriggerJobOpts.runHLT'
ex_CA.input = ''
ex_CA.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_run3_v1_HLTReprocessing_prescale"']
ex_CA.args += ' -M --dump-config-exit'
ex_CA.perfmon = False
ex_CA.fpe_auditor = False
ex_CA.workdir = 'CA'

# Skip comparison of properties that depend on the chain names
ignore_properties = [
    "LegToInputCollectionMap",
    "HypoTools",
    "FinalDecisions",
    "MultiplicitiesMap",
    "FinalDecisionKeys",
    "FinalStepDecisions",
    "FixLinks",
    "TrigCompositeContainer",
    "Decisions",
    "Chains",
    "ChainsPerInput",
    "CollectionsToSerialize",
    "InputMakerInputDecisions",
    "DecisionCollectorTools",
]

# Run confTool to diff JOs
json_diff = ExecStep.ExecStep('HLTJsonDiff')
json_diff.type = 'other'
json_diff.executable = 'confTool.py'
json_diff.input = ''
json_diff.args = '--diff --ignoreMissing --ignoreIrrelevant --ignoreDefaults'
json_diff.args += ' '.join([' --ignore '+prop for prop in ignore_properties])
json_diff.args += ' CA/HLTJobOptions.json legacy/HLTJobOptions.json' # Inputs
json_diff.depends_on_previous = True

postprocessing = ExecStep.ExecStep('SanitiseDiff')
postprocessing.type = 'other'
postprocessing.executable = 'clean-suppression-messages.py'
postprocessing.input = ''
postprocessing.args = 'HLTJsonDiff.log CA_legacy_json_diff_clean.txt'
postprocessing.depends_on_previous = True

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_leg, ex_CA, json_diff, postprocessing]
# Only keep relevant checks from the defaults
test.check_steps = [chk for chk in CheckSteps.default_check_steps(test)
                    if type(chk) is CheckSteps.CheckLogStep]
# No log merging because we don't fork - force checking only the mother log
for chk in test.check_steps:
    chk.log_file = 'CA/athenaHLT.ConfigCA.log'

import sys
sys.exit(test.run())
