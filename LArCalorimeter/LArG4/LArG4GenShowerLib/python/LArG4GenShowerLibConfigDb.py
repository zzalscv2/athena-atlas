# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm, addTool

addTool("LArG4GenShowerLib.LArG4GenShowerLibConfig.getTestActionShowerLib", "TestActionShowerLib")
addAlgorithm("LArG4GenShowerLib.LArG4GenShowerLibConfig.getLArG4GenShowerLib", "LArG4GenShowerLib")
