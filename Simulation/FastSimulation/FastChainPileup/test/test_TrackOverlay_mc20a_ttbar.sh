#!/bin/sh

# art-description: Run FastChain with Track-overlay for MC20a, ttbar
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: config.txt
# art-output: RAWtoALL_config.txt
# art-output: RDOtoRDOTrigger_config.txt
# art-architecture: '#x86_64-intel'

events=25
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO.26811885._035498_TrackOverlay_Run2.pool.root.1"
RDO_File="TrackOverlay.RDO.pool.root"
AOD_File="TrackOverlay.AOD.pool.root"
NTUP_File="TrackOverlay.NTUP.pool.root"

Overlay_tf.py \
  --inputHITSFile ${HITS_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --skipSecondaryEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag OFLCOND-MC16-SDR-RUN2-09  \
  --geometryVersion ATLAS-R2-2016-01-00-01 \
  --preInclude 'all:Campaigns/MC20a.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --preExec "from OverlayCommonAlgs.OverlayFlags import overlayFlags;overlayFlags.doTrackOverlay=True;" \
  --imf False 

rc=$?
echo "art-result: ${rc} HITStoRDO"


rc2=999
rc3=999
rc4=999
if [ ${rc} -eq 0 ]
then
    # Reconstruction
    Reco_tf.py --inputRDOFile ${RDO_File} \
               --maxEvents '-1' \
               --autoConfiguration=everything \
               --outputAODFile ${AOD_File} \
               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
               --athenaopts "all:--threads=1" \
               --postExec 'RDOtoRDOTrigger:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RDOtoRDOTrigger_config.txt")' 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --imf False

     rc2=$?
     if [ ${rc2} -eq 0 ]
     then
         # NTUP prod.
         Reco_tf.py --inputAODFile ${AOD_File} \
                    --maxEvents '-1' \
                    --outputNTUP_PHYSVALFile ${NTUP_File} \
                    --ignoreErrors True \
                    --validationFlags 'doInDet' \
                    --valid 'True'
         rc3=$?

         # regression test
         art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=summary
         rc4=$?
     fi
fi
echo  "art-result: ${rc2} RDOtoAOD"
echo  "art-result: ${rc3} AODtoNTUP"
echo  "art-result: ${rc4} regression"
