#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: TrkVertexSeedFinderTools/share/DummySeedFinder_test.py
# Author: scott snyder <snyder@bnl.gov>
# Data: Jun, 2019
# Brief: Unit test for DummySeedFinder.  Incomplete!
#


include ('TrkVertexSeedFinderTools/VertexSeedFinderTestCommon.py')


from TrkVertexSeedFinderTools.TrkVertexSeedFinderToolsConf import \
    Trk__VertexSeedFinderTestAlg, Trk__DummySeedFinder
finder = Trk__DummySeedFinder ('DummySeedFinder')
testalg1 = Trk__VertexSeedFinderTestAlg ('testalg1',
                                         VertexSeedFinderTool = finder,
                                         Expected1 = [0, 0, 0],
                                         Expected2 = [0, 0, 0],
                                         Expected3 = [0, 0, 0])
topSequence += testalg1
