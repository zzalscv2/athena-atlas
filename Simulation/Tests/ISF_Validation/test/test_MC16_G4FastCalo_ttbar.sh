#!/bin/sh
#
# art-description: MC16-style simulation using G4FastCalo
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root
# art-output: truth.root

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14
Sim_tf.py \
--conditionsTag 'default:OFLCOND-MC16-SDR-14' \
--simulator 'G4FastCalo' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC16Simulation.py' \
--preExec 'EVNTtoHITS:from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags;ISF_FastCaloSimFlags.ParamsInputFilename="FastCaloSim/MC16/TFCSparam_v012.root"' \
--geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
--inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
--outputHITSFile "test.HITS.pool.root" \
--maxEvents 4 \
--imf False

rc=$?
status=$rc
rc2=-9999
echo  "art-result: $rc simOLD"
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed
    rc2=$?
    status=$rc2
fi

echo  "art-result: $rc2 regression"
exit $status
