/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DenseEnvironmentsAmbiguityProcessorTool_H
#define DenseEnvironmentsAmbiguityProcessorTool_H

#include "AmbiguityProcessorBase.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/IAmbiTrackSelectionTool.h"
#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h"
#include "TrkValInterfaces/ITrkObserverTool.h"

#include "TrkToolInterfaces/IPRDtoTrackMapTool.h"
#include "TrkEventUtils/PRDtoTrackMap.h"

//need to include the following, since its a typedef and can't be forward declared.
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/TrackSeedMap.h"
#include "TrkParameters/TrackParameters.h"
//
#include "AmbiCounter.h"
//
#include <map>
#include <vector>




namespace Trk {
  class ITruthToTrack;
  //
  class DenseEnvironmentsAmbiguityProcessorTool final : public AmbiguityProcessorBase{
  public:
  // default methods
  DenseEnvironmentsAmbiguityProcessorTool(const std::string&,const std::string&,const IInterface*);
  virtual ~DenseEnvironmentsAmbiguityProcessorTool ();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize  () override;
  void dumpStat(MsgStream &out) const;


  /**Returns a processed TrackCollection from the passed 'tracks'
  @param tracks collection of tracks which will have ambiguities resolved. Will not be
  modified.
  The tracks will be refitted if no fitQuality is given at input.
  @return new collections of tracks, with ambiguities resolved. Ownership is passed on
  (i.e. client handles deletion)*/
  virtual const TrackCollection*  process(const TracksScores *trackScoreTrackMap) const override;

  virtual const TrackCollection*  process(const TrackCollection*,Trk::PRDtoTrackMap *) const override {return nullptr;};

  /** statistics output to be called by algorithm during finalize. */
  virtual void statistics() override;

  private:
    void solveTracks(const TracksScores& trackScoreTrackMap,
                     Trk::PRDtoTrackMap &prd_to_track_map,
                     TrackCollection &finalTracks,
                     std::vector<std::unique_ptr<const Trk::Track> >& trackDustbin,
                     Counter &stat) const;


    /** refit PRDs */
    virtual Track*
    refitPrds( const Track* track, Trk::PRDtoTrackMap &prd_to_track_map,
    Counter &stat) const override final;

    virtual std::unique_ptr<Trk::Track>
    doBremRefit(const Trk::Track & track) const override final;


    std::unique_ptr<Trk::Track>
    fit(const std::vector<const Trk::PrepRawData*> &raw,
          const TrackParameters &param, bool flag, Trk::ParticleHypothesis hypo) const;

    std::unique_ptr<Trk::Track>
    fit(const std::vector<const Trk::MeasurementBase*> &measurements,
          const TrackParameters &param, bool flag, Trk::ParticleHypothesis hypo) const;

    std::unique_ptr<Trk::Track>
    fit(const Track &track, bool flag, Trk::ParticleHypothesis hypo) const override final;
    bool
    checkTrack(const Trk::Track *) const;

    /** refitting tool - used to refit tracks once shared hits are removed.
        Refitting tool used is configured via jobOptions.*/
    ToolHandleArray<ITrackFitter> m_fitterTool;

    ToolHandle<Trk::IPRDtoTrackMapTool>         m_assoTool
       {this, "AssociationTool", "InDet::InDetPRDtoTrackMapToolGangedPixels" };

    /** key for the PRDtoTrackMap to filled by the ambiguity score processor.**/
    SG::ReadHandleKey<Trk::PRDtoTrackMap>  m_assoMapName
       {this,"AssociationMapName",""};  ///< the key given to the newly created association map

    /** selection tool - here the decision which hits remain on a track and
        which are removed are made */
    ToolHandle<IAmbiTrackSelectionTool> m_selectionTool
      {this, "SelectionTool", "InDet::InDetDenseEnvAmbiTrackSelectionTool/InDetAmbiTrackSelectionTool"};

