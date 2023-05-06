#!/bin/sh
#
# art-description: CA vs Legacy code diff (ATLFAST3F_G4MS with pileup profile) for MC21a ttbar
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena
# art-output: run_*
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: *DO.pool.root
# art-output: pkldiff.log
# art-architecture: '#x86_64-intel'

events=50
EVNT_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3876_s3880/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3876_s3880/*"
RDO_File="RDO.pool.root"
AOD_File="AOD.pool.root"
NTUP_File="NTUP.pool.root"

mkdir -p run_ca; cd run_ca
FastChain_tf.py \
    --CA \
    --runNumber 601229 \
    --simulator 'ATLFAST3F_G4MS' \
    --physicsList 'FTFP_BERT_ATL' \
    --useISF True \
    --jobNumber 1 \
    --randomSeed 123 \
    --digiSteeringConf "StandardSignalOnlyTruth" \
    --inputEVNTFile ${EVNT_File} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --outputRDOFile ${RDO_File} \
    --maxEvents ${events} \
    --skipEvents 0 \
    --digiSeedOffset1 '511' \
    --digiSeedOffset2 '727' \
    --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
    --preInclude 'Campaigns.MC21a' 'Campaigns.MC21SimulationNoIoV' \
    --postInclude 'PyJobTransforms.UseFrontier' 'Digitization.DigitizationSteering.DigitizationTestingPostInclude' \
    --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

ca=$?
echo  "art-result: $ca EVNTtoRDO_CA"
status=$ca
cp log.EVNTtoRDO ../log.EVNTtoRDO_CA
cp ${RDO_File} ../CA.${RDO_File}
capkl=-9999
if [ -f "ConfigCA.pkl" ]; then
    capkl=0
    cp ConfigCA.pkl ../ConfigCA.pkl
fi
cd ../


mkdir -p ./run_cg_pkl; cd run_cg_pkl
FastChain_tf.py \
    --runNumber 601229 \
    --simulator 'ATLFAST3F_G4MS' \
    --physicsList 'FTFP_BERT_ATL' \
    --useISF True \
    --jobNumber 1 \
    --randomSeed 123 \
    --digiSteeringConf "StandardSignalOnlyTruth" \
    --inputEVNTFile ${EVNT_File} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --outputRDOFile ${RDO_File} \
    --maxEvents ${events} \
    --skipEvents 0 \
    --digiSeedOffset1 '511' \
    --digiSeedOffset2 '727' \
    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
    --preInclude 'all:Campaigns/MC21a.py,Campaigns/PileUpMC21a.py,Campaigns/MC21SimulationNoIoV.py' \
    --postInclude='PyJobTransforms/UseFrontier.py' \
    --postExec 'default:job+=CfgMgr.JobOptsDumperAlg(FileName="PileupLegacyConfig.txt");from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("LegacyConfig.txt")' \
    --athenaopts '"--config-only=ConfigCG.pkl"'\
    --imf False


cgpkl=-9999
if [ -f "ConfigCG.pkl" ]; then
    cgpkl=0
    cp ConfigCG.pkl ../ConfigCG.pkl
fi
echo "art-result: $cgpkl EVNTtoRDO_CG_PKL"
cd ../

mkdir -p ./run_cg; cd run_cg
FastChain_tf.py \
    --runNumber 601229 \
    --simulator 'ATLFAST3F_G4MS' \
    --physicsList 'FTFP_BERT_ATL' \
    --useISF True \
    --jobNumber 1 \
    --randomSeed 123 \
    --digiSteeringConf "StandardSignalOnlyTruth" \
    --inputEVNTFile ${EVNT_File} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --outputRDOFile ${RDO_File} \
    --maxEvents ${events} \
    --skipEvents 0 \
    --digiSeedOffset1 '511' \
    --digiSeedOffset2 '727' \
    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
    --preInclude 'all:Campaigns/MC21a.py,Campaigns/PileUpMC21a.py,Campaigns/MC21SimulationNoIoV.py' \
    --postInclude='PyJobTransforms/UseFrontier.py' \
    --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("LegacyConfig.txt")' \
    --imf False

cg=$?
cp log.EVNTtoRDO ../log.EVNTtoRDO_CG
cp ${RDO_File} ../${RDO_File}
echo "art-result: $cg EVNTtoRDO_CG"
if [ $status -eq 0 ]
then
    status=$cg
fi
cd ../

pkldiff=-9999
if [ $cgpkl -eq 0 ] && [ $capkl -eq 0 ]
then
   confTool.py --diff --ignoreIrrelevant --shortenDefaultComponents --ignoreDefaults "run_cg_pkl/ConfigCG.pkl" "run_ca/ConfigCA.pkl" > pkldiff.log
   pkldiff=$(grep -o 'differ' pkldiff.log | wc -l)
fi
echo  "art-result: $pkldiff pklDiff"

diff=-9999
if [ $status -eq 0 ]
then
   acmd.py diff-root run_cg/${RDO_File} run_ca/${RDO_File} --mode=semi-detailed --error-mode resilient --entries 10
   diff=$?
   status=$diff
fi
echo  "art-result: $diff OLDvsCA"

reg=-9999
if [ $cg -eq 0 ]
then
   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
   reg=$?
   status=$reg
fi
echo  "art-result: $reg regression"

rec=-9999
ntup=-9999
if [ ${cg} -eq 0 ]
then
    # Reconstruction
    Reco_tf.py \
               --CA "all:True" "RDOtoRDOTrigger:False" \
               --inputRDOFile run_cg/${RDO_File} \
               --outputAODFile ${AOD_File} \
               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
               --maxEvents '-1' \
               --autoConfiguration=everything \
               --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
               --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
               --athenaopts "all:--threads=1" \
               --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --imf False

     rec=$?
     if [ ${rec} -eq 0 ]
     then
         # NTUP prod.
         Reco_tf.py --inputAODFile ${AOD_File} \
                    --outputNTUP_PHYSVALFile ${NTUP_File} \
                    --maxEvents '-1' \
                    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
                    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
                    --ignoreErrors True \
                    --validationFlags 'doInDet' \
                    --valid 'True'
         ntup=$?
         status=$ntup
     fi
fi

echo  "art-result: $rec reconstruction"
echo  "art-result: $ntup physics validation"

exit $status
