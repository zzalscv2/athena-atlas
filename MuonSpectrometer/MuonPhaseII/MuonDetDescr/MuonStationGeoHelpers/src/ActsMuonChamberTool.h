/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONLAYERSURFACESVC_ACTSMUONCHAMBERTOOL_H
#define MUONSTATIONLAYERSURFACESVC_ACTSMUONCHAMBERTOOL_H

#include <MuonStationGeoHelpers/IActsMuonChamberTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>


namespace MuonGMR4{
    class ActsMuonChamberTool: public AthAlgTool, public IActsMuonChamberTool {
    
    public:
        /** @brief Standard tool constructor **/
        ActsMuonChamberTool( const std::string& type, const std::string& name, const IInterface* parent );

        virtual ~ActsMuonChamberTool() = default;

        StatusCode initialize() override final;

        ChamberSet buildChambers() const override final;
    
    private:
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                    "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };

    };

}
#endif