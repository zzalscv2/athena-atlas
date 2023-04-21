#!/bin/sh
#
# art-description: MC23-style RUN3 simulation using FullG4MT_QS in AthenaMT
# art-include: 23.0/Athena
# art-include: master/Athena
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R3S-2021-03-01-00 and OFLCOND-MC21-SDR-RUN3-07
Sim_tf.py \
    --CA \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postExec 'all:from IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002"))' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc22_pre/valid2.900311.Epos_minbias_inelastic_lowjetphoton.evgen.EVNT.e8480/EVNT.30415957._001017.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 200 \
    --postExec 'EVNTtoHITS:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
Sim_tf.py \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postExec 'all:conddb.addOverride("/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002");' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc22_pre/valid2.900311.Epos_minbias_inelastic_lowjetphoton.evgen.EVNT.e8480/EVNT.30415957._001017.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 200 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postExec 'all:conddb.addOverride("/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002");' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc22_pre/valid2.900311.Epos_minbias_inelastic_lowjetphoton.evgen.EVNT.e8480/EVNT.30415957._001017.pool.root.1" \
    --outputHITSFile "test.CG.HITS.pool.root" \
    --maxEvents 200 \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    # Compare the outputs - ignoring truth jet variables which are set to NaN.
    acmd.py diff-root test.CG.HITS.pool.root test.CA.HITS.pool.root \
        --error-mode resilient \
        --mode semi-detailed \
        --order-trees \
        --ignore-leaves index_ref xAOD::AuxContainerBase_AntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_AntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_AntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_AntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

rc5=-9999
if [ $rc -eq 0 ]
then
    FilterHit_tf.py \
        --CA \
        --inputHITSFile="test.CA.HITS.pool.root" \
        --maxEvents 200 \
        --postInclude 'PyJobTransforms.UseFrontier' \
        --skipEvents="0" \
        --TruthReductionScheme="SingleGenParticle" \
        --outputHITS_FILTFile="filt.CA.HITS.pool.root" \
        --postExec 'with open("ConfigFiltCA.pkl", "wb") as f: cfg.store(f)' \
        --imf False
    rc5=$?
    status=$rc5
    mv log.FilterHitTf log.FilterHitTf.CA
fi
echo  "art-result: $rc5 filtCA"

rc6=-9999
if [ $rc2 -eq 0 ]
then
    FilterHit_tf.py \
        --inputHITSFile="test.CG.HITS.pool.root" \
        --maxEvents 200 \
        --postInclude="default:PyJobTransforms/UseFrontier.py" \
        --skipEvents="0" \
        --TruthReductionScheme="SingleGenParticle" \
        --outputHITS_FILTFile="filt.CG.HITS.pool.root" \
        --imf False \
        --athenaopts '"--config-only=ConfigFiltCG.pkl"'

    FilterHit_tf.py \
        --inputHITSFile="test.CG.HITS.pool.root" \
        --maxEvents 200 \
        --postInclude="default:PyJobTransforms/UseFrontier.py" \
        --skipEvents="0" \
        --TruthReductionScheme="SingleGenParticle" \
        --outputHITS_FILTFile="filt.CG.HITS.pool.root" \
        --imf False
    rc6=$?
    status=$rc6
    mv log.FilterHitTf log.FilterHitTf.CG
fi
echo  "art-result: $rc6 filtOLD"

rc7=-9999
if [ $status -eq 0 ]
then
    # Compare the outputs - ignoring truth jet variables which are set to NaN.
    acmd.py diff-root filt.CG.HITS.pool.root filt.CA.HITS.pool.root \
        --error-mode resilient \
        --mode semi-detailed \
        --order-trees \
        --ignore-leaves index_ref xAOD::AuxContainerBase_AntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_AntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_AntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_AntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt
    rc7=$?
    status=$rc7
fi
echo "art-result: $rc7 filt_OLDvsCA"

rc8=-9999
if [ $rc6 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}  --order-trees --mode=semi-detailed --diff-root --file=filt.CG.HITS.pool.root
    rc8=$?
    status=$rc8
fi
echo  "art-result: $rc8 filt_regression"
exit $status
