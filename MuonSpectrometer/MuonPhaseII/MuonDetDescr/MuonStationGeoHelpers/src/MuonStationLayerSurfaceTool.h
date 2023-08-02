/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONLAYERSURFACESVC_MuonStationLayerSurfaceSvc_H
#define MUONSTATIONLAYERSURFACESVC_MuonStationLayerSurfaceSvc_H

#include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>
#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonStationGeoHelpers/StationCenterCache.h>

#include <set>
#include <memory>

namespace MuonGMR4{
  
    class MuonStationLayerSurfaceTool: public AthAlgTool,  virtual public IMuonStationLayerSurfaceTool {        
        
        public:
          /** @brief Standard tool constructor **/
          MuonStationLayerSurfaceTool( const std::string& type, 
                                      const std::string& name, 
                                      const IInterface* parent );


         virtual ~MuonStationLayerSurfaceTool() = default;

         StatusCode initialize() override final;
      
          /// Returns the local -> global transformation for a given measurement Identifier
          const Amg::Transform3D& chambCenterToGlobal(const ActsGeometryContext& gctx, 
                                                      const Identifier& id) const override final;
        
          const Amg::Transform3D& globalToChambCenter(const ActsGeometryContext& gctx,
                                                      const Identifier& id) const override final;

          unsigned int storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const override;
      private:
          const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                              "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

          StationCacheSet m_cenCache{};
          
         
};  
}
#endif
