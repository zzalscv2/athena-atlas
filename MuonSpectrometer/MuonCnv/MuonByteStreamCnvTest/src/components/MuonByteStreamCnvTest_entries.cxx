#include "../CscRdoToCscDigit.h"
#include "../MM_DigitToRDO.h"
#include "../MM_RdoToDigit.h"
#include "../MdtRdoToMdtDigit.h"
#include "../RpcRdoToRpcDigit.h"
#include "../STGC_DigitToRDO.h"
#include "../STGC_RdoToDigit.h"
#include "../TgcRdoToTgcDigit.h"
#include "MuonByteStreamCnvTest/CscDigitToCscRDO.h"
#include "MuonByteStreamCnvTest/MdtDigitToMdtRDO.h"
#include "MuonByteStreamCnvTest/MuonRdoToMuonDigit.h"
#include "MuonByteStreamCnvTest/MuonRdoToMuonDigitTool.h"
#include "MuonByteStreamCnvTest/ReadCscDigit.h"
#include "MuonByteStreamCnvTest/ReadMdtDigit.h"
#include "MuonByteStreamCnvTest/ReadMdtPRD.h"
#include "MuonByteStreamCnvTest/ReadMdtRDO.h"
#include "MuonByteStreamCnvTest/ReadRpcDigit.h"
#include "MuonByteStreamCnvTest/ReadRpcRDO.h"
#include "MuonByteStreamCnvTest/ReadTgcDigit.h"
#include "MuonByteStreamCnvTest/ReadTgcRDO.h"
#include "MuonByteStreamCnvTest/RpcDigitToRpcRDO.h"
#include "MuonByteStreamCnvTest/TgcDigitToTgcRDO.h"

DECLARE_COMPONENT(RpcDigitToRpcRDO)
DECLARE_COMPONENT(MdtDigitToMdtRDO)
DECLARE_COMPONENT(TgcDigitToTgcRDO)
DECLARE_COMPONENT(CscDigitToCscRDO)
DECLARE_COMPONENT(STGC_DigitToRDO)
DECLARE_COMPONENT(MM_DigitToRDO)

DECLARE_COMPONENT(ReadRpcRDO)
DECLARE_COMPONENT(ReadMdtRDO)
DECLARE_COMPONENT(ReadTgcRDO)

DECLARE_COMPONENT(ReadRpcDigit)
DECLARE_COMPONENT(ReadMdtDigit)
DECLARE_COMPONENT(ReadTgcDigit)
DECLARE_COMPONENT(ReadCscDigit)
DECLARE_COMPONENT(ReadMdtPRD)
DECLARE_COMPONENT(CscRdoToCscDigit)
DECLARE_COMPONENT(MdtRdoToMdtDigit)
DECLARE_COMPONENT(RpcRdoToRpcDigit)
DECLARE_COMPONENT(TgcRdoToTgcDigit)
DECLARE_COMPONENT(STGC_RdoToDigit)
DECLARE_COMPONENT(MM_RdoToDigit)
DECLARE_COMPONENT(MuonRdoToMuonDigit)
DECLARE_COMPONENT(MuonRdoToMuonDigitTool)
