/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSEXTRAPOLATIONALG_H
#define ACTSGEOMETRY_ACTSEXTRAPOLATIONALG_H

// ATHENA
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"  /*no forward decl: typedef*/
#include "GaudiKernel/ISvcLocator.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// STL
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>


namespace Acts {
  class TrackingGeometry;
  namespace detail {
    struct Step;
  }
}


class IActsMaterialTrackWriterSvc;

class EventContext;
class IAthRNGSvc;
class IActsExtrapolationTool;
class IActsPropStepRootWriterSvc;

class ActsExtrapolationAlg : public AthReentrantAlgorithm {
public:
  ActsExtrapolationAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  ServiceHandle<IActsPropStepRootWriterSvc> m_propStepWriterSvc;
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc;

  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", "ActsExtrapolationTool"};


  // poor-mans Particle Gun is included here right now
  Gaudi::Property<std::vector<double>> m_etaRange{this, "EtaRange", {-3, 3}, "The eta range for particles"};
  Gaudi::Property<std::vector<double>> m_ptRange{this, "PtRange", {0.1, 1000}, "The pt range for particles"};
  Gaudi::Property<size_t> m_nParticlePerEvent{this, "NParticlesPerEvent", 1, "The number of particles per event"};

  // material track writer for the material map validation
  Gaudi::Property<bool> m_writeMaterialTracks{this, "WriteMaterialTracks", false, "Write material track"};
  Gaudi::Property<bool> m_writePropStep{this, "WritePropStep", false, "Write propagation step"};
  ServiceHandle<IActsMaterialTrackWriterSvc> m_materialTrackWriterSvc;

  // Mutex and members for optional debugging output
  mutable std::mutex m_writeMutex;
  mutable size_t m_objVtxCount ATLAS_THREAD_SAFE {0};

  void writeStepsObj(const std::vector<Acts::detail::Step>& steps) const;

};

#endif // ActsGeometry_ActsExtrapolation_h
