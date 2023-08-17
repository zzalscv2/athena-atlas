/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_RPCREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/CutOutArea.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class RpcReadoutGeomTool : public AthAlgTool,
                           virtual public IMuonReadoutGeomTool {
   public:
    // Constructor
    RpcReadoutGeomTool(const std::string &type, const std::string &name,
                       const IInterface *parent);

    StatusCode initialize() override final;

    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    
    /// Temporarily
    struct parameterBook{
       double stripPitchEta{0.};
       double stripPitchPhi{0.};

       int numEtaStrips{0};
       int numPhiStrips{0};
    };
    
    struct FactoryCache {
       std::set<StripDesignPtr, StripDesignSorter> stripDesigns{};
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook();
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(RpcReadoutElement::defineArgs& args, FactoryCache& factory );
    
    IdentifierHash layerHash(const RpcReadoutElement::defineArgs& args, const int gasGap, const int doubPhi, const bool measPhi) const;

    using ParamBookTable = std::map<std::string, parameterBook>; 
    ParamBookTable m_parBook{};

    std::map<Identifier, std::vector<CutOutArea>> m_amdbCutOuts{};

};

}  // namespace MuonGMR4
#endif
