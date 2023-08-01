#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test running HITS->RDO in main/23.0, then RDO->RDO_TRIG in 21.0-mc16a, then RDO_TRIG->AOD in main/23.0, then AOD->DAOD with multiprocess in main
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input, Step
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# HITS -> RDO step in main/23.0
hit2rdo = ExecStep.ExecStep('HITtoRDO')
hit2rdo.type = 'Reco_tf'
hit2rdo.input = 'ttbar_HITS'

hit2rdo_lumidict = {
  'run': 284500,
  'startmu': 40.0,
  'endmu': 70.0,
  'stepmu': 1.0,
  'startlb': 1,
  'timestamp': 1446539185
}

hit2rdo_preexec = ';'.join([
  'userRunLumiOverride={:s}'.format(str(hit2rdo_lumidict)),
  'ScaleTaskLength=0.1'
]) + ';'

hit2rdo_preinclude= ','.join([
  'Digitization/ForceUseOfPileUpTools.py',
  'SimulationJobOptions/preInlcude.PileUpBunchTrainsMC16c_2017_Config1.py',
  'RunDependentSimData/configLumi_muRange.py'
])

pu_low = Input.get_input('pileup_low')
pu_high = Input.get_input('pileup_high')

hit2rdo.args = '--outputRDOFile=RDO.pool.root'
hit2rdo.args += ' --inputLowPtMinbiasHitsFile=' + pu_low.paths[0]
hit2rdo.args += ' --inputHighPtMinbiasHitsFile=' + pu_high.paths[0]
hit2rdo.args += ' --numberOfCavernBkg="0"'
hit2rdo.args += ' --numberOfHighPtMinBias="0.15520183"'
hit2rdo.args += ' --numberOfLowPtMinBias="59.3447981771"'
hit2rdo.args += ' --pileupFinalBunch="6"'
hit2rdo.args += ' --jobNumber="1"'
hit2rdo.args += ' --preExec "HITtoRDO:{:s}"'.format(hit2rdo_preexec)
hit2rdo.args += ' --preInclude "HITtoRDO:{:s}"'.format(hit2rdo_preinclude)
hit2rdo.args += ' --conditionsTag="HITtoRDO:OFLCOND-MC16-SDR-RUN2-11"'

# RDO -> RDO_TRIG step in 21.0
rdo2rdotrig = ExecStep.ExecStep('RDOtoRDOTrigger')
rdo2rdotrig.type = 'Reco_tf'
rdo2rdotrig.input = ''
rdo2rdotrig.imf = False
rdo2rdotrig.explicit_input = True
rdo2rdotrig.args = '--inputRDOFile=RDO.pool.root --outputRDO_TRIGFile=RDO_TRIG.pool.root'
rdo2rdotrig.args += ' --asetup="RDOtoRDOTrigger:Athena,21.0-mc16a,latest"'
rdo2rdotrig.args += ' --triggerConfig="MCRECO:MC_pp_v6_tight_mc_prescale"'
rdo2rdotrig.args += ' --imf="all:True"'
rdo2rdotrig.args += ' --preExec="all:from TriggerJobOpts.TriggerFlags import TriggerFlags; TriggerFlags.run2Config=\'2016\'"'
rdo2rdotrig.args += ' --conditionsTag="RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02a"'

# Clear AthFile cache from r21 because it is incompatible with py3 r22 (ATR-21489)
rm_cache = ExecStep.ExecStep('ClearAthFileCache')
rm_cache.type = 'other'
rm_cache.input = ''
rm_cache.executable = 'rm'
rm_cache.args = '-f athfile-cache.ascii.gz'
rm_cache.auto_report_result = False  # Do not set art-result for this step
rm_cache.output_stream = Step.Step.OutputStream.STDOUT_ONLY  # Do not create a log file for this step

# RDO_TRIG -> AOD step in main/23.0
rdotrig2aod = ExecStep.ExecStep('RDOTriggertoAOD')
rdotrig2aod.type = 'Reco_tf'
rdotrig2aod.input = ''
rdotrig2aod.explicit_input = True
rdotrig2aod.args = '--inputRDO_TRIGFile=RDO_TRIG.pool.root --outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
rdotrig2aod.args += ' --preExec="all:flags.Trigger.AODEDMSet=\'AODFULL\'"'
rdotrig2aod.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-11"'
rdotrig2aod.args += ' --postExec="all:from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; extraContent=[\'xAOD::TrigCompositeContainer#HLTNav_R2ToR3Summary\',\'xAOD::TrigCompositeAuxContainer#HLTNav_R2ToR3SummaryAux.\']; cfg.merge(addToAOD(flags, extraContent));"'
rdotrig2aod.args += ' --CA "all:True"'

# AOD -> DAOD step in main
aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = ''
aod2daod.forks = 4
aod2daod.explicit_input = True
aod2daod.args = '--inputAODFile=AOD.pool.root --outputDAODFile=DAOD.pool.root --CA --formats=PHYS'
aod2daod.args += ' --sharedWriter=True --athenaMPMergeTargetSize "DAOD_*:0"'
aod2daod.args += ' --asetup="all:Athena,main,latest"'

# Define the test with the above steps
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hit2rdo, rdo2rdotrig, rm_cache, rdotrig2aod, aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
