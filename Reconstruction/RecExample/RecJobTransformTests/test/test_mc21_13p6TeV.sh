#!/bin/sh
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# art-description: Run 3 MC reco
# art-output: log.*
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena

#This is used by the Reco_tf --multithread option. It is set
#on grid nodes, but not on local machines
#Explicitly set it to ensure uniform behaviour between the two.
export ATHENA_CORE_NUMBER=8

Reco_tf.py --AMI=q445 --outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root

RES=$?
echo "art-result: $RES Reco"
