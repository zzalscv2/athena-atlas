#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, 100 GeV Muons, no pileup, TrigFastTrackFinder as an offline algorithm
# art-type: grid
# art-include: master/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

lastref_dir=last_results
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_ITk_FTF.xml"
rdo_23p0=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/mc15_14TeV.900040.PG_singlemu_Pt100_etaFlatnp0_43.recon.RDO.e8185_s3856_r13824/RDO.29831688._000001.pool.root.1

geometry=ATLAS-P2-RUN4-01-00-00

run () {
    name="${1}"
    cmd="${@:2}"
    ############
    echo "Running ${name}..."
    time ${cmd}
    rc=$?
    echo "art-result: $rc ${name}"
    return $rc
}

run "Reconstruction" \
    Reco_tf.py \
    --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude \
    --postInclude TrigFastTrackFinder.ITkFastTrackFinderStandaloneConfig.ITkFastTrackFinderStandaloneCfg \

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doHitLevelPlots \
    --doExpertPlots \
    --truthMinPt=1000 \
    --validateExtraTrackCollections 'TrigFastTrackFinder_FTF_'

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    -c ${dcubeXml} \
    -r ${lastref_dir}/idpvm.root \
    idpvm.root