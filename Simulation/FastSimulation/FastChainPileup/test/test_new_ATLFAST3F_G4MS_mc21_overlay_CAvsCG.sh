#!/bin/sh
#
# art-description: CA vs Legacy code diff (ATLFAST3F_G4MS with overlay)
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-output: run_*
# art-output: pkldiff.log
# art-architecture: '#x86_64-intel'

maxevent=25
EVNT_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run3/v3/mc21a_presampling.RDO.pool.root"
RDO_File="MC_plus_MC.RDO.pool.root"
AOD_File="MC_plus_MC.AOD.pool.root"
NTUP_File="MC_plus_MC.NTUP.pool.root"

mkdir ./run_cg_pkl; cd run_cg_pkl
FastChain_tf.py \
  --simulator ATLFAST3F_G4MS \
  --steering doFCwOverlay \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-05' \
  --geometryVersion 'ATLAS-R3S-2021-02-00-00' \
  --postInclude 'default:PyJobTransforms/UseFrontier.py' \
  --preInclude 'all:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.FrozenShowersFCalOnly.py,Campaigns/MC21a.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --DataRunNumber '330000' \
  --athenaopts '"--config-only=ConfigCG.pkl"' \
  --imf False

cgpkl=999
if [ -f "ConfigCG.pkl" ]; then
    cgpkl=0
fi
echo "art-result: $cgpkl EVNTtoRDO_CG_PKL"

cd ..; mkdir ./run_cg; cd run_cg
FastChain_tf.py \
  --simulator ATLFAST3F_G4MS \
  --steering doFCwOverlay \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-05' \
  --geometryVersion 'ATLAS-R3S-2021-02-00-00' \
  --postInclude 'default:PyJobTransforms/UseFrontier.py' \
  --preInclude 'all:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.FrozenShowersFCalOnly.py,Campaigns/MC21a.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --DataRunNumber '330000' \
  --imf False
cg=$?
echo "art-result: $cg EVNTtoRDO_CG"

cd ../; mkdir run_ca; cd run_ca
FastChain_tf.py \
  --CA \
  --simulator ATLFAST3F_G4MS \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --preInclude 'Campaigns.MC21Simulation' 'Campaigns.MC21a' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --conditionsTag OFLCOND-MC21-SDR-RUN3-05  \
  --geometryVersion ATLAS-R3S-2021-02-00-00 \
  --imf False
ca=$?
echo  "art-result: $ca EVNTtoRDO_CA"
cd ..

diff=999
pkldiff=999
if [ $cg -eq 0 ] && [ $ca -eq 0 ]
then
   confTool.py --diff --ignoreIrrelevant --shortenDefaultComponents --ignoreDefaults run_cg_pkl/ConfigCG.pkl run_ca/ConfigCA.pkl > pkldiff.log
   pkldiff=$(grep -o 'differ' pkldiff.log | wc -l)

   art.py compare ref run_ca/RDO_CA.pool.root run_cg/RDO_CG.pool.root --mode=semi-detailed --entries 10
   diff=$?
fi
echo  "art-result: ${pkldiff} pklDiff"
echo  "art-result: ${diff} regression"
