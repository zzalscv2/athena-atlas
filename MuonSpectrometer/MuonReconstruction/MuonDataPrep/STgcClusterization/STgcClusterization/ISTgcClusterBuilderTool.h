/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ISTgcClusterBuilderTool_h
#define ISTgcClusterBuilderTool_h

//
// Interface class for STgc clustering
//
#include <vector>
#include "GaudiKernel/IAlgTool.h"

class EventContext;
namespace Muon {
  class sTgcPrepData;
}

namespace Muon {
  class ISTgcClusterBuilderTool : virtual public IAlgTool {
    public:  
      DeclareInterfaceID(Muon::ISTgcClusterBuilderTool, 1 , 0);
    
    //
    // build clusters having as input the hashId of the collection, the
    // resolution of the single channel, and the vector of firing strips
    //
    virtual StatusCode getClusters(const EventContext& ctx,
                                   std::vector<Muon::sTgcPrepData>&&  stripsVect, 
				                           std::vector<std::unique_ptr<Muon::sTgcPrepData>>& clustersVect)const=0;
    
  };
}

#endif
