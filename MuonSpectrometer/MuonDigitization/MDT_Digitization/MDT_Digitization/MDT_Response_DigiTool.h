/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_DIGITIZATION_MDT_RESPONSE_DIGITOOL_H
#define MDT_DIGITIZATION_MDT_RESPONSE_DIGITOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MDT_Digitization/IMDT_DigitizationTool.h"
#include "MDT_Digitization/MdtDigiToolOutput.h"
#include "MDT_Response/MDT_Response.h"
/*-----------------------------------------------

   Created 7-5-2004 by Niels van Eldik

 Digitization tool which uses MDT_Response to convert MDT digitization
 input quantities into the output

-----------------------------------------------*/

namespace MuonGM {
    class MuonDetectorManager;
}
class MdtIdHelper;

class MDT_Response_DigiTool : public AthAlgTool, virtual public IMDT_DigitizationTool {
public:
    MDT_Response_DigiTool(const std::string& type, const std::string& name, const IInterface* parent);

    MdtDigiToolOutput digitize(const MdtDigiToolInput& input, CLHEP::HepRandomEngine* rndmEngine);

    StatusCode initialize();

    bool initializeTube(const MuonGM::MuonDetectorManager* detMgr);

private:
    MDT_Response m_tube;

    Gaudi::Property<double> m_clusterDensity{this, "ClusterDensity" , 8.5};
    Gaudi::Property<double> m_threshold{this, "Threshold" , 20.};
    Gaudi::Property<double> m_attenuationLength{this, "AttenuationLength", 16000};
    Gaudi::Property<bool> m_DoQballGamma{this, "DoQballGamma", false};

    const MdtIdHelper* m_idHelper;
};

#endif
