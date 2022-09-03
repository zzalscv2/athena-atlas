#!/bin/bash
#
# art-description: Reco_tf.py q449, RAWtoALL in MT and AODtoDAOD in MP
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
  --AMI q449 \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00429782.physics_Main.daq.RAW._lb0562._SFO-11._0001.data \
  --sharedWriter True \
  --steering doRAWtoALL \
  --outputDAODFile art.pool.root \
  --reductionConf PHYS PHYSLITE \
  --athenaopts "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10; from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence;swapAlgsInSequence(topSequence,"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK", "UFOInfoAlgCSSK" );' \
  --maxEvents 2000

rc1=$?
echo "art-result: ${rc1} Reco_tf_q449_phys_physlite_mt_mp" 

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

echo "============ checkxAOD tmp.AOD"
checkxAOD tmp.AOD
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

# dcube references
echo "============ dcube references"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449/v0/hist_physlite_2304.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449/v0/dcube_config_hist_physlite_2304.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc4=$?
echo "art-result: ${rc4} dcube_physlite" 
