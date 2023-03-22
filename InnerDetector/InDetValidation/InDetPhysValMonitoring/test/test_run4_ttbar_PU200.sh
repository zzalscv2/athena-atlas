#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup
# art-type: grid
# art-include: master/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

lastref_dir=last_results
dcubeXml=dcube_ART_IDPVMPlots_ITk.xml
rdo_23p0=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ATLAS-P2-RUN4-01-01-00_ttbar_mu200.RDO.root

geometry=ATLAS-P2-RUN4-01-01-00

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

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
    Reco_tf.py --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --HSFlag All

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
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.root \
    idpvm.root

