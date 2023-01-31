#!/bin/sh

# art-description: Run FastChain with Simulation (ATLFAST3F_G4MS) and Track-Overlay in one job for MC20a, ttbar
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: config.txt
# art-output: RAWtoALL_config.txt
# art-output: RDOtoRDOTrigger_config.txt
# art-architecture: '#x86_64-intel'

events=25
EVNT_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO.26811885._035498_TrackOverlay_Run2.pool.root.1"
RDO_File="TrackOverlay.RDO.pool.root"
AOD_File="TrackOverlay.AOD.pool.root"
NTUP_File="TrackOverlay.NTUP.pool.root"


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
  --skipSecondaryEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --conditionsTag OFLCOND-MC16-SDR-RUN2-09  \
  --geometryVersion ATLAS-R2-2016-01-00-01 \
  --preInclude 'all:Campaigns/MC16SimulationNoIoV.py,Campaigns/MC20a.py' \
  --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' 'from IOVDbSvc.CondDB import conddb;conddb.addOverride("/TILE/OFL02/CALIB/SFR","TileOfl02CalibSfr-SIM-05")' \
  --preExec "from OverlayCommonAlgs.OverlayFlags import overlayFlags;overlayFlags.doTrackOverlay=True;" \
  --DataRunNumber '284500' \
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
