#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg, ITkPixelReadoutManagerCfg

# test setup
log.setLevel(DEBUG)
flags = initConfigFlags()
flags.Input.Files = []
# test
PixelReadoutManagerAcc = PixelReadoutManagerCfg(flags, name="PixelReadoutManagerTest")
# prevent raise on __del__
PixelReadoutManagerAcc.wasMerged()

# test ITk
ITkPixelReadoutManagerAcc = ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManagerTest")
# prevent raise on __del__
ITkPixelReadoutManagerAcc.wasMerged()
