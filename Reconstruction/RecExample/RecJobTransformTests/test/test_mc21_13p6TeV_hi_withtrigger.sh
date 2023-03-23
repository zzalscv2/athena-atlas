#!/bin/sh
#
# art-description: heavy ion reconstruction on mc21 using ttbar
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
export TRF_ECHO=True; Reco_tf.py  \
--multithreaded \
--inputHITSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/mc21_13p6TeV/HITSFiles/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/HITS.29625927._000632.pool.root.1 \
--outputESDFile=ESD.pool.root \
--outputAODFile=AOD.pool.root \
--maxEvents=20 \
--steering 'doRDO_TRIG' \
--conditionsTag 'all:OFLCOND-MC21-SDR-RUN3-07' \
--postInclude 'all:RecJobTransforms/UseFrontier.py' \
--postExec  'r2a:y=(StreamAOD.ItemList if "StreamAOD" in dir() else []);y+=["xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.SECOND_R.SECOND_LAMBDA.CENTER_MAG.CENTER_LAMBDA.FIRST_ENG_DENS.ENG_FRAC_MAX.ISOLATION.ENG_BAD_CELLS.N_BAD_CELLS.BADLARQ_FRAC.ENG_POS.AVG_LAR_Q.AVG_TILE_Q.EM_PROBABILITY.BadChannelList.CELL_SIGNIFICANCE.CELL_SIG_SAMPLING"];' \
--preExec  'r2a:from InDetRecExample.InDetJobProperties import InDetFlags;InDetFlags.cutLevel.set_Value_and_Lock(4);jobproperties.Beam.bunchSpacing.set_Value_and_Lock(100);rec.doDPD.set_Value_and_Lock(True);' 'all:from AthenaMonitoring.DQMonFlags import jobproperties; jobproperties.DQMonFlagsCont.doHIMon.set_Value_and_Lock(False);rec.doZdc.set_Value_and_Lock(False);rec.doHeavyIon.set_Value_and_Lock(True);' 'r2t:from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Trigger.triggerMenuSetup="Dev_HI_run3_v1"; ConfigFlags.Trigger.AODEDMSet = "AODFULL"; ConfigFlags.Trigger.HLTSeeding.forceEnableAllChains=True;' \
--autoConfiguration 'everything' \
--DataRunNumber '313000' 

RES=$?
echo "art-result: $RES Reco"

