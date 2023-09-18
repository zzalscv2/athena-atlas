/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SiSPGNNTrackMaker_H
#define SiSPGNNTrackMaker_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/DataHandle.h"

// data containers
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkTrack/TrackCollection.h"

// Tool handles
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"
#include "InDetRecToolInterfaces/ISeedFitter.h"
#include "TrkFitterInterfaces/ITrackFitter.h"

namespace Trk {
  class ITrackFitter;
}


namespace InDet {
   /**
   * @class InDet::SiSPGNNTrackMaker
   * 
   * @brief InDet::SiSPGNNTrackMaker is an algorithm that uses the GNN-based 
   * track finding tool to reconstruct tracks and the use track fitter to obtain
   * track parameters. It turns a collection of Trk::Tracks.
   * 
   * @author xiangyang.ju@cern.ch
   */
    class SiSPGNNTrackMaker : public AthReentrantAlgorithm {
      public:
      SiSPGNNTrackMaker(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~SiSPGNNTrackMaker() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;
      virtual StatusCode finalize() override;

      /// Make this algorithm clonable.
      virtual bool isClonable() const override { return true; };
      //@}

      MsgStream&    dump     (MsgStream&    out) const;
      std::ostream& dump     (std::ostream& out) const;

      protected:
      /// --------------------
      /// @name Data handles
      /// --------------------
      //@{
      // input containers
      SG::ReadHandleKey<SpacePointContainer> m_SpacePointsPixelKey{
        this, "SpacePointsPixelName", "PixelSpacePoints"};
      SG::ReadHandleKey<SpacePointContainer> m_SpacePointsSCTKey{
        this, "SpacePointsSCTName", "SCT_SpacePoints"};
      //@}

      // output container
      SG::WriteHandleKey<TrackCollection> m_outputTracksKey{
        this, "TracksLocation", "SiSPGNNTracks"};

      /// --------------------
      /// @name Tool handles
      /// --------------------
      //@{
      /// GNN-based track finding tool that produces track candidates
      ToolHandle<IGNNTrackFinder> m_gnnTrackFinder{
        this, "GNNTrackFinder", 
        "InDet::SiGNNTrackFinder/InDetSiGNNTrackFinder", "Track Finder"
      };
      ToolHandle<ISeedFitter> m_seedFitter{
        this, "SeedFitter",
        "InDet::SiSeedFitter/InDetSiSeedFitter", "Seed Fitter"
      };
      /// Track Fitter
      ToolHandle<Trk::ITrackFitter> m_trackFitter {
        this, "TrackFitter", 
        "Trk::GlobalChi2Fitter/InDetTrackFitter", "Track Fitter"
      };
      //@}

      MsgStream&    dumptools(MsgStream&    out) const;
      MsgStream&    dumpevent(MsgStream&    out) const;
    };

    MsgStream&    operator << (MsgStream&   ,const SiSPGNNTrackMaker&);
    std::ostream& operator << (std::ostream&,const SiSPGNNTrackMaker&); 
}

#endif
