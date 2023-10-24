#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using 21.0-compatible geometry and FullG4MT_QS
# art-type: build
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC23-SDR-RUN3-01
export TRF_ECHO=1
Sim_tf.py \
    --CA True \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "Hits.pool.root" \
    --maxEvents 2

echo  "art-result: $? simulation"
