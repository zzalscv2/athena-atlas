#!/bin/sh
#
# art-description: MC23-style simulation using FullG4_QS (13.6 TeV Zprime input)
# art-type: build
# art-include: 22.0/Athena
# art-include: 22.0/AthSimulation
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation

# RUN3 setup - Frozen Showers currently off by default
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC21-SDR-RUN3-07
Sim_tf.py \
--conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00_VALIDATION' \
--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc23_13p6TeV.800030.Py8EG_A14NNPDF23LO_flatpT_Zprime_Extended.evgen.EVNT.e8514.33116368._004446.pool.root.1' \
--outputHITSFile "test.HITS.pool.root" \
--maxEvents 2

echo  "art-result: $? simulation"
