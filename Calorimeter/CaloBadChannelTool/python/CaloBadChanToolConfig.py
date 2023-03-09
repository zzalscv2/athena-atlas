# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured private Calo bad channels tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloBadChanToolCfg(flags, **kwargs):
    """Return component accumulator with configured private Calo bad channels tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    kwargs.setdefault('name', 'CaloBadChanTool')
    kwargs.setdefault('LArBadChanKey', 'LArBadChannel')

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc = LArBadChannelCfg(flags)

    if 'TileBadChanTool' not in kwargs:
        from TileConditions.TileBadChannelsConfig import TileBadChanToolCfg
        badChannelsTool = acc.popToolsAndMerge( TileBadChanToolCfg(flags) )
        kwargs['TileBadChanTool'] = badChannelsTool

    CaloBadChanTool=CompFactory.CaloBadChanTool
    acc.setPrivateTools(CaloBadChanTool(**kwargs))

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.fillFromArgs()
    flags.lock()

    acc = ComponentAccumulator()

    print( acc.popToolsAndMerge( CaloBadChanToolCfg(flags) ) )
    flags.dump()

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('CaloBadChanTool.pkl','wb') )
