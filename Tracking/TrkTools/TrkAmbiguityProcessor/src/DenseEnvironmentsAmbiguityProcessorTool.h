/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DenseEnvironmentsAmbiguityProcessorTool_H
#define DenseEnvironmentsAmbiguityProcessorTool_H

#include "AmbiguityProcessorBase.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/IAmbiTrackSelectionTool.h"
#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h"
#include "TrkAmbiguityProcessor/dRMap.h"
#include "TrkCaloClusterROI/CaloClusterROI_Collection.h"

#include "TrkToolInterfaces/IPRDtoTrackMapTool.h"
#include "TrkEventUtils/PRDtoTrackMap.h"

//need to include the following, since its a typedef and can't be forward declared.
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/TrackSeedMap.h"
#include "TrkParameters/TrackParameters.h"
//
#include "AmbiCounter.icc"
//
#include <map>
#include <vector>




class AtlasDetectorID;
class PixelID;

namespace InDet{
  class IPixelClusterSplitProbTool;
  class PixelCluster;
  class SCT_Cluster;
}

namespace Trk {
  class ITruthToTrack;
  class IExtrapolator;
  //
  class DenseEnvironmentsAmbiguityProcessorTool : public AmbiguityProcessorBase{
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
  virtual TrackCollection*  process(const TracksScores *trackScoreTrackMap) const override;

  virtual TrackCollection*  process(const TrackCollection*,Trk::PRDtoTrackMap *) const override {return nullptr;};

  /** statistics output to be called by algorithm during finalize. */
  virtual void statistics() override;
  
  private:
    void solveTracks(const TracksScores& trackScoreTrackMap,
                     Trk::PRDtoTrackMap &prd_to_track_map,
                     TrackCollection &finalTracks,
                     std::vector<std::unique_ptr<const Trk::Track> >& trackDustbin,
                     Counter &stat) const;


    /** refit PRDs */
    Track* 
    refitPrds( const Track* track, Trk::PRDtoTrackMap &prd_to_track_map,
    Counter &stat) const override final;

    /** refit ROTs corresponding to PRDs*/
    //TODO or Q: new created track, why const
    /**Track* 
    refitRots( const Track* track, Counter &stat) const override final;**/

    /** stores the minimal dist(trk,trk) for covariance correction*/
    void 
    storeTrkDistanceMapdR(TrackCollection& tracks, std::vector<Trk::Track*> &refit_tracks_out );
    
    /** refit Tracks that are in the region of interest and removes inner hits that are wrongly assigned*/
    void 
    removeInnerHits(std::vector<const Trk::MeasurementBase*>& measurements) const;
    
    Trk::Track* 
    refitTracksFromB(const Trk::Track* track,double fitQualityOriginal) const;
   
    /** see if we are in the region of interest for B tracks*/
    bool 
    decideIfInHighPtBROI(const Trk::Track*) const;

    /** Check if the cluster is compatible with a hadronic cluster*/
    bool 
    isHadCaloCompatible(const Trk::TrackParameters& Tp) const;

    /** Load the clusters to see if they are compatibles with ROI*/
    void 
    reloadHadROIs() const;
    
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

    /** variables to decide if we are in a ROI */
    bool m_useHClusSeed;
    float m_minPtBjetROI;
    float m_phiWidth;
    float m_etaWidth;
    SG::ReadHandleKey<CaloClusterROI_Collection> m_inputHadClusterContainerName;

    mutable std::vector<double>   m_hadF;
    mutable std::vector<double>   m_hadE;
    mutable std::vector<double>   m_hadR;
    mutable std::vector<double>   m_hadZ;
    
    /** refitting tool - used to refit tracks once shared hits are removed. 
        Refitting tool used is configured via jobOptions.*/
    ToolHandleArray<ITrackFitter> m_fitterTool;
    /** extrapolator tool - used to refit tracks once shared hits are removed. 
        Extrapolator tool used is configured via jobOptions.*/
    ToolHandle<IExtrapolator> m_extrapolatorTool;

    ToolHandle<Trk::IPRDtoTrackMapTool>         m_assoTool
       {this, "AssociationTool", "InDet::InDetPRDtoTrackMapToolGangedPixels" };

    /** key for the PRDtoTrackMap to filled by the ambiguity score processor.**/
    SG::ReadHandleKey<Trk::PRDtoTrackMap>  m_assoMapName
       {this,"AssociationMapName",""};  ///< the key given to the newly created association map

    /** selection tool - here the decision which hits remain on a track and
        which are removed are made */
    ToolHandle<IAmbiTrackSelectionTool> m_selectionTool;

    /**These allow us to retrieve the helpers*/
    const PixelID* m_pixelId;
    const AtlasDetectorID* m_idHelper;

    SG::WriteHandleKey<InDet::DRMap>                    m_dRMap;      //!< the actual dR map         

    bool m_rejectInvalidTracks;
  };
  
  inline std::unique_ptr<Trk::Track>
  DenseEnvironmentsAmbiguityProcessorTool::fit(const std::vector<const Trk::PrepRawData*> &raw,
                                                           const TrackParameters &param, bool flag,
                                                           Trk::ParticleHypothesis hypo) const {
     std::unique_ptr<Trk::Track> newTrack;
     for ( const ToolHandle<ITrackFitter> &thisFitter : m_fitterTool) {
          newTrack.reset(thisFitter->fit(raw, param, flag,hypo));
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
      newTrack.reset(thisFitter->fit(measurements, param, flag, hypo));
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
      newTrack.reset(thisFitter->fit(track,flag, hypo));
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

