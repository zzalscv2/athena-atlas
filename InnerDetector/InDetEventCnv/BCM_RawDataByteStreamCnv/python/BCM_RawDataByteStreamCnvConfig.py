# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BCM_RawDataProviderAlgCfg(flags):
    result=ComponentAccumulator()
    #Default properties are fine ... 
    result.addEventAlgo(CompFactory.BCM_RawDataProvider())
    return result

