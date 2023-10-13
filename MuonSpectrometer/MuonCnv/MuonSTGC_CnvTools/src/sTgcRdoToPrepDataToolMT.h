/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_sTgcRdoToPrepDataToolMT
#define MUONTGC_CNVTOOLS_sTgcRdoToPrepDataToolMT

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonRDO/STGC_RawDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "STgcClusterization/ISTgcClusterBuilderTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"

namespace Muon 
{
  /** @class STGC_RawDataToPrepDataTool 
   *  This is the algorithm that convert STGC Raw data  To STGC PRD  as a tool.
   */  

  class sTgcRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool>
    {
    public:
      /** Constructor */
      sTgcRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p);
      
      /** Destructor */
      virtual ~sTgcRdoToPrepDataToolMT()=default;
      
      /** Standard AthAlgTool initialize method */
      virtual StatusCode initialize() override;
      
      /** Decode RDO to PRD  
       *  A vector of IdentifierHash are passed in, and the data corresponding to this list (i.e. in a Region of Interest) are converted.  
       *  @param requestedIdHashVect          Vector of hashes to convert i.e. the hashes of ROD collections in a 'Region of Interest'  
       *  @return selectedIdHashVect This is the subset of requestedIdVect which were actually found to contain data   
       *  (i.e. if you want you can use this vector of hashes to optimise the retrieval of data in subsequent steps.) */
      
      StatusCode decode(const EventContext& ctx, std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& idWithDataVect) const override;      
      StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;
      StatusCode provideEmptyContainer(const EventContext& ctx) const override;

      
      StatusCode processCollection(const EventContext& ctx, 
                                   Muon::sTgcPrepDataContainer* stgcPrepDataContainer,
                                   const STGC_RawDataCollection *rdoColl, 
                                   std::vector<IdentifierHash>& idWithDataVect) const;

      virtual void printPrepData(const EventContext& ctx) const override;
      virtual void printInputRdo(const EventContext& ctx) const override;
      
    protected:
      
      virtual Muon::sTgcPrepDataContainer* setupSTGC_PrepDataContainer(const EventContext& ctx) const;

      const STGC_RawDataContainer* getRdoContainer(const EventContext& ctx) const;

      void processRDOContainer(const EventContext& ctx,
                               Muon::sTgcPrepDataContainer* stgcPrepDataContainer,
                               const std::vector<IdentifierHash>& idsToDecode,
                               std::vector<IdentifierHash>& idWithDataVect) const;

      SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 

      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    
      SG::ReadHandleKey<STGC_RawDataContainer> m_rdoContainerKey{this, "InputCollection", "sTGCRDO", "RDO container to read"};
      SG::WriteHandleKey<sTgcPrepDataContainer> m_stgcPrepDataContainerKey{this, "OutputCollection", "STGC_Measurements", "Muon::sTgcPrepDataContainer to record"};
      Gaudi::Property<bool> m_merge{this, "Merge", true}; // merge Prds

      ToolHandle<ISTgcClusterBuilderTool> m_clusterBuilderTool{this,"ClusterBuilderTool","Muon::SimpleSTgcClusterBuilderTool/SimpleSTgcClusterBuilderTool"};
      ToolHandle<INSWCalibTool> m_calibTool{this,"NSWCalibTool", ""};

      /// This is the key for the cache for the sTGC PRD containers, can be empty
      SG::UpdateHandleKey<sTgcPrepDataCollection_Cache> m_prdContainerCacheKey{this, "PrdCacheKey", "", "Optional external cache for the sTGC PRD container"};


   }; 
} // end of namespace

#endif // MUONTGC_CNVTOOLS_STGCRDOTOPREPDATATOOL_H
