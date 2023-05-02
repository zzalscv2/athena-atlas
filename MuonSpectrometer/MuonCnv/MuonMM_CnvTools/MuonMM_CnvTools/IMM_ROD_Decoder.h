/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IMM_ROD_Decoder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef MUONMM_CNVTOOLS_IMUONMM_ROD_DECODER_H
#define MUONMM_CNVTOOLS_IMUONMM_ROD_DECODER_H

#include "GaudiKernel/IAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "MuonRDO/MM_RawDataCollection.h"

namespace Muon
{
  class MM_RawDataContainer;

  
   /** @class IMM_ROD_Decoder, based on the respective class for TGCs.
   *   The interface for AlgTool which decodes an MM ROB fragment into MM RDO. 
   *   @author Stelios Angelidakis <sangelid@cern.ch>
   */  

  class IMM_ROD_Decoder : virtual public IAlgTool
  {
    public:
      DeclareInterfaceID(Muon::IMM_ROD_Decoder, 1, 0);

      /** Convert ROBFragments to RDOs */
      virtual StatusCode fillCollection(const EventContext&, const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&, const std::vector<IdentifierHash>&, std::unordered_map<IdentifierHash, std::unique_ptr<MM_RawDataCollection>>&) const = 0;
  };

} // end of namespace

#endif // MUONMM_CNVTOOLS_IMUONMM_ROD_DECODER_H

