/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MDTREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_MDTREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class MdtReadoutGeomTool : public AthAlgTool,
                           virtual public IMuonReadoutGeomTool {
   public:
    // Constructor
    MdtReadoutGeomTool(const std::string &type, const std::string &name,
                       const IInterface *parent);

    StatusCode initialize() override final;

    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc",
                                              "GeoDbTagSvc"};

    using parameterBook = MdtReadoutElement::parameterBook;
    

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook();
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(MdtReadoutElement::defineArgs& args );

    using ParamBookTable = std::map<std::string, parameterBook>; 
    ParamBookTable m_parBook{};
};

}  // namespace MuonGMR4
#endif
