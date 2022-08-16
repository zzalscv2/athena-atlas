/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "../MDT_RawDataProviderToolMT.h"
#include "../MdtCsmContByteStreamTool.h"
#include "../MdtRDO_Decoder.h"
#include "../MdtROD_Decoder.h"
#include "../MdtRdoToPrepDataToolMT.h"

DECLARE_COMPONENT(Muon::MdtRdoToPrepDataToolMT)
DECLARE_COMPONENT(Muon::MdtCsmContByteStreamTool)
DECLARE_COMPONENT(Muon::MDT_RawDataProviderToolMT)
DECLARE_COMPONENT(Muon::MdtRDO_Decoder)
DECLARE_COMPONENT(MdtROD_Decoder)
