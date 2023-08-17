
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
     ATH_CHECK(detStore()->retrieve(m_detMgr));
     {
        const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
        std::vector<const MdtReadoutElement*> readoutEles = m_detMgr->getAllMdtReadoutElements();
        for(const MdtReadoutElement* readOutMl1 : readoutEles){
            const Identifier moduleID = readOutMl1->identify();
            if (readOutMl1->multilayer() == 2) {
              ATH_MSG_VERBOSE("Element "<<m_idHelperSvc->toStringDetEl(moduleID)<<" has wrong multilayer");
              continue;
            }
            /// Retrieve the second detector element            
            const unsigned int nMl = id_helper.numberOfMultilayers(moduleID);
            const Identifier idMl2 = id_helper.multilayerID(moduleID, nMl);
            const MdtReadoutElement* readOutMl2 = m_detMgr->getMdtReadoutElement(idMl2);
            if (!readOutMl2) readOutMl2 = readOutMl1;
            /// Build the hash for the first tube layer
            const IdentifierHash hashEle1 = readOutMl1->measurementHash(1, 0);
            const IdentifierHash hashEle2 = readOutMl2->measurementHash(readOutMl2->numLayers(), 0);
            m_cenCache.emplace(readOutMl1,hashEle1, readOutMl2, hashEle2);
        } 
     }
     return StatusCode::SUCCESS;
  }
  
  const Amg::Transform3D& MuonStationLayerSurfaceTool::chambCenterToGlobal(const ActsGeometryContext& gctx, 
                                                                          const Identifier& id) const {
        const StationCacheSet::const_iterator cache_itr = m_cenCache.find(id);
        if (cache_itr != m_cenCache.end()){
          return cache_itr->localToGlobal(gctx);
        }
        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" no chamber -> global transformation could be found for "
                        << m_idHelperSvc->toString(id));
        return dummy;
  }
  const Amg::Transform3D& MuonStationLayerSurfaceTool::globalToChambCenter(const ActsGeometryContext& gctx,
                                                      const Identifier& id) const {
      const StationCacheSet::const_iterator cache_itr = m_cenCache.find(id);
      if (cache_itr != m_cenCache.end()){
        return cache_itr->globalToLocal(gctx);
      }
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" no chamber -> global transformation could be found for "
                      << m_idHelperSvc->toString(id));
      return dummy;
  }
  unsigned int MuonStationLayerSurfaceTool::storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const {
     unsigned int n = 0;
     for (const StationCenterCache& cache : m_cenCache) {
        n += cache.storeAlignment(alignStore);
     }
     return n;
  }

}