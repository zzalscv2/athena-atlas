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
#include "ActsEvent/MultiTrajectoryHandle.h"


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
  ActsTrk::MutableMultiTrajectoryHandle<ActsTrk::TrkToActsConvertorAlg> m_mtjHandle {this, "MTJKey", "Converted"};
  SG::WriteHandleKey<ActsTrk::ConstMultiTrajectory> m_constMTJKey {this, "ConstMTJKey", "ConvertedMultiTrajectory"};


};
}  // namespace ActsTrk


#endif
