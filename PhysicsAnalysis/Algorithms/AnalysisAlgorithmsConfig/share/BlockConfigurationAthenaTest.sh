#/bin/bash
#
# Copyright (C) 2002-2022  CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


set -e
set -u

dataType=$1
athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType
athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType --block-config


acmd.py diff-root --enforce-leaves -t analysis FullCPAlgorithmsTest.$dataType.hist.root FullCPAlgorithmsConfigTest.$dataType.hist.root
