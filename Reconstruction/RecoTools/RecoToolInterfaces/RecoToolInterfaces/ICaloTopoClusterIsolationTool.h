/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITopoClusterIsolationTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef CALOTOPOCLUSTERISOLATIONTOOLS_ICALOTOPOCLUSTERISOLATIONTOOL_H
#define CALOTOPOCLUSTERISOLATIONTOOLS_ICALOTOPOCLUSTERISOLATIONTOOL_H

#include "AsgTools/AsgTool.h"
#include "xAODPrimitives/IsolationType.h"
#include "xAODBase/IParticle.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "IsolationCommon.h"

namespace xAOD {

  /** @class ICaloTopoClusterIsolationTool
      @brief interface for tools calculating topo cluster isolation
 
      @author Niels van Eldik, Sandrine Laplace
   */

  class ICaloTopoClusterIsolationTool : virtual public asg::IAsgTool {
    ASG_TOOL_INTERFACE( xAOD::ICaloTopoClusterIsolationTool )
  public:

    /**ICaloTopoClusterIsolationTool interface for cluster isolation: 
       The tool expects the cones to be order in decreasing order (topetcone40 -> topoetcone20)
       Internally it reorders the cones so the output isolation values are also in the same order. 
       @param[in] result    output object to be filled
       @param[in] tp        input iparticle
       @param[in] cones     vector of input cones to be used
       @param[in] corrections bitset specifying which corrections to apply to isolation
       @param[in] container topo cluster container (for trigger only)
       @return true if the calculation was successful
    */    
    virtual bool caloTopoClusterIsolation(CaloIsolation& result, const IParticle& tp, 
					  const std::vector<Iso::IsolationType>& cones, 
					  const CaloCorrection& corrections, 
					  const CaloClusterContainer* container = 0 ) const = 0;

  };
  
} // end of namespace

#endif 
