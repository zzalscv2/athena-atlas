# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


#
# Configure BS unpacking for event dumping.
#
def BSReadCfg (flags):

    acc = ComponentAccumulator()

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge (ByteStreamReadCfg (flags))


    ####### Inner detector
    
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import \
        PixelRawDataProviderAlgCfg
    acc.merge (PixelRawDataProviderAlgCfg(flags))
    
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import \
        SCTRawDataProviderCfg
    acc.merge(SCTRawDataProviderCfg(flags))
    
    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import \
        TRTRawDataProviderCfg
    acc.merge(TRTRawDataProviderCfg(flags))

    
    ####### Calorimeters

    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
    acc.merge (LArRawDataReadingCfg (flags))

    from TileByteStream.TileByteStreamConfig import \
        TileDigitsContByteStreamToolCfg, TileMuRcvContByteStreamToolCfg, \
        TileRawChannelContByteStreamToolCfg, TileL2ContByteStreamToolCfg
    acc.merge (TileDigitsContByteStreamToolCfg (flags)[0])
    acc.merge (TileMuRcvContByteStreamToolCfg (flags)[0])
    acc.merge (TileRawChannelContByteStreamToolCfg (flags)[0])
    acc.merge (TileL2ContByteStreamToolCfg (flags)[0])

    acc.merge (ByteStreamReadCfg (flags,
                                  type_names=['TileDigitsContainer/TileDigitsCnt',
                                              'TileDigitsContainer/MuRcvDigitsCnt',
                                              'TileRawChannelContainer/TileRawChannelCnt',
                                              'TileL2Container/TileL2Cnt',
                                              'TileBeamElemContainer/TileBeamElemCnt',
                                              ],
                                  ))


    ####### Muons

    from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
    acc.merge (MuonByteStreamDecodersCfg (flags))


    ####### Forward 

    from BCM_RawDataByteStreamCnv.BCM_RawDataByteStreamCnvConfig import \
        BCM_RawDataProviderAlgCfg
    acc.merge(BCM_RawDataProviderAlgCfg(flags))
    
    acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
    
    from ForwardRec.LucidRecConfig import LucidRecCfg
    acc.merge (LucidRecCfg (flags))

    acc.addEventAlgo (CompFactory.AFP_RawDataProvider ())
    acc.addEventAlgo (CompFactory.ALFA_RawDataProvider ())

    
    ####### Trigger

    from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
    acc.merge (TriggerRecoCfgData (flags))


    return acc
