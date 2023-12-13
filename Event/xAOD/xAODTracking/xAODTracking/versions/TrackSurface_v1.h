/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKSURFACE_V1_H
#define XAODTRACKING_VERSIONS_TRACKSURFACE_V1_H

#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"
#include "xAODTracking/TrackingPrimitives.h"
#include <cstdint>
#include <vector>


namespace xAOD
{
  /**
   * @brief TrackSurface for Acts MultiTrajectory and TrackSummary
   **/

  class TrackSurface_v1 : public SG::AuxElement
  {

  public:
    TrackSurface_v1() = default;

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
    const xAOD::SurfaceType& surfaceType() const;
    /**
     * access set SurfaceType
     **/
    void setSurfaceType(const xAOD::SurfaceType& m);
  
    /**
     * @brief retrieve the size of the internal vectors for the data storage
     */
    size_t size() const;
  };
}
#endif
