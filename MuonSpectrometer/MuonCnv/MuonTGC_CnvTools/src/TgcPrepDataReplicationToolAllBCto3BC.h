/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGCPREPDATAREPLICATIONTOOLAllBCto3BC_H
#define MUONTGC_CNVTOOLS_TGCPREPDATAREPLICATIONTOOLAllBCto3BC_H

#include "MuonTGC_CnvTools/ITgcPrepDataReplicationTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "CxxUtils/checker_macros.h"

namespace Muon 
{
  class ATLAS_NOT_THREAD_SAFE TgcPrepDataReplicationToolAllBCto3BC 
    : public extends<AthAlgTool, ITgcPrepDataReplicationTool>
  {
    public:
      /** Constructor */
      TgcPrepDataReplicationToolAllBCto3BC(const std::string& t, const std::string& n, const IInterface* p);
      
      /** Destructor */
      virtual ~TgcPrepDataReplicationToolAllBCto3BC()=default;

      virtual StatusCode initialize() override;
      virtual StatusCode replicate() const override;
      StatusCode convertAllBCto3BC() const;
      
/** Make new TgcPrepData */ //Static to avoid code duplication with sister class
      static TgcPrepData* makeTgcPrepData(TgcPrepDataCollection::const_iterator itr, uint16_t bcBitMap);

    private:
      enum {BC_PREVIOUS=0, BC_CURRENT, BC_NEXT, BC_ALL, BC_NUM};

      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::WriteHandleKeyArray<TgcPrepDataContainer> m_3BCKeys;
      SG::ReadHandleKey<TgcPrepDataContainer> m_AllBCKey;

      
      

   }; 
} // end of namespace

#endif // MUONTGC_CNVTOOLS_TGCPREPDATAREPLICATIONTOOLAllBCto3BC_H
