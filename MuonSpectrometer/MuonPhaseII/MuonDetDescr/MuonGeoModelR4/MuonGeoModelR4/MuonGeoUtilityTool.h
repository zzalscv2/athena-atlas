/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_MUONGEOUTILITYTOOL_H
#define MUONGEOMODELR4_MUONGEOUTILITYTOOL_H

#include <CxxUtils/checker_macros.h>
#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <GeoModelUtilities/GeoModelTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>


namespace MuonGMR4 {

class MuonGeoUtilityTool final : virtual public IMuonGeoUtilityTool, public AthAlgTool {

   public:
    // Constructor
    MuonGeoUtilityTool(const std::string &type, const std::string &name,
                     const IInterface *parent);

    /// tool hook
    StatusCode initialize() override final;
    // Destructor
    virtual ~MuonGeoUtilityTool() override final;

    const GeoShape* extractShape(const PVConstLink& physVol) const override final;
    const GeoShape* extractShape(const GeoShape* inShape) const override final;
    
    // Navigates through the bolume to find the shifts / rotations etc.
    Amg::Transform3D extractShifts(const PVConstLink& physVol) const override final;
    Amg::Transform3D extractShifts(const GeoShape* inShape) const override final;

    
    std::string dump(const Amg::Transform3D& transform) const override final;
    /// Dumps the shape to string
    std::string dumpShape(const GeoShape* inShape) const override final;

    std::string dumpVolume(const PVConstLink& physVol) const override final;


   private:
    std::string dumpVolume(const PVConstLink& physVol, const std::string& childDelim) const;


    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};
}  // namespace MuonGMR4
#endif
