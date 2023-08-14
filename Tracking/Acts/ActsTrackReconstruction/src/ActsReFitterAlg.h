/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSREFITTERALG_H
#define ACTSGEOMETRY_ACTSREFITTERALG_H

// ATHENA
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"  /*no forward decl: typedef*/
#include "GaudiKernel/ISvcLocator.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// PACKAGE
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

// STL
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>

class EventContext;

namespace ActsTrk {

class ActsReFitterAlg : public AthReentrantAlgorithm {
public:
  ActsReFitterAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

  ToolHandle<Trk::ITrackFitter> m_actsFitter{this, "ActsFitter", "ActsKalmanFitter", "Choice of Acts Fitter (Kalman by default)"};
  // --- job options
  SG::ReadHandleKey<TrackCollection>  m_trackName{this, "TrackName", "Tracks", "Collection name for tracks to be refitted"};
  SG::WriteHandleKey<TrackCollection> m_newTrackName{this, "NewTrackName", "ReFitted_Tracks", "Collection name for output tracks"};

  mutable std::mutex m_writeMutex{};

  //Gaudi Property to choose from PRD or ROT measurment ReFit
  Gaudi::Property<bool> m_doReFitFromPRD{this, "DoReFitFromPRD", false, "Do Refit From PRD instead of ROT"};
};

} // namespace

#endif 
