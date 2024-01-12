# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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


def TileRawDataReadingCfg(flags, readDigits=True, readRawChannel=True,
                          readMuRcv=None, readMuRcvDigits=False, readMuRcvRawCh=False,
                          readBeamElem=None, readLaserObj=None, readDigitsFlx=False,
                          stateless=False, **kwargs):
    """
    Configure reading the Tile BS files

    Arguments:
      read[...] -- flag to read the corresponding Tile data from BS.
                   Possible values: None (default), True, False.
                   In the case of None it will be autoconfigured.
      stateless -- read online Tile data using emon BS service.
    """

    isPhysicsRun = flags.Tile.RunType == 'PHY'
    isLaserRun = flags.Tile.RunType == 'LAS'
    isCalibRun = not isPhysicsRun

    # Set up default data
    readMuRcv = isPhysicsRun if readMuRcv is None else readMuRcv
    readBeamElem = isCalibRun if readBeamElem is None else readBeamElem
    readLaserObj = isLaserRun if readLaserObj is None else readLaserObj

    typeNames = kwargs.get('type_names', [])

    prefix = flags.Overlay.BkgPrefix if flags.Overlay.DataOverlay else ''

    if readDigits:
        typeNames += [f'TileDigitsContainer/{prefix}TileDigitsCnt']
    if readRawChannel:
        typeNames += [f'TileRawChannelContainer/{prefix}TileRawChannelCnt']
    if readMuRcv:
        typeNames += ['TileMuonReceiverContainer/TileMuRcvCnt']
        typeNames += ['SG::AuxVectorBase/TileMuRcvCnt']
    if readMuRcvDigits:
        typeNames += [f'TileDigitsContainer/{prefix}MuRcvDigitsCnt']
    if readMuRcvRawCh:
        typeNames += ['TileRawChannelContainer/MuRcvRawChCnt']
    if readLaserObj:
        typeNames += ['TileLaserObject/TileLaserObj']
    if readBeamElem:
        typeNames += ['TileBeamElemContainer/TileBeamElemCnt']
    if readDigitsFlx:
        typeNames += ['TileDigitsContainer/TileDigitsFlxCnt']

    cfg = ComponentAccumulator()
    if stateless:
        from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
        cfg.merge( EmonByteStreamCfg(flags, type_names=typeNames) )
    else:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge( ByteStreamReadCfg(flags, type_names=typeNames) )
        cfg.getService("ByteStreamCnvSvc").ROD2ROBmap = ["-1"]

    if not flags.Common.isOnline:
        cfg.addPublicTool(CompFactory.TileROD_Decoder())

    return cfg
