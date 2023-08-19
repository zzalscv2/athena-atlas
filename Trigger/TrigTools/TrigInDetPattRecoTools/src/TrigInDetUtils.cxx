//utils.cxx
#include "TrigInDetPattRecoTools/TrigInDetUtils.h"

bool FTF::isGoodTrackUTT(const Trk::Track* track, trackInfo& theTrackInfo, const float shift_x, const float shift_y) {
      static constexpr int   TRKCUT_N_HITS_INNER  = 1;
      static constexpr int   TRKCUT_N_HITS_PIX    = 2;
      static constexpr float TRKCUT_A0BEAM        = 2.5;
      static constexpr float TRKCUT_PTGEV_LOOSE   = 3.0;
      static constexpr int   TRKCUT_N_HITS        = 4;

      std::unordered_map<Identifier, int> umap_fittedTrack_identifier;

      if ( track->perigeeParameters()==nullptr ) return false;
      if ( track->trackSummary()==nullptr )  return false;
      theTrackInfo.n_hits_innermost = track->trackSummary()->get(Trk::SummaryType::numberOfInnermostPixelLayerHits);
      float n_hits_next_to_innermost = track->trackSummary()->get(Trk::SummaryType::numberOfNextToInnermostPixelLayerHits);
      theTrackInfo.n_hits_inner = theTrackInfo.n_hits_innermost + n_hits_next_to_innermost;
      theTrackInfo.n_hits_pix   = track->trackSummary()->get(Trk::SummaryType::numberOfPixelHits);
      theTrackInfo.n_hits_sct   = track->trackSummary()->get(Trk::SummaryType::numberOfSCTHits);
      if( theTrackInfo.n_hits_inner < TRKCUT_N_HITS_INNER )      return false;
      if( theTrackInfo.n_hits_pix < TRKCUT_N_HITS_PIX )          return false;
      if( (theTrackInfo.n_hits_pix+theTrackInfo.n_hits_sct) < TRKCUT_N_HITS ) return false;
      theTrackInfo.eta = track->perigeeParameters()->eta();
      theTrackInfo.ptGeV = track->perigeeParameters()->pT()/Gaudi::Units::GeV;
      if( theTrackInfo.ptGeV < TRKCUT_PTGEV_LOOSE ) return false;
      float a0 = track->perigeeParameters()->parameters()[Trk::d0];
      theTrackInfo.phi0 = track->perigeeParameters()->parameters()[Trk::phi0];
      theTrackInfo.a0beam = a0 + shift_x*sin(theTrackInfo.phi0)-shift_y*cos(theTrackInfo.phi0);
      if( std::abs(theTrackInfo.a0beam) > TRKCUT_A0BEAM ) {return false;}
      return true;
}

void FTF::getBeamSpotShift(float& shift_x, float& shift_y, const InDet::BeamSpotData& beamSpotHandle) {
  Amg::Vector3D vertex = beamSpotHandle.beamPos();
  //ATH_MSG_VERBOSE("Beam spot position " << vertex);
  double xVTX = vertex.x();
  double yVTX = vertex.y();
  double zVTX = vertex.z();
  double tiltXZ = beamSpotHandle.beamTilt(0);
  double tiltYZ = beamSpotHandle.beamTilt(1);
  shift_x = xVTX - tiltXZ*zVTX;//correction for tilt
  shift_y = yVTX - tiltYZ*zVTX;//correction for tilt
  //ATH_MSG_VERBOSE("Beam center position:  " << shift_x <<"  "<< shift_y);
}