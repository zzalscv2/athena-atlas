# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool, addService, addAlgorithm
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags

################################################################################
# Standard BS algorithms
################################################################################
addTool( "MuonCnvExample.MuonReadBSConfig.MdtROD_Decoder",         "MdtROD_Decoder" )
addTool( "MuonCnvExample.MuonReadBSConfig.MdtRawDataProviderTool", "MdtRawDataProviderTool" )
addAlgorithm( "Muon::MdtRawDataProvider",                          "MuonMdtRawDataProvider" )

addTool( "MuonCnvExample.MuonReadBSConfig.RpcROD_Decoder",         "RpcROD_Decoder" )
addTool( "MuonCnvExample.MuonReadBSConfig.RpcRawDataProviderTool", "RpcRawDataProviderTool" )
addAlgorithm( "Muon::RpcRawDataProvider",                          "MuonRpcRawDataProvider" )

addTool( "MuonCnvExample.MuonReadBSConfig.TgcROD_Decoder",         "TgcROD_Decoder" )
addTool( "MuonCnvExample.MuonReadBSConfig.TgcRawDataProviderTool", "TgcRawDataProviderTool" )
addAlgorithm("Muon::TgcRawDataProvider",                           "MuonTgcRawDataProvider" )

if MuonGeometryFlags.hasCSC():
    addTool( "MuonCnvExample.MuonReadBSConfig.CscROD_Decoder",         "CscROD_Decoder" )
    addTool( "MuonCnvExample.MuonReadBSConfig.CscRawDataProviderTool", "CscRawDataProviderTool" )
    addAlgorithm("Muon::CscRawDataProvider",                           "MuonCscRawDataProvider" )


################################################################################
# Tools/algorithms/services from MuonCnvExample.MuonCalibConfig
################################################################################
if MuonGeometryFlags.hasCSC():
    addTool( "MuonCnvExample.MuonCalibConfig.CscCalibTool", "CscCalibTool")
    addTool( "MuonCnvExample.MuonCalibConfig.MdtCalibDbTool", "MdtCalibDbTool")
addService( "MuonCnvExample.MuonCalibConfig.MdtCalibrationDbSvc", "MdtCalibrationDbSvc")
addService( "MuonCnvExample.MuonCalibConfig.MdtCalibrationSvc", "MdtCalibrationSvc")


################################################################################
# Tools/algorithms/services from MuonCnvExample.MuonCnvConfig
################################################################################
if MuonGeometryFlags.hasCSC():
    addTool( "MuonCnvExample.MuonCnvConfig.CscDigitToCscRDOTool", "CscDigitToCscRDOTool" )
    addTool( "MuonCnvExample.MuonCnvConfig.CscDigitToCscRDOTool2", "CscDigitToCscRDOTool2" )
    addTool( "MuonCnvExample.MuonCnvConfig.CscDigitToCscRDOTool4", "CscDigitToCscRDOTool4" )
    addAlgorithm( "MuonCnvExample.MuonCnvConfig.CscDigitToCscRDO", "CscDigitToCscRDO" )

