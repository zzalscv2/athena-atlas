/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SimpleSTgcClusterBuilderTool_h
#define SimpleSTgcClusterBuilderTool_h

#include "STgcClusterization/ISTgcClusterBuilderTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MuonCondData/NswErrorCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>
#include <string>

//
// Simple clusterization tool for STgc
//
namespace Muon
{
  
  class ISTgcClusterBuilderTool;

  class SimpleSTgcClusterBuilderTool : virtual public ISTgcClusterBuilderTool, public AthAlgTool {

  public:

    /** Default constructor */
    SimpleSTgcClusterBuilderTool(const std::string&, const std::string&, const IInterface*);
    
    /** Default destructor */
    virtual ~SimpleSTgcClusterBuilderTool()=default;

    /** standard initialize method */
    virtual StatusCode initialize() override;

    StatusCode getClusters(const EventContext& ctx,
                           std::vector<Muon::sTgcPrepData>&& stripsVect, 
			                     std::vector<std::unique_ptr<Muon::sTgcPrepData>>& clustersVect) const override;

  private: 

    Gaudi::Property<double> m_chargeCut{this, "ChargeCut", 0.};
    Gaudi::Property<unsigned int> m_maxHoleSize{this, "maxHoleSize", 0};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                              "Key of the parametrized NSW uncertainties"};

    /// private functions
    void dumpStrips( std::vector<Muon::sTgcPrepData>& stripsVect,
		     std::vector<Muon::sTgcPrepData*>& clustersVect )const;
  };
}
#endif

