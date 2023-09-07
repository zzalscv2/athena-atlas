/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_SURFACEBACKEND_V1_H
#define XAODTRACKING_VERSIONS_SURFACEBACKEND_V1_H

#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"
#include "xAODTracking/TrackingPrimitives.h"
#include <cstdint>
#include <vector>


namespace xAOD
{
  /**
   * @brief Surface Backend for Acts MultiTrajectory
   **/

  class SurfaceBackend_v1 : public SG::AuxElement
  {

  public:
    SurfaceBackend_v1() = default;

    /**
     * access translation parameters as plain vector
     **/
    const std::vector<float> &translation() const;
    /**
     * access set translation parameters from plain vector
     **/
    void setTranslation(const std::vector<float> &m);
    

    /**
     * access translation parameters as plain vector
     **/
    const std::vector<float> &rotation() const;
    /**
     * access set translation parameters from plain vector
     **/
    void setRotation(const std::vector<float> &m);
    

    /**
     * access translation parameters as plain vector
     **/
    const std::vector<float> &boundValues() const;
    /**
     * access set translation parameters from plain vector
     **/
    void setBoundValues(const std::vector<float> &m);
    

    /**
     * access SurfaceType
     **/
    xAOD::SurfaceType SurfaceType() const;
    /**
     * access set SurfaceType
     **/
    void setSurfaceType(xAOD::SurfaceType m);
  
    /**
     * @brief pointers API needed by MTJ
     */
    const xAOD::SurfaceType* SurfaceTypePtr() const;
    xAOD::SurfaceType* SurfaceTypePtr();
  

    /**
     * @brief retrieve the size of the internal vectors for the data storage
     */
    size_t size() const;
  };
}
#endif
