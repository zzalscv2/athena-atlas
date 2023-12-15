#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: PhysicsP1_pp_run3_v1 menu test only dumping options for SMK generation
# art-type: build
# art-include: main/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# as per test_trigP1_v1PhysP1_noL1Sim_dumpConfig__build.py
ex_leg = ExecStep.ExecStep('ConfigLegacy')
ex_leg.type = 'athena'
ex_leg.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex_leg.input = 'ttbar'
ex_leg.threads = 1
precommand = ''.join([
  "setMenu='Dev_pp_run3_v1_TriggerValidation_prescale';",  
  "doWriteBS=False;",
  "doWriteRDOTrigger=True;",
])
ex_leg.args = '-c "{:s}"'.format(precommand)
ex_leg.config_only = True
ex_leg.workdir = 'legacy'

ex_CA = ExecStep.ExecStep('ConfigCA')
ex_CA.type = 'athena'
ex_CA.job_options = 'TriggerJobOpts/runHLT.py'
ex_CA.input = 'ttbar'
ex_CA.args = '--CA'
ex_CA.threads = 1
ex_CA.flags = [
    'Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"',
    'Trigger.doRuntimeNaviVal=True',
    'Output.doWriteRDO=True',
]
ex_CA.config_only = True
ex_CA.workdir = 'CA'

# Run confTool to convert pkl to json
json_cnv_legacy = ExecStep.ExecStep('HLTJsonCnvLegacy')
json_cnv_legacy.type = 'other'
json_cnv_legacy.executable = 'confTool.py'
json_cnv_legacy.input = ''
json_cnv_legacy.args += ' --toJSON legacy/athena.ConfigLegacy.pkl'
json_cnv_legacy.depends_on_previous = True

json_cnv_CA = ExecStep.ExecStep('HLTJsonCnvCA')
json_cnv_CA.type = 'other'
json_cnv_CA.executable = 'confTool.py'
json_cnv_CA.input = ''
json_cnv_CA.args += ' --toJSON CA/athena.ConfigCA.pkl'
json_cnv_CA.depends_on_previous = True

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
json_diff.args += ' CA/athena.ConfigCA.json legacy/athena.ConfigLegacy.json' # Inputs
json_diff.depends_on_previous = True

postprocessing = ExecStep.ExecStep('SanitiseDiff')
postprocessing.type = 'other'
postprocessing.executable = 'clean-suppression-messages.py'
postprocessing.input = ''
postprocessing.args = 'HLTJsonDiff.log CA_legacy_json_diff_clean.txt'
postprocessing.depends_on_previous = True

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_leg, ex_CA, json_cnv_legacy, json_cnv_CA, json_diff, postprocessing]
# Only keep relevant checks from the defaults
test.check_steps = [chk for chk in CheckSteps.default_check_steps(test)
                    if type(chk) is CheckSteps.CheckLogStep]
# No log merging because we don't fork - force checking only the mother log
for chk in test.check_steps:
    chk.log_file = 'CA/athena.ConfigCA.log'

import sys
sys.exit(test.run())
