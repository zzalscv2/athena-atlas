#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, 100 GeV Muons, no pileup, TrigFastTrackFinder as an offline algorithm
# art-input: mc15_14TeV:mc15_14TeV.900040.PG_singlemu_Pt100_etaFlatnp0_43.evgen.EVNT.e8185
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

lastref_dir=last_results
dcubeXml=dcube_ART_IDPVMPlots_ITk.xml

geometry=ATLAS-P2-RUN4-03-00-00

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

run "Simulation" \
    Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC15c-SDR-14-05' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
    --geometryVersion "default:${geometry}" \
    --inputEVNTFile ${ArtInFile} \
    --outputHITSFile HITS.root \
    --maxEvents -1 \
    --imf False \
    --detectors Bpipe ITkPixel ITkStrip HGTD

run "Digitization"\
    Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC15c-SDR-14-05 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --geometryVersion "default:${geometry}" \
    --inputHITSFile HITS.root \
    --jobNumber 568 \
    --maxEvents -1 \
    --outputRDOFile RDO.root \
    --preInclude 'HITtoRDO:Campaigns.PhaseIINoPileUp' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --detectors ITkPixel ITkStrip HGTD

run "Reconstruction" \
    Reco_tf.py \
    --CA \
    --inputRDOFile RDO.root \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude \
    --preExec "flags.Tracking.useITkFTF=True;flags.Tracking.doITkFastTracking=True"

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --truthMinPt=1000

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
