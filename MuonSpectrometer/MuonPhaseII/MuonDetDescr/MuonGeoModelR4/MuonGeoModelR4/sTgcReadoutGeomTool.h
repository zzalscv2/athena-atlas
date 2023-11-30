/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_STGCREAOUDGEOMTOOL_H
#define MUONGEOMODELR4_STGCREAOUDGEOMTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/CutOutArea.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonGMR4 {

class sTgcReadoutGeomTool : public AthAlgTool,
                           virtual public IMuonReadoutGeomTool {
   public:
    // Constructor
    sTgcReadoutGeomTool(const std::string &type, const std::string &name,
                       const IInterface *parent);

    StatusCode initialize() override final;

    StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc"};

    PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };

    /// Struct to cache the relevant parameters of from the WSTGC tables
    struct wSTGCTable {
  
      //// Strips
      int numStrips{0}; //nStrips
      double stripPitch{0.}; //stripPitch
      double stripWidth{0.}; //stripWidth
      std::vector<double> firstStripPitch; //firstStripWidth

      //// Wires
      std::vector<int> numWires; //numWires
      std::vector<int> firstWireGroupWidth; //firstWireGroupWidth
      std::vector<int> numWireGroups; //numWireGroups
      std::vector<double> wireCutout; //wireCutout
      double wirePitch{0.}; //wirePitch
      double wireWidth{0.}; //wireWidth
      int wireGroupWidth{0}; //wireGroupWidth
      std::vector<double> firstWirePos; //firstWire 


      //// Pads
      std::vector<int> numPadEta; //nPadH
      std::vector<int> numPadPhi; //nPadPhi
      std::vector<double> firstPadHeight; //firstPadH
      std::vector<double> padHeight; //padH

      double gasTck{0.}; //gasTck

    };

    struct FactoryCache {
       
      using ParamBookTable = std::map<std::string, wSTGCTable>;
      using CutOutTable = std::map<Identifier, std::vector<CutOutArea>>;

       std::set<StripDesignPtr, StripDesignSorter> stripDesigns{};
       ParamBookTable parameterBook{};
       CutOutTable cutOuts{};
       
    };

    /// Retrieves the auxillary tables from the database
    StatusCode readParameterBook(FactoryCache& cache);
    /// Loads the chamber dimensions from GeoModel
    StatusCode loadDimensions(sTgcReadoutElement::defineArgs& args, FactoryCache& factory);
    
    IdentifierHash layerHash(const sTgcReadoutElement::defineArgs& args, const int gasGap, const int channelType) const;

};

}  // namespace MuonGMR4
#endif