    /**Observer tool      This tool is used to observe the tracks and their 'score' */
    PublicToolHandle<Trk::ITrkObserverTool> m_observerToolWriter{this, "ObserverToolWriter", "", "track observer writer within ambiguity solver"};

    bool m_rejectInvalidTracks{};
    /// If enabled, this flag will make the tool restore the hole information from the input track after a refit.
    /// This is used when we want to use holes from the pattern recognition instead of repeating the hole search
    /// Off by default
    BooleanProperty m_keepHolesFromBeforeFit{this,"KeepHolesFromBeforeRefit",false,"Restore hole information from input tracks after refit"};
  };

  inline std::unique_ptr<Trk::Track>
  DenseEnvironmentsAmbiguityProcessorTool::fit(const std::vector<const Trk::PrepRawData*> &raw,
                                                           const TrackParameters &param, bool flag,
                                                           Trk::ParticleHypothesis hypo) const {
     std::unique_ptr<Trk::Track> newTrack;
     for ( const ToolHandle<ITrackFitter> &thisFitter : m_fitterTool) {
          newTrack=(thisFitter->fit(Gaudi::Hive::currentContext(),raw, param, flag,hypo));
          if (Trk::DenseEnvironmentsAmbiguityProcessorTool::checkTrack(newTrack.get())) {
                      return newTrack;
          }
          ATH_MSG_WARNING( "The track fitter, " <<  thisFitter->name() << ", produced a track with an invalid covariance matrix." );
     }
     ATH_MSG_WARNING( "None of the " <<  m_fitterTool.size() << " track fitter(s) produced a track with a valid covariance matrix." );
     if (m_rejectInvalidTracks) {
         newTrack.reset(nullptr);
     }
     return newTrack;
  }

  inline std::unique_ptr<Trk::Track>
  DenseEnvironmentsAmbiguityProcessorTool::fit(const std::vector<const Trk::MeasurementBase*> &measurements,
                                                           const TrackParameters &param,
                                                           bool flag,
                                                           Trk::ParticleHypothesis hypo) const{
    std::unique_ptr<Trk::Track> newTrack;
    for ( const ToolHandle<ITrackFitter> &thisFitter : m_fitterTool) {
      newTrack=thisFitter->fit(Gaudi::Hive::currentContext(),measurements, param, flag, hypo);
      if (Trk::DenseEnvironmentsAmbiguityProcessorTool::checkTrack(newTrack.get())) {
        return newTrack;
      }
      ATH_MSG_WARNING( "The track fitter, " <<  thisFitter->name() << ", produced a track with an invalid covariance matrix." );
    }
    ATH_MSG_WARNING( "None of the " <<  m_fitterTool.size() << " track fitter(s) produced a track with a valid covariance matrix." );
    if (m_rejectInvalidTracks) {
      newTrack.reset(nullptr);
    }
    return newTrack;
  }

  inline std::unique_ptr<Trk::Track>
  DenseEnvironmentsAmbiguityProcessorTool::fit(const Track &track, bool flag, Trk::ParticleHypothesis hypo) const{
    std::unique_ptr<Trk::Track> newTrack;
    for ( const ToolHandle<ITrackFitter> &thisFitter : m_fitterTool) { //note: there is only ever one fitter anyway
      newTrack=(thisFitter->fit(Gaudi::Hive::currentContext(),track,flag, hypo));
      if (Trk::DenseEnvironmentsAmbiguityProcessorTool::checkTrack(newTrack.get())) {
         return newTrack;
      }
      ATH_MSG_WARNING( "The track fitter, " <<  thisFitter->name() << ", produced a track with an invalid covariance matrix." );
      //TODO: potential memory leakage
    }
    ATH_MSG_WARNING( "None of the " <<  m_fitterTool.size() << " track fitter(s) produced a track with a valid covariance matrix." );
    if (m_rejectInvalidTracks) {
      newTrack.reset(nullptr);
    }
    return newTrack;
  }

} //end ns


#endif // TrackAmbiguityProcessorTool_H

