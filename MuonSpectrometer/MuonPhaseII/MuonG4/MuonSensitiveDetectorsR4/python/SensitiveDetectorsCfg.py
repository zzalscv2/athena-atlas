# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []
    result.setPrivateTools(tools)
    return result