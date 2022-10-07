#!/bin/bash
#
# art-description: Reco_tf.py q445, HITtoRDO/RDOtoRDOTrigger/RAWtoALL in MT and AODtoDAOD in MP
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
  --AMI q445 \
  --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8357_e7400_s3775/HITS.27679639._074687.pool.root.1 \
  --sharedWriter True \
  --steering 'doRDO_TRIG' 'doTRIGtoALL' \
  --outputDAODFile art.pool.root \
  --reductionConf PHYS PHYSLITE \
  --athenaopts "HITtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10; from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence;swapAlgsInSequence(topSequence,"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK", "UFOInfoAlgCSSK" );' \
  --runNumber=410000 \
  --DataRunNumber=410000 \
  --maxEvents 2000

rc1=$?
echo "art-result: ${rc1} Reco_tf_q445_phys_physlite_mt_mp" 

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
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q445/v0/hist_physlite_2305.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q445/v0/dcube_config_hist_physlite_2305.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc4=$?
echo "art-result: ${rc4} dcube_physlite" 
