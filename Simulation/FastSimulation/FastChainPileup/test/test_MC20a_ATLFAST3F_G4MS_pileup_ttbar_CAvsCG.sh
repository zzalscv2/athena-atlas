#!/bin/sh
#
# art-description: CA vs Legacy code diff (ATLFAST3F_G4MS with pileup profile) for MC20a ttbar
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

events=25
EVNT_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.110401.PowhegPythia_P2012_ttbar_nonallhad.evgen.EVNT.e3099.01517252._000001.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/*"
RDO_File="RDO.pool.root"
AOD_File="AOD.pool.root"
NTUP_File="NTUP.pool.root"

mkdir -p run_ca; cd run_ca
FastChain_tf.py \
    --CA \
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
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --preInclude 'Campaigns.MC20a' 'Campaigns.MC16SimulationNoIoV' \
    --postInclude='PyJobTransforms.UseFrontier' \
    --postExec 'from IOVDbSvc.CondDB import conddb;conddb.addOverride("/TILE/OFL02/CALIB/SFR","TileOfl02CalibSfr-SIM-05")' 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
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
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --preInclude 'Campaigns/MC20a.py,Campaigns/PileUpMC20a.py,Campaigns/MC16SimulationNoIoV.py' \
    --postInclude='PyJobTransforms/UseFrontier.py' \
    --postExec 'from IOVDbSvc.CondDB import conddb;conddb.addOverride("/TILE/OFL02/CALIB/SFR","TileOfl02CalibSfr-SIM-05")' \
    --athenaopts '"--config-only=ConfigCG.pkl"' \
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
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --preInclude 'Campaigns/MC20a.py,Campaigns/PileUpMC20a.py,Campaigns/MC16SimulationNoIoV.py' \
    --postInclude='PyJobTransforms/UseFrontier.py' \
    --postExec 'from IOVDbSvc.CondDB import conddb;conddb.addOverride("/TILE/OFL02/CALIB/SFR","TileOfl02CalibSfr-SIM-05")' \
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
    Reco_tf.py --inputRDOFile run_cg/${RDO_File} \
               --outputAODFile ${AOD_File} \
               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
               --maxEvents '-1' \
               --autoConfiguration=everything \
               --athenaopts "all:--threads=1" \
               --postExec 'RDOtoRDOTrigger:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RDOtoRDOTrigger_config.txt")' 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --imf False

     rec=$?
     if [ ${rec} -eq 0 ]
     then
         # NTUP prod.
         Reco_tf.py --inputAODFile ${AOD_File} \
                    --outputNTUP_PHYSVALFile ${NTUP_File} \
                    --maxEvents '-1' \
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
