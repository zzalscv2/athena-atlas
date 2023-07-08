#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform HITS->RDO followed by RDO->RDO_TRIG
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

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

hit2rdo = ExecStep.ExecStep('HITtoRDO')
hit2rdo.type = 'Reco_tf'
hit2rdo.input = 'ttbar_HITS'
hit2rdo.max_events = 800
hit2rdo.args = '--outputRDOFile=RDO.pool.root'

pu_low = Input.get_input('pileup_low')
pu_high = Input.get_input('pileup_high')
hit2rdo.args += ' --inputLowPtMinbiasHitsFile=' + pu_low.paths[0]
hit2rdo.args += ' --inputHighPtMinbiasHitsFile=' + pu_high.paths[0]
hit2rdo.args += ' --numberOfCavernBkg="0"'
hit2rdo.args += ' --numberOfHighPtMinBias="0.15520183"'
hit2rdo.args += ' --numberOfLowPtMinBias="59.3447981771"'
hit2rdo.args += ' --pileupFinalBunch="6"'
hit2rdo.args += ' --jobNumber="1"'

hit2rdo.args += ' --preExec "HITtoRDO:userRunLumiOverride={\'run\':300000, \'startmu\':40.0, \'endmu\':70.0, \'stepmu\':1.0, \'startlb\':1, \'timestamp\': 1500000000};ScaleTaskLength=0.1"'
hit2rdo.args += ' --preInclude "HITtoRDO:Digitization/ForceUseOfPileUpTools.py,SimulationJobOptions/preInlcude.PileUpBunchTrainsMC16c_2017_Config1.py,RunDependentSimData/configLumi_muRange.py"'
hit2rdo.args += ' --postInclude="h2r:LArROD/LArSuperCellEnable.py"' # to be removed when Run3 geometry RDO is integrated in Trigger ART tests

rdo2rdotrig = ExecStep.ExecStep('RDOtoRDOTrigger')
rdo2rdotrig.type = 'Reco_tf'
rdo2rdotrig.input = ''
rdo2rdotrig.explicit_input = True
rdo2rdotrig.threads = 4
rdo2rdotrig.concurrent_events = 4
rdo2rdotrig.args = '--inputRDOFile=RDO.pool.root --outputRDO_TRIGFile=RDO_TRIG.pool.root'
rdo2rdotrig.args += ' --preExec="setMenu=\'Dev_pp_run3_v1_TriggerValidation_prescale\'"'
rdo2rdotrig.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-09"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hit2rdo, rdo2rdotrig]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
