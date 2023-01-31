#!/bin/sh
#
# art-description: CA vs Legacy code diff (ATLFAST3F_G4MS with overlay)
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-output: run_*
# art-output: pkldiff.log
# art-architecture: '#x86_64-intel'

events=50
EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1'
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"
RDO_File="MC_plus_MC.RDO.pool.root"

mkdir ./run_cg_pkl; cd run_cg_pkl
FastChain_tf.py \
  --runNumber 601229 \
  --simulator ATLFAST3F_G4MS \
  --steering doFCwOverlay \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --jobNumber 1 \
  --DataRunNumber 410000 \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --postInclude 'default:PyJobTransforms/UseFrontier.py' \
  --preInclude 'all:Campaigns/MC21a.py,Campaigns/MC21SimulationNoIoV.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --athenaopts '"--config-only=ConfigCG.pkl"' \
  --imf False

cgpkl=999
if [ -f "ConfigCG.pkl" ]; then
    cgpkl=0
fi
echo "art-result: $cgpkl EVNTtoRDO_CG_PKL"

cd ..; mkdir ./run_cg; cd run_cg
FastChain_tf.py \
  --runNumber 601229 \
  --simulator ATLFAST3F_G4MS \
  --steering doFCwOverlay \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --jobNumber 1 \
  --DataRunNumber 410000 \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --postInclude 'default:PyJobTransforms/UseFrontier.py' \
  --preInclude 'all:Campaigns/MC21a.py,Campaigns/MC21SimulationNoIoV.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --imf False
cg=$?
echo "art-result: $cg EVNTtoRDO_CG"

cd ../; mkdir run_ca; cd run_ca
FastChain_tf.py \
  --CA \
  --runNumber 601229 \
  --simulator ATLFAST3F_G4MS \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --jobNumber 1 \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --preInclude 'Campaigns.MC21a' 'Campaigns.MC21SimulationNoIoV' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07'  \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
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

   art.py compare ref run_ca/${RDO_File} run_cg/${RDO_File} --mode=semi-detailed --entries 10
   diff=$?
fi
echo  "art-result: ${pkldiff} pklDiff"
echo  "art-result: ${diff} regression"
