#include "../MuonCacheCreator.h"
#include "MuonByteStream/CscRawDataProvider.h"
#include "MuonByteStream/CscRdoContByteStreamCnv.h"
#include "MuonByteStream/MM_RawDataProvider.h"
#include "MuonByteStream/MdtCsmContByteStreamCnv.h"
#include "MuonByteStream/MdtRawDataProvider.h"
#include "MuonByteStream/RpcPadContByteStreamCnv.h"
#include "MuonByteStream/RpcRawDataProvider.h"
#include "MuonByteStream/TgcRawDataProvider.h"
#include "MuonByteStream/TgcRdoContByteStreamCnv.h"
#include "MuonByteStream/sTgcRawDataProvider.h"
#include "MuonByteStream/sTgcPadTriggerRawDataProvider.h"

DECLARE_COMPONENT(Muon::MdtRawDataProvider)
DECLARE_COMPONENT(Muon::RpcRawDataProvider)
DECLARE_COMPONENT(Muon::TgcRawDataProvider)
DECLARE_COMPONENT(Muon::CscRawDataProvider)
DECLARE_COMPONENT(Muon::sTgcRawDataProvider)
DECLARE_COMPONENT(Muon::sTgcPadTriggerRawDataProvider)
DECLARE_COMPONENT(Muon::MM_RawDataProvider)
DECLARE_CONVERTER(MdtCsmContByteStreamCnv)
DECLARE_CONVERTER(CscRdoContByteStreamCnv)
DECLARE_CONVERTER(RpcPadContByteStreamCnv)
DECLARE_CONVERTER(TgcRdoContByteStreamCnv)

DECLARE_COMPONENT(MuonCacheCreator)
