/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONLAYERSURFACESVC_IACTSMUONCHAMBERTOOL_H
#define MUONSTATIONLAYERSURFACESVC_IACTSMUONCHAMBERTOOL_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <MuonStationGeoHelpers/MuonChamber.h>

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/INamedInterface.h>

namespace MuonGMR4 {
    /** Interface of the IActMuonChamberTool. 
     *      The tool loops over the list of all MuonReadoutElements and groups them into
     *      chambers
    */
    class IActsMuonChamberTool: virtual public IAlgTool {
        public:
            virtual ~IActsMuonChamberTool() = default;

            DeclareInterfaceID(IActsMuonChamberTool, 1, 0);

            virtual ChamberSet buildChambers() const = 0;
    };


}
#endif
