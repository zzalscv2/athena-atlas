#!/bin/sh
#
# art-description: Run simulation using ISF with the ATLFASTIIF simulator, reading minbias events, writing HITS, using 2015 geometry and conditions
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: master/Athena
# art-type: grid
# art-output: test.HITS.pool.root
# art-output: truth.root

Sim_tf.py \
--conditionsTag 'default:OFLCOND-RUN12-SDR-19' \
--physicsList 'FTFP_BERT' \
--truthStrategy 'MC15aPlus' \
--simulator 'ATLFASTIIF' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' 'G4AtlasTests/postInclude.DCubeTest.py' \
--DataRunNumber '222525' \
--geometryVersion 'default:ATLAS-R2-2015-03-01-00' \
--inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.119994.Pythia8_A2MSTW2008LO_minbias_inelastic.evgen.EVNT.e3099.01517253._000001.pool.root.1" \
--outputHITSFile "test.HITS.pool.root" \
--maxEvents 2000 \
--imf False

echo  "art-result: $? simulation"

ArtPackage=$1
ArtJobName=$2
# TODO This is a regression test I think. We would also need to compare these files to fixed references
art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}

echo  "art-result: $? regression"

#TODO Add Digi and Reco steps
