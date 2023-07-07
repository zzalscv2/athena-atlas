#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test running HITS->RDO in master, then RDO->RDO_TRIG in 21.0, then RDO_TRIG->AOD in master, then AOD->DAOD with multiprocess in master
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input, Step
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# HITS -> RDO step in master
hit2rdo = ExecStep.ExecStep('HITtoRDO')
hit2rdo.type = 'Reco_tf'
hit2rdo.input = 'ttbar_HITS'

hit2rdo_lumidict = {
  'run': 310000,
  'startmu': 40.0,
  'endmu': 70.0,
  'stepmu': 1.0,
  'startlb': 1,
  'timestamp': 1550000000
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

# RDO -> RDO_TRIG step in 21.0
rdo2rdotrig = ExecStep.ExecStep('RDOtoRDOTrigger')
rdo2rdotrig.type = 'Reco_tf'
rdo2rdotrig.input = ''
rdo2rdotrig.imf = False
rdo2rdotrig.explicit_input = True
rdo2rdotrig.args = '--inputRDOFile=RDO.pool.root --outputRDO_TRIGFile=RDO_TRIG.pool.root'
rdo2rdotrig.args += ' --asetup="RDOtoRDOTrigger:Athena,21.0,latest"'
rdo2rdotrig.args += ' --triggerConfig="MCRECO:MC_pp_v7_BulkMCProd_mc_prescale"'
rdo2rdotrig.args += ' --imf="all:True"'

# Clear AthFile cache from r21 because it is incompatible with py3 r22 (ATR-21489)
rm_cache = ExecStep.ExecStep('ClearAthFileCache')
rm_cache.type = 'other'
rm_cache.input = ''
rm_cache.executable = 'rm'
rm_cache.args = '-f athfile-cache.ascii.gz'
rm_cache.auto_report_result = False  # Do not set art-result for this step
rm_cache.output_stream = Step.Step.OutputStream.STDOUT_ONLY  # Do not create a log file for this step

# RDO_TRIG -> AOD step in master
rdotrig2aod = ExecStep.ExecStep('RDOTriggertoAOD')
rdotrig2aod.type = 'Reco_tf'
rdotrig2aod.input = ''
rdotrig2aod.explicit_input = True
rdotrig2aod.args = '--inputRDO_TRIGFile=RDO_TRIG.pool.root --outputAODFile=AOD.pool.root --steering "doRDO_TRIG" "doTRIGtoALL"'
rdotrig2aod.args += ' --preExec="all:from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Trigger.AODEDMSet=\'AODFULL\'"'
rdotrig2aod.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-09"'
rdotrig2aod.args += ' --postExec="from OutputStreamAthenaPool.MultipleStreamManager import MSMgr; aod=MSMgr.GetStream(\\\"StreamAOD\\\"); aod.AddItem(\\\"xAOD::TrigCompositeContainer#HLTNav_R2ToR3Summary\\\"); aod.AddItem(\\\"xAOD::TrigCompositeAuxContainer#HLTNav_R2ToR3SummaryAux.\\\"); "'

# AOD -> DAOD
aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = ''
aod2daod.forks = 4
aod2daod.explicit_input = True
aod2daod.args = '--inputAODFile=AOD.pool.root --outputDAODFile=DAOD.pool.root --CA --formats=PHYS'
aod2daod.args += ' --sharedWriter=True --athenaMPMergeTargetSize "DAOD_*:0"'
aod2daod.args += ' --asetup="all:Athena,master,latest"'

# Define the test with the above steps
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hit2rdo, rdo2rdotrig, rm_cache, rdotrig2aod, aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
