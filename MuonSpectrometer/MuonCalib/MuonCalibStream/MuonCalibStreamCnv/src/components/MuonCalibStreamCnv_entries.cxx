//#include "MuonCalibStreamCnv/CscRDOContCalibStreamCnv.h"
#include "MuonCalibStreamCnv/EventInfoMuonCalibStreamCnv.h"
// #include "MuonCalibStreamCnv/MdtPRDCollCalibStreamCnv.h"
// #include "MuonCalibStreamCnv/MdtPRDContCalibStreamCnv.h"
// #include "MuonCalibStreamCnv/RpcRDOContCalibStreamCnv.h"
// #include "MuonCalibStreamCnv/TgcRDOContCalibStreamCnv.h"
#include "MuonCalibStreamCnv/MdtCalibRawDataProvider.h"
#include "MuonCalibStreamCnv/RpcCalibRawDataProvider.h"
#include "MuonCalibStreamCnv/TgcCalibRawDataProvider.h"
#include "MuonCalibStreamCnv/EventInfoCalibRawDataProvider.h"
#include "MuonCalibStreamCnv/MuonCalibStreamTestAlg.h"


DECLARE_COMPONENT(MuonCalibStreamTestAlg)
DECLARE_COMPONENT(MdtCalibRawDataProvider)
DECLARE_COMPONENT(RpcCalibRawDataProvider)
DECLARE_COMPONENT(TgcCalibRawDataProvider)
DECLARE_COMPONENT(EventInfoCalibRawDataProvider)
// DECLARE_CONVERTER(MdtPRDCollCalibStreamCnv)
// DECLARE_CONVERTER(MdtPRDContCalibStreamCnv)
// DECLARE_CONVERTER(RpcRDOContCalibStreamCnv)
// DECLARE_CONVERTER(TgcRDOContCalibStreamCnv)
//DECLARE_CONVERTER(CscRDOContCalibStreamCnv)
DECLARE_CONVERTER(EventInfoMuonCalibStreamCnv)
