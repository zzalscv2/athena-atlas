#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
#**************************************************************

TileCorrectTime = False

include("TileRecEx/jobOptions_TileCalibRec.py")

if TileLasRun and doTileFit:
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    from TileCalibAlgs.TileCalibAlgsConf import TileTopCalibAlg
    from TileCalibAlgs.TileCalibAlgsConf import TileLaserTimingTool

    topCalibAlg = TileTopCalibAlg( "TileTopCalibAlg" )
    topCalibAlg.RunNumber = RunNumber
    topCalibAlg.RunType   = TileRunType
    topCalibAlg.FileName  = '%(dir)s/tileCalibLAStime_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }

    LaserTimingTool = TileLaserTimingTool("LaserTimingTool")
    LaserTimingTool.OutputLevel = DEBUG # VERBOSE
    LaserTimingTool.TileRawChannelContainer ="TileRawChannelFit"
    LaserTimingTool.TileDigitsContainer = "TileDigitsCnt"
    LaserTimingTool.NtupleID = "h3000"
    LaserTimingTool.FiberLightSpeed=22.5
    LaserTimingTool.NSamples = TileFrameLength
    LaserTimingTool.EneLowLimitPulseShape = 0.000001

    topCalibAlg.TileCalibTools += [LaserTimingTool]
    topSequence += topCalibAlg

    print(topCalibAlg)
