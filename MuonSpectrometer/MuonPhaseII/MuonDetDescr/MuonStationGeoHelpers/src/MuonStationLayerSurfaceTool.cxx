
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStationLayerSurfaceTool.h"
///
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>

namespace {
    const Amg::Transform3D dummy{Amg::Transform3D::Identity()};
}
namespace MuonGMR4{

  MuonStationLayerSurfaceTool::MuonStationLayerSurfaceTool( const std::string& type, 
                                                            const std::string& name, 
                                                            const IInterface* parent ):
        AthAlgTool{type,name, parent} {
    declareInterface<IMuonStationLayerSurfaceTool>(this);
  }
  
  StatusCode MuonStationLayerSurfaceTool::initialize() {
    if (toolSvc() != parent()) {
       ATH_MSG_FATAL("The tool "<<name()<<" must be public. Please use PublicToolHandles<>");
       return StatusCode::FAILURE;
    }
     ATH_CHECK(m_idHelperSvc.retrieve());
     ATH_CHECK(m_chambTool.retrieve());
     m_cenCache = m_chambTool->buildChambers();
     return StatusCode::SUCCESS;
  }
  
  const Amg::Transform3D& MuonStationLayerSurfaceTool::chambCenterToGlobal(const ActsGeometryContext& gctx, 
                                                                          const Identifier& id) const {
        const ChamberSet::const_iterator cache_itr = m_cenCache.find(id);
        if (cache_itr != m_cenCache.end()){
          return cache_itr->localToGlobalTrans(gctx);
        }
        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" no chamber -> global transformation could be found for "
                        << m_idHelperSvc->toString(id));
        return dummy;
  }
  const Amg::Transform3D& MuonStationLayerSurfaceTool::globalToChambCenter(const ActsGeometryContext& gctx,
                                                      const Identifier& id) const {
      const ChamberSet::const_iterator cache_itr = m_cenCache.find(id);
      if (cache_itr != m_cenCache.end()){
        return cache_itr->globalToLocalTrans(gctx);
      }
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" no chamber -> global transformation could be found for "
                      << m_idHelperSvc->toString(id));
      return dummy;
  }
  unsigned int MuonStationLayerSurfaceTool::storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const {
     unsigned int n = 0;
     for (const MuonChamber& cache : m_cenCache) {
        n += cache.storeAlignment(alignStore);
     }
     return n;
  }

}