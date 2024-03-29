#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

def DataQualityToolsConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaMonitoring.DQConfigFlags import DQDataType
    from .DQTLumiMonAlg import DQTLumiMonAlgConfig
    from .DQTBackgroundMon import DQTBackgroundMonAlgConfig
    from .DQTDetSynchMonAlg import DQTDetSynchMonAlgConfig
    from .DQTGlobalWZFinderAlg import DQTGlobalWZFinderAlgConfig

    result = ComponentAccumulator()

    # the following should not run in RAW to ESD, if we're in two-step
    if flags.DQ.Environment != 'tier0Raw':
        if flags.DQ.DataType is not DQDataType.Cosmics:
            result.merge(DQTLumiMonAlgConfig(flags))
            result.merge(DQTGlobalWZFinderAlgConfig(flags))

    # only when input is RAW
    if flags.DQ.Environment in ('online', 'tier0', 'tier0Raw'):
        result.merge(DQTDetSynchMonAlgConfig(flags))
        result.merge(DQTBackgroundMonAlgConfig(flags))

    return result
