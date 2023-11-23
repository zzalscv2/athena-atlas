/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#ifndef TAURECTOOLS_TAUTRACKFINDER_H
#define TAURECTOOLS_TAUTRACKFINDER_H

#include "tauRecTools/TauRecToolBase.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////

/** 
 * @brief Associate tracks to the tau candidate.
 * 
 *  The tracks have to pass dedicated quality criteria and have to 
 *  match to a primary vertex consistent with the tau origin.
 * 
 * @author KG Tan <Kong.Guan.Tan@cern.ch>
 * @author Felix Friedrich
 */

class TauTrackFinder : public TauRecToolBase {
public:
    //-------------------------------------------------------------
    //! Constructor and Destructor
    //-------------------------------------------------------------
    TauTrackFinder(const std::string& name);
    ASG_TOOL_CLASS2(TauTrackFinder, TauRecToolBase, ITauToolBase);
    ~TauTrackFinder();

    //-------------------------------------------------------------
    //! Enumerator defining type of tau track
    //-------------------------------------------------------------
    enum TauTrackType
    {
        TauTrackCore  = 0,
        TauTrackWide  = 1,
        TauTrackOther = 2,
        NotTauTrack   = 3
    };

    //-------------------------------------------------------------
    //! Algorithm functions
    //-------------------------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode executeTrackFinder(xAOD::TauJet& pTau, xAOD::TauTrackContainer& tauTrackCon) const override;
    
private:
    //-------------------------------------------------------------
    //! Extrapolate track eta and phi to the calorimeter middle surface
    //-------------------------------------------------------------
    StatusCode extrapolateToCaloSurface(xAOD::TauJet& pTau,
                                        xAOD::TauTrackContainer& tauTrackCon) const;

    TauTrackType tauTrackType( const xAOD::TauJet& tauJet,
			       const xAOD::TrackParticle& trackParticle,
			       const xAOD::Vertex* primaryVertex) const;

    void getTauTracksFromPV( const xAOD::TauJet& tauJet,
			     const std::vector<const xAOD::TrackParticle*>& vecTrackParticles,
			     const xAOD::Vertex* primaryVertex,
			     const bool& useGhostTracks,
			     const xAOD::JetContainer* jetContainer,
			     std::vector<const xAOD::TrackParticle*> &tauTracks,
			     std::vector<const xAOD::TrackParticle*> &wideTracks,
			     std::vector<const xAOD::TrackParticle*> &otherTracks) const;

    // new xAOD version
    void removeOffsideTracksWrtLeadTrk(std::vector<const xAOD::TrackParticle*> &tauTracks,
				       std::vector<const xAOD::TrackParticle*> &wideTracks,
				       std::vector<const xAOD::TrackParticle*> &otherTracks,
				       const xAOD::Vertex* tauOrigin,
				       double maxDeltaZ0) const;

    //-------------------------------------------------------------
    //! Some internally used functions
    //-------------------------------------------------------------
    float getZ0(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex) const;   //xAOD version
    bool  isLargeD0Track(const xAOD::TrackParticle* track) const;   

    //-------------------------------------------------------------
    //! tools
    //-------------------------------------------------------------
    ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtensionTool {this, "ParticleCaloExtensionTool", "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool", "Tool for the extrapolation of charged tracks"};
    ToolHandle<Trk::ITrackSelectorTool> m_trackSelectorTool_tau {this, "TrackSelectorToolTau", "", "Tool for track selection"};
    ToolHandle<Reco::ITrackToVertex> m_trackToVertexTool {this, "TrackToVertexTool", "Reco::TrackToVertex"};
    ToolHandle<Trk::ITrackToVertexIPEstimator> m_trackToVertexIPEstimator {this, "TrackToVertexIPEstimator", ""};
    
    Gaudi::Property<double> m_maxJetDr_tau {this, "MaxJetDrTau", 0.2};
    Gaudi::Property<double> m_maxJetDr_wide {this, "MaxJetDrWide", 0.4};
    Gaudi::Property<bool> m_applyZ0cut {this, "removeTracksOutsideZ0wrtLeadTrk", false};
    Gaudi::Property<float> m_z0maxDelta {this, "maxDeltaZ0wrtLeadTrk", 1000.};
    Gaudi::Property<bool> m_storeInOtherTrks {this, "StoreRemovedCoreWideTracksInOtherTracks", true};
    Gaudi::Property<bool> m_removeDuplicateCoreTracks {this, "removeDuplicateCoreTracks", true};
    Gaudi::Property<bool> m_bypassSelector {this, "BypassSelector", false};
    Gaudi::Property<bool> m_bypassExtrapolator {this, "BypassExtrapolator", false};
    Gaudi::Property<bool> m_useGhostTracks {this, "useGhostTracks", false};
    Gaudi::Property<double> m_ghostTrackDR {this, "ghostTrackDR", 0.25};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackPartInputContainer {this,"Key_trackPartInputContainer", "InDetTrackParticles", "input track particle container key"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_largeD0TracksInputContainer {this,"Key_LargeD0TrackInputContainer", "", "input LRT particle container key"}; //Expecting InDetLargeD0TrackParticles (offline tracks) if using LRT used
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainer {this,"Key_jetContainer", "", "Name of the seed jet container, when using ghost matching"};
    SG::ReadHandleKey<CaloExtensionCollection> m_ParticleCacheKey {this,"tauParticleCache", "ParticleCaloExtension", "Name of the particle measurement extrapolation cache for TauTrackFinder"};
    
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

    std::set<CaloSampling::CaloSample> m_EMSamplings;
    std::set<CaloSampling::CaloSample> m_HadSamplings;
};

#endif // TAURECTOOLS_TAUTRACKFINDER_H
#endif // XAOD_ANALYSIS
