/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTOACTSCONVERTORALG_H
#define TRKTOACTSCONVERTORALG_H


#include "ActsTrkEventCnv/IActsToTrkConverterTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkTrack/TrackCollection.h"
// #include "xAODTracking/TrackMeasurement.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsTrkEvent/MultiTrajectory.h"

namespace ActsTrk {
/** Algorithm convert Trk::Track to ACTS multistate objects
 */
class TrkToActsConvertorAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 protected:
  ToolHandle<IActsToTrkConverterTool> m_convertorTool{this, "ConvertorTool",
                                                      ""};
  SG::ReadHandleKeyArray<TrackCollection> m_trackCollectionKeys{
      this,
      "TrackCollectionKeys",
      {"CombinedInDetTracks", "CombinedMuonTracks", "MuonSpectrometerTracks"},
      "Keys for Track Containers"};
  
  SG::WriteHandleKey<Acts::ConstVectorTrackContainer> m_vectorTrackContainer {this, "VectorTrackContainerLocation", "ConvertedVectorTrackContainer", "Location of the converted VectorTrackContainer"};
  SG::WriteHandleKey<xAOD::TrackStateContainer> m_trackStatesKey {this, "TrackStatesLocation", "ConvertedTrackStates", "Location of the converted TrackStates"}; 
  SG::WriteHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey {this, "TrackJacobiansLocation", "ConvertedTrackJacobians", "Location of the converted TrackJacobians"};
  SG::WriteHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey {this, "TrackMeasurementsLocation", "ConvertedTrackMeasurements", "Location of the converted TrackMeasurements"};
  SG::WriteHandleKey<xAOD::TrackParametersContainer> m_parametersKey {this, "TrackParametersLocation", "ConvertedTrackParameters", "Location of the converted TrackParameters"};


};
}  // namespace ActsTrk


#endif
