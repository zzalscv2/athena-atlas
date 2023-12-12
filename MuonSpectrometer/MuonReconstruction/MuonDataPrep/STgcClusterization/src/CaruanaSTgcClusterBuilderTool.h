/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CaruanaSTgcClusterBuilderTool_h
#define CaruanaSTgcClusterBuilderTool_h

#include "STgcClusterization/ISTgcClusterBuilderTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCondData/NswErrorCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>
#include <string>
#include <set>

//
// Simple clusterization tool for STgc
//
namespace Muon
{

  class ISTgcClusterBuilderTool;

  class CaruanaSTgcClusterBuilderTool : virtual public ISTgcClusterBuilderTool, public AthAlgTool {

  public:

    /** Default constructor */
    CaruanaSTgcClusterBuilderTool(const std::string&, const std::string&, const IInterface*);

    /** Default destructor */
    virtual ~CaruanaSTgcClusterBuilderTool()=default;

    /** standard initialize method */
    virtual StatusCode initialize();

    StatusCode getClusters(const EventContext& ctx,
                           std::vector<Muon::sTgcPrepData>&& stripsVect,
			                     std::vector<std::unique_ptr<Muon::sTgcPrepData>>& clustersVect)const;

  private:

    Gaudi::Property<double> m_chargeCut{this, "ChargeCut", 0.};
    Gaudi::Property<unsigned int> m_maxHoleSize{this, "maxHoleSize", 0};
    // The resolution parameters were obtained from tests with cosmic muons.
    // These values are also used in the Digitization.
    Gaudi::Property<double> m_positionStripResolution{this,"positionStripResolution", 0.0949};
    Gaudi::Property<double> m_angularStripResolution{this, "angularStripResolution", 0.305};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"};
    
    SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                              "Key of the parametrized NSW uncertainties"};

    /// private functions
    void dumpStrips( std::vector<Muon::sTgcPrepData>& stripsVect,
		     std::vector<Muon::sTgcPrepData*>& clustersVect )const;

  };
}
#endif
