/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MM_ROD_DECODER_H
#define MUONBYTESTREAM_MM_ROD_DECODER_H

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonMM_CnvTools/IMM_ROD_Decoder.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCablingData/MicroMega_CablingMap.h"

class MmIdHelper;

namespace Muon
{

class MM_RawData;
class MM_RawDataCollection;

class MM_ROD_Decoder : virtual public IMM_ROD_Decoder, public AthAlgTool 
{
  public: 
    MM_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent ) ;
    virtual ~MM_ROD_Decoder() = default;  
    virtual StatusCode initialize() override;
    virtual StatusCode fillCollection(const EventContext&, const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&, const std::vector<IdentifierHash>&, std::unordered_map<IdentifierHash, std::unique_ptr<MM_RawDataCollection>>&) const override;

  protected:
    const MmIdHelper* m_MmIdHelper = nullptr;
    SG::ReadCondHandleKey<MicroMega_CablingMap> m_mmCablingMap{this, "MmCablingMap","MicroMegaCabling","Key of MicroMega_CablingMap"};

  private:

};

}

#endif



