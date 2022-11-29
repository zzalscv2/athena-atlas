# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
#
# File: share/RootAllocTestReadNoAllocWithAlloc_t.py
# Author: snyder@bnl.gov
# Date: Nov 2022
# Purpose: Testing an xAOD object with a non-standard memory allocator.
#

import ROOT
import cppyy

ROOT.xAOD.TEvent


from AthenaCommon.Include import Include
include = Include(show = False)
include('DataModelRunTests/xAODRootTest.py')

xAODInit()
ana = Analysis('alloctestWithoutAlloc.root')
ana.add (AllocTestRead())
ana.run()
ana.finalize()
