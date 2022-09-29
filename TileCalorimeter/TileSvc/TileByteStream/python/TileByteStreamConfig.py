# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory

def _createTileContByteStreamToolsConfig (name, TileContByteStreamTool, InitializeForWriting=False, stream=None, **kwargs):

    kwargs['name'] = name
    kwargs['InitializeForWriting'] = InitializeForWriting
    tool = TileContByteStreamTool(**kwargs)

    if InitializeForWriting:
        from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlg
        TileHid2RESrcIDCondAlg(ForHLT=True)

        if stream:
            stream.ExtraInputs += [('TileHid2RESrcID', 'ConditionStore+TileHid2RESrcIDHLT')]

    return tool

def TileRawChannelContByteStreamToolConfig(name='TileRawChannelContByteStreamTool', InitializeForWriting=False, stream=None, **kwargs):
    from TileByteStream.TileByteStreamConf import TileRawChannelContByteStreamTool
    return _createTileContByteStreamToolsConfig(name, TileRawChannelContByteStreamTool, InitializeForWriting, stream, **kwargs)

def TileL2ContByteStreamToolConfig(name='TileL2ContByteStreamTool', InitializeForWriting=False, stream=None, **kwargs):
    from TileByteStream.TileByteStreamConf import TileL2ContByteStreamTool
    return _createTileContByteStreamToolsConfig(name, TileL2ContByteStreamTool, InitializeForWriting, stream, **kwargs)



# ComponentAccumulator version

def _createTileContByteStreamToolCfg (flags, name, InitializeForWriting=False, **kwargs):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    TileContByteStreamTool = CompFactory.getComp(name)
    tool = TileContByteStreamTool(name, **kwargs)
    tool.InitializeForWriting = InitializeForWriting
    acc.addPublicTool(tool)

    extraOutputs = []
    if InitializeForWriting:
        from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
        acc.merge( TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True) )

        extraOutputs = [('TileHid2RESrcID', 'ConditionStore+TileHid2RESrcIDHLT')]

    return acc, extraOutputs


def TileDigitsContByteStreamToolCfg (flags,
                                     name="TileDigitsContByteStreamTool",
                                     InitializeForWriting=False,
                                     **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileRawChannelContByteStreamToolCfg (flags,
                                         name="TileRawChannelContByteStreamTool",
                                         InitializeForWriting=False,
                                         **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileMuRcvContByteStreamToolCfg (flags,
                                    name="TileMuRcvContByteStreamTool",
                                    InitializeForWriting=False,
                                    **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileL2ContByteStreamToolCfg (flags,
                                 name="TileL2ContByteStreamTool",
                                 InitializeForWriting=False,
                                 **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileLaserObjByteStreamToolCfg (flags,
                                   name="TileLaserObjByteStreamTool",
                                   InitializeForWriting=False,
                                   **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)
