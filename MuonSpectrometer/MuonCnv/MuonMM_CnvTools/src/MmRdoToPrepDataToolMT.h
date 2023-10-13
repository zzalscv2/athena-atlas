/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMmRdoToPrepDataToolMT_H
#define MUONMmRdoToPrepDataToolMT_H

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonRDO/MM_RawDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MMClusterization/IMMClusterBuilderTool.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"

namespace Muon 
{

  class MmRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool>
  {
  public:
    MmRdoToPrepDataToolMT(const std::string&,const std::string&,const IInterface*);
    
    /** default destructor */
    virtual ~MmRdoToPrepDataToolMT()=default;
    
    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    
    /** Decode method - declared in Muon::IMuonRdoToPrepDataTool*/
    virtual StatusCode decode(const EventContext& ctx, std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& selectedIdVect ) const override;
    virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds ) const override;
    virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
    StatusCode processCollection(const EventContext& ctx,
                                 Muon::MMPrepDataContainer* mmPrepDataContainer,
                                 const std::vector<IdentifierHash>& idsToDecode,
                                 const MM_RawDataCollection *rdoColl, 
   				                       std::vector<IdentifierHash>& idWithDataVect) const;

    virtual void printInputRdo(const EventContext& ctx) const override;
    virtual void printPrepData(const EventContext& ctx) const override;
    
  protected:
    
    Muon::MMPrepDataContainer* setupMM_PrepDataContainer(const EventContext& ctx) const;

    const MM_RawDataContainer* getRdoContainer(const EventContext& ctx) const;

    void processRDOContainer(const EventContext& ctx, 
                             Muon::MMPrepDataContainer* mmPrepDataContainer,
                             const std::vector<IdentifierHash>& idsToDecode,
                             std::vector<IdentifierHash>& idWithDataVect ) const;

    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
 
    /// MdtPrepRawData containers
    SG::WriteHandleKey<Muon::MMPrepDataContainer> m_mmPrepDataContainerKey{this, "OutputCollection", "MM_Measurements"};
    SG::ReadHandleKey<MM_RawDataContainer> m_rdoContainerKey{this, "InputCollection", "MMRDO"};

     /// This is the key for the cache for the MM PRD containers, can be empty
    SG::UpdateHandleKey<MMPrepDataCollection_Cache> m_prdContainerCacheKey{this, "PrdCacheKey", 
                                                                           "", "Optional external cache for the MM PRD container"};
    Gaudi::Property<bool> m_merge{this, "MergePrds", true}; 

    ToolHandle<IMMClusterBuilderTool> m_clusterBuilderTool{this,"ClusterBuilderTool","Muon::SimpleMMClusterBuilderTool/SimpleMMClusterBuilderTool"};
    ToolHandle<INSWCalibTool> m_calibTool{this,"NSWCalibTool", ""};

    // charge cut is temporarily disabled for comissioning studies. Should be reenabled at some point. pscholer 13.05.2022
    Gaudi::Property<float> m_singleStripChargeCut {this,"singleStripChargeCut", FLT_MIN /*6241 * 0.4*/}; // 0.4 fC from BB5 cosmics

  }; 
} // end of namespace

#endif 
