#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# File: AthExStoreGateExample/share/HandleTest_jobOptions.py
# Author: Frank Winklmeier
# Date: July, 2022
# Brief: Test for DecorHandleKey depending on a regular handle key
#

import AthenaCommon.AtlasUnixGeneratorJob
theApp.EvtMax = 1

from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

from AthExStoreGateExample.AthExStoreGateExampleConf import (
   AthEx__HandleTestAlg, AthEx__HandleTestTool3 )

topSeq += AthEx__HandleTestAlg ('testalg',
                                Tool1 = AthEx__HandleTestTool3 ('testool',
                                                                RHKey="myrcont",
                                                                RDecorKey="myrdecor",
                                                                WHKey="mywcont",
                                                                WDecorKey="mywdecor"))
