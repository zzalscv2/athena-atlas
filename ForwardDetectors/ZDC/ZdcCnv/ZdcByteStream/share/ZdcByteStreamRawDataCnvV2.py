from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as geoFlags

if (geoFlags.Run()=="RUN3"):
    from ZdcByteStream.ZdcByteStreamConf import ZdcByteStreamLucrodData
    job += ZdcByteStreamLucrodData( "ZdcByteStreamLucrodData" )

if (geoFlags.Run()=="RUN2"):
    from ZdcByteStream.ZdcByteStreamConf import ZdcByteStreamRawDataV2
    job += ZdcByteStreamRawDataV2( "ZdcByteStreamRawDataV2" )

job.ZdcByteStreamTester = DEBUG
