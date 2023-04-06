#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: TrkVertexSeedFinderTools/share/ZScanSeedFinder_test.py
# Author: scott snyder <snyder@bnl.gov>
# Data: Jun, 2019
# Brief: Unit test for ZScanSeedFinder.  Incomplete!
#


include ('TrkVertexSeedFinderTools/VertexSeedFinderTestCommon.py')

from InDetRecExample.TrackingCommon import getTrackToVertexIPEstimator


from TrkVertexSeedFinderTools.TrkVertexSeedFinderToolsConf import \
    Trk__VertexSeedFinderTestAlg, Trk__ZScanSeedFinder
finder = Trk__ZScanSeedFinder ('ZScanSeedFinder',
                               IPEstimator = getTrackToVertexIPEstimator(),
                               OutputLevel = INFO)
testalg1 = Trk__VertexSeedFinderTestAlg ('testalg1',
                                         VertexSeedFinderTool = finder,
                                         Expected1 = [  0,   0, -8.14159],
                                         Expected2 = [1.7, 1.3, -7.82529],
                                         Expected3 = [0, 0, 11.6246])
topSequence += testalg1

