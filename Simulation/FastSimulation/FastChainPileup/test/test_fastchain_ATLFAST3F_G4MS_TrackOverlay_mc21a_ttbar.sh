#!/bin/sh

# art-description: Run FastChain with Simulation (ATLFAST3F_G4MS) and Track-Overlay in one job without reco for MC21a (RUN3), ttbar
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: config.txt
# art-output: RAWtoALL_config.txt
# art-output: RDOtoRDOTrigger_config.txt
# art-architecture: '#x86_64-intel'

events=50
EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1'
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO.29482411._003778_TrackOverlay.pool.root.1"
RDO_File="TrackOverlay.RDO.pool.root"
AOD_File="TrackOverlay.AOD.pool.root"
NTUP_File="TrackOverlay.NTUP.pool.root"


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
  --skipSecondaryEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --postInclude 'default:PyJobTransforms/UseFrontier.py' \
  --preInclude 'all:Campaigns/MC21a.py,Campaigns/MC21SimulationNoIoV.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' \
  --preExec "from OverlayCommonAlgs.OverlayFlags import overlayFlags;overlayFlags.doTrackOverlay=True;" \
  --imf False

rc=$?
echo "art-result: ${rc} EVNTtoRDOwOverlay"


rc2=999
rc3=999
rc4=999
if [ ${rc} -eq 0 ]
then
    # Reconstruction
    Reco_tf.py --inputRDOFile ${RDO_File} --maxEvents '-1' \
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
         Reco_tf.py --inputAODFile ${AOD_File} --maxEvents '-1' \
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
