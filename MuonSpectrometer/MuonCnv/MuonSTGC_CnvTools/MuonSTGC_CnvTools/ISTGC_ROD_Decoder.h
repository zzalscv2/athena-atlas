/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ISTGC_ROD_Decoder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef MUONSTGC_CNVTOOLS_IMUONSTGC_ROD_DECODER_H
#define MUONSTGC_CNVTOOLS_IMUONSTGC_ROD_DECODER_H

#include "GaudiKernel/IAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "MuonRDO/STGC_RawDataCollection.h"

namespace Muon
{
  class STGC_RawDataContainer;

  
   /** @class ISTGC_ROD_Decoder, based on the respective class for TGCs.
   *   The interface for AlgTool which decodes an STGC ROB fragment into STGC RDO. 
   *   @author Stelios Angelidakis <sangelid@cern.ch>
   */  

  class ISTGC_ROD_Decoder : virtual public IAlgTool
  {
    public:
      DeclareInterfaceID(Muon::ISTGC_ROD_Decoder, 1, 0);

      /** Convert ROBFragments to RDOs */
      virtual StatusCode fillCollection(const EventContext& ctx,
                                        const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&, 
                                        const std::vector<IdentifierHash>&, 
                                        std::unordered_map<IdentifierHash, std::unique_ptr<STGC_RawDataCollection>>&  ) const = 0;
  };

} // end of namespace

#endif // MUONSTGC_CNVTOOLS_IMUONSTGC_ROD_DECODER_H

