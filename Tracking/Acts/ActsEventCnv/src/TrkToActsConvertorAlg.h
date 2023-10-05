/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/  

#ifndef TRKTOACTSCONVERTORALG_H
#define TRKTOACTSCONVERTORALG_H


#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkTrack/TrackCollection.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackContainerHandle.h"
#include "ActsEvent/FutureTrackContainer.h"


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
  
  

  SG::WriteHandleKey<ActsTrk::future::TrackContainer> m_trackContainerKey {this, "TrackContainerLocation", "ConvertedTrackContainer", "Location of the converted TrackContainer"};
  ActsTrk::MutableTrackContainerHandle<ActsTrk::TrkToActsConvertorAlg> m_trackContainerBackends{this, "", "Converted"};

};
}  // namespace ActsTrk


#endif
