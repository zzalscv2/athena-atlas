#!/bin/bash
#
# art-description: Reco_tf.py q449 RAWtoALL in MT and AODtoDAOD in MP with CA
# art-type: grid
# art-include: master/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
  --AMI q449 \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data \
  --steering doRAWtoALL \
  --outputAODFile myAOD.pool.root \
  --athenaopts "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10; from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence;swapAlgsInSequence(topSequence,"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK", "UFOInfoAlgCSSK" );' \
  --maxEvents -1

rc1=$?
echo "art-result: ${rc1} Reco_tf_q449_mt" 

Derivation_tf.py \
  --CA \
  --inputAODFile myAOD.pool.root \
  --outputDAODFile art.pool.root \
  --sharedWriter True \
  --multiprocess True \
  --formats PHYS PHYSLITE \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10' \
  --maxEvents -1

rcderiv=$?
echo "art-result: ${rcderiv} Derivation_tf_q449_phys_physlite_mp_ca" 

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

echo "============ checkxAOD myAOD.pool.root"
checkxAOD myAOD.pool.root
echo "============ checkxAOD DAOD_PHYS.art.pool.root"
checkxAOD DAOD_PHYS.art.pool.root
echo "============ checkxAOD DAOD_PHYSLITE.art.pool.root"
checkxAOD DAOD_PHYSLITE.art.pool.root
rc2=$?
echo "art-result: ${rc2} checkxAOD" 

echo "============ xAODHist DAOD_PHYSLITE.art.pool.root"
xAODHist.py --analysis --outputHISTFile hist_physlite_latest.root DAOD_PHYSLITE.art.pool.root 
rc3=$?
echo "art-result: ${rc3} xAODHist DAOD_PHYSLITE.art.pool.root" 

echo "============ xAODHistSize DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,myAOD.pool.root"
xAODHistSize.py --outputHISTFile hist_physlite_latest.root --xAODFiles DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,myAOD.pool.root
rc4=$?
echo "art-result: ${rc4} xAODHistSize" 

# dcube references
echo "============ dcube references"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449_ca/v0/hist_physlite_2310.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449_ca/v0/dcube_config_hist_physlite_2310.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc5=$?
echo "art-result: ${rc5} dcube_physlite" 
