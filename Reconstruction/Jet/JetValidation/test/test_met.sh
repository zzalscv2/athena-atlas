#!/bin/sh
# art-description: ART test job HITS to AOD
# art-type: grid
# art-include: master/Athena
# art-memory: 4096
# art-output: *.root
# art-output: dcube
# art-html: dcube

Nevents=3000
echo "Number of test events  : ${Nevents}"

ART_AOD="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13167/AOD.27162646._000001.pool.root.1"
echo "Input AOD file: ${ART_AOD}"

ART_Validation="nightly_met.PHYSVAL.root"
echo "Output Validation file : ${ART_Validation}"

echo "Submitting Reconstruction ..."

Reco_tf.py \
    --inputAODFile=${ART_AOD} \
    --reductionConf PHYSVAL \
    --preExec "default:from AthenaCommon.DetFlags import DetFlags; DetFlags.detdescr.all_setOff(); DetFlags.BField_setOn(); DetFlags.digitize.all_setOff(); DetFlags.detdescr.Calo_setOn(); DetFlags.simulate.all_setOff(); DetFlags.pileup.all_setOff(); DetFlags.overlay.all_setOff(); DetFlags.detdescr.Muon_setOn();" \
    --postExec 'from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence; swapAlgsInSequence(topSequence,"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK", "UFOInfoAlgCSSK" );' \
    --maxEvents=${Nevents} \
    --outputDAODFile pool.root

Reco_tf.py \
    --inputAODFile="DAOD_PHYSVAL.pool.root" \
    --outputNTUP_PHYSVALFile=${ART_Validation} \
    --valid=True \
    --validationFlags 'noExample,doMET'

rc=$?
echo "art-result: $rc Reco"

rc2=-9999
if [ ${rc} -eq 0 ]
then
  art.py download --user=artprod --dst=last_results JetValidation test_met
  # Histogram comparison with DCube
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x dcube \
     -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/JetValidation/DCUBE/met.xml \
     -r last_results/nightly_met.PHYSVAL.root \
     nightly_met.PHYSVAL.root
  rc2=$?
fi
echo "art-result: ${rc2} plot"
