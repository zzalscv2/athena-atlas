#!/bin/sh
#
# art-description: CA-based config Track-overlay for MC21a ttbar
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena
# art-output: run_*
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-architecture: '#x86_64-intel'

events=50
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/HITS.29625925._010619_100evts.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3.pool.root"
RDO_File="RDO.pool.root"
AOD_File="AOD.pool.root"
NTUP_File="NTUP.pool.root"

mkdir -p run_ca; cd run_ca
Overlay_tf.py \
  --CA \
  --inputHITSFile ${HITS_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --preInclude 'Campaigns.MC21a' \
  --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07'  \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --preExec 'ConfigFlags.Overlay.doTrackOverlay=True;' \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False
ca=$?
echo  "art-result: $ca HITStoRDO_CA"
# Copy outputs back to main directory
cp ${RDO_File} ../${RDO_File}
cp log.* ../
cp *.txt ../
status=$ca
if [ -f "ConfigCA.pkl" ]; then
    cp ConfigCA.pkl ../ConfigCA.pkl
fi
cd ../

reg=-9999
if [ $ca -eq 0 ]
then
   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
   reg=$?
   status=$reg
fi
echo  "art-result: $reg regression"

rec=-9999
ntup=-9999
if [ ${ca} -eq 0 ]
then
    # Reconstruction
    Reco_tf.py \
               --CA "all:True" "RDOtoRDOTrigger:False" \
               --inputRDOFile run_ca/${RDO_File} --maxEvents '-1' \
               --outputAODFile ${AOD_File} \
               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
               --autoConfiguration=everything \
               --athenaopts "all:--threads=1" \
               --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --preExec 'RAWtoALL:ConfigFlags.Overlay.doTrackOverlay=True;' 'RDOtoRDOTrigger:from OverlayCommonAlgs.OverlayFlags import overlayFlags; overlayFlags.doTrackOverlay=True; ConfigFlags.Overlay.doTrackOverlay=True;'\
               --imf False

     rec=$?
     if [ ${rec} -eq 0 ]
     then
         # NTUP prod.
         Reco_tf.py --inputAODFile ${AOD_File} --maxEvents '-1' \
                    --outputNTUP_PHYSVALFile ${NTUP_File} \
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
