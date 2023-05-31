# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm, addTool

addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getMdtRdoToMdtDigitAlg", "MdtRdoToMdtDigitAlg")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getRpcRdoToRpcDigitAlg", "RpcRdoToRpcDigitAlg")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getNrpcDigitToNrpcRDO", "NrpcRdoToNrpcDigitAlg")

addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getTgcRdoToTgcDigitAlg", "TgcRdoToTgcDigitAlg")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getSTGC_RdoToDigitAlg", "STGC_RdoToDigitAlg")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getMM_RdoToDigitAlg", "MM_RdoToDigitAlg")

addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getMdtDigitToMdtRDO" , "MdtDigitToMdtRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getRpcDigitToRpcRDO" , "RpcDigitToRpcRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getNrpcDigitToNrpcRDO" , "NrpcDigitToNrpcRDO")

addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getTgcDigitToTgcRDO" , "TgcDigitToTgcRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getSTGC_DigitToRDO" , "STGC_DigitToRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getMM_DigitToRDO" , "MM_DigitToRDO")

addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getSigMdtDigitToMdtRDO" , "SigMdtDigitToMdtRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getSigRpcDigitToRpcRDO" , "SigRpcDigitToRpcRDO")
addAlgorithm("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getSigTgcDigitToTgcRDO" , "SigTgcDigitToTgcRDO")

addTool("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getCscRdoDecoder", "CscRDO_Decoder")
addTool("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getsTgcRdoDecoder", "STGC_RDO_Decoder")
addTool("MuonByteStreamCnvTest.MuonByteStreamCnvTestConfigLegacy.getMmRdoDecoder", "MM_RDO_Decoder")
