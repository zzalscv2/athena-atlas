# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def DataOverlayPPTest(flags):
    """Test configuration of p-p data overlay"""
    flags.Beam.NumberOfCollisions = 20.

    from LArConfiguration.LArConfigRun2 import LArConfigRun2PileUp
    LArConfigRun2PileUp(flags)

    flags.LAr.OFCShapeFolder = "4samples1phase"
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.correctTime = False
