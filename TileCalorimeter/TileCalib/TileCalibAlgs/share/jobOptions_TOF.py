#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
#**************************************************************

# settings to read ESD file
doSim = False
ReadPool = True
ReadDigits = False
ReadRch = False
doCaloCell = False
doTileMuId = False
doTileMuonFitter = False
doTileTMDBRawChannel = False
useRODReco = False
doTileMon = False
doTileCalib = False
doTileNtuple = False
doCaloNtuple = False
TileUseDCS = True

include("TileRecEx/jobOptions_TileCalibRec.py")

if TilePhysRun:
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    from TileCalibAlgs.TileCalibAlgsConf import TileTopCalibAlg
    from TileCalibAlgs.TileCalibAlgsConf import TileTOFTool

    topCalibAlg = TileTopCalibAlg( "TileTopCalibAlg" )
    topCalibAlg.RunNumber = RunNumber
    topCalibAlg.RunType   = TileRunType
    topCalibAlg.FileName  = '%(dir)s/tileCalibTOF_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    
    TileTOF = TileTOFTool("TileTOF")
    TileTOF.CaloCellContainer = "AllCalo"
    TileTOF.OutputLevel = DEBUG # VERBOSE

    topCalibAlg.TileCalibTools += [TileTOF]
    topSequence += topCalibAlg

    print(topCalibAlg)
