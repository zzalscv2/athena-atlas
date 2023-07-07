#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: perform debug recovery from PU crash using PhysicsP1 menu
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps


#====================================================================================================
# HLT BS_RDO->RAW
hlt = ExecStep.ExecStep()
hlt.type = 'Trig_reco_tf'
hlt.forks = 1
hlt.threads = 1
hlt.concurrent_events = 1
hlt.max_events = 50
hlt.args  = '--precommand=\\\"setMenu=\\\'PhysicsP1_pp_run3_v1\\\'\\\"'
hlt.args += ' --streamSelection=Main,BphysDelayed,VBFDelayed'
hlt.args += ' --prodSysBSRDO True'
hlt.args += ' --outputBSFile=RAW.pool.root'
hlt.args += ' --outputHIST_DEBUGSTREAMMONFile=HIST_DEBUGSTREAMMON.ntup.root' # Turn on debug recovery step
hlt.input = 'data_dbg_stream'


#====================================================================================================
# The full test
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hlt]
test.check_steps = CheckSteps.default_check_steps(test)

from TrigValTools.TrigValSteering.CheckSteps import LogMergeStep

# Rename Trig_reco_tf.log to athena.log for linking in ART Monitor
logmerge = LogMergeStep()
logmerge.merged_name = 'athena.log'
logmerge.log_files = ['Trig_reco_tf.log']
test.check_steps.append(logmerge)

# Overwrite default MessageCount settings
msgcount = test.get_step("MessageCount")
msgcount.thresholds = {
   'INFO': 320,
   'WARNING': 40, # ATR-22815
   'other': 10
}

msgcount.required = True

test.check_steps.remove(test.get_step("ZeroCounts"))

import sys
sys.exit(test.run())
