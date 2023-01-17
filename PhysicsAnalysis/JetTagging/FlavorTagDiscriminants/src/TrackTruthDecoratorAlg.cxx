/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"


namespace FlavorTagDiscriminants {

  TrackTruthDecoratorAlg::TrackTruthDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode TrackTruthDecoratorAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Inizializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_TrackContainerKey      );
    ATH_MSG_DEBUG( "    ** " << m_TruthEventsKey      );

    ATH_CHECK( m_TrackContainerKey.initialize() );
    ATH_CHECK( m_TruthEventsKey.initialize() );

    // Prepare decorators
    m_dec_origin_label = m_TrackContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_TrackContainerKey.key() + "." + m_dec_type_label.key();    
    m_dec_vertex_index = m_TrackContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_barcode = m_TrackContainerKey.key() + "." + m_dec_barcode.key();
    m_dec_parent_barcode = m_TrackContainerKey.key() + "." + m_dec_parent_barcode.key();

    // Initialize decorators
    ATH_MSG_DEBUG( "Inizializing decorators:"  );
    ATH_MSG_DEBUG( "    ** " << m_dec_origin_label );
    ATH_MSG_DEBUG( "    ** " << m_dec_type_label );
    ATH_MSG_DEBUG( "    ** " << m_dec_vertex_index );
    ATH_MSG_DEBUG( "    ** " << m_dec_barcode );
    ATH_MSG_DEBUG( "    ** " << m_dec_parent_barcode );

    CHECK( m_dec_origin_label.initialize() );
    CHECK( m_dec_type_label.initialize() );
    CHECK( m_dec_vertex_index.initialize() );
    CHECK( m_dec_barcode.initialize() );
    CHECK( m_dec_parent_barcode.initialize() );
    
    // Retrieve tools
    ATH_CHECK( m_trackTruthOriginTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode TrackTruthDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using TPC = xAOD::TrackParticleContainer;

    // read collections
    SG::ReadHandle<TPC> tracks(m_TrackContainerKey,ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " tracks..." );
    SG::ReadHandle<xAOD::TruthEventContainer> truth_events(m_TruthEventsKey,ctx);
    CHECK( truth_events.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << truth_events->size() << " truth events..." );
    
    // instantiate decorators
    SG::WriteDecorHandle<TPC, int> dec_origin_label(m_dec_origin_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_type_label(m_dec_type_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_vertex_index(m_dec_vertex_index, ctx);
    SG::WriteDecorHandle<TPC, int> dec_barcode(m_dec_barcode, ctx);
    SG::WriteDecorHandle<TPC, int> dec_parent_barcode(m_dec_parent_barcode, ctx);

    // get the truth primary vertex
    const xAOD::TruthVertex* truth_PV = truth_events->at(0)->truthVertex(0);

    // sort the tracks by pt to ensure the vertex clustering is deterministic
    std::vector<const xAOD::TrackParticle*> sorted_tracks;
    for (const auto track : *tracks) { sorted_tracks.push_back(track); }
    std::sort(sorted_tracks.begin(), sorted_tracks.end(), TruthDecoratorHelpers::sort_particles);

    // first loop - decorate origin label, just store truth vertex for now
    auto trk_truth_vertex = std::vector<const xAOD::TruthVertex*>();
    for ( const auto& track : sorted_tracks ) {
      
      // get linked truth particle and decorate bacode
      auto truth = m_trackTruthOriginTool->getTruth(track);
      dec_barcode(*track) = truth ? truth->barcode() : -2;

      // get parent hadron and decorate barcode 
      auto truth_parent = TruthDecoratorHelpers::get_parent_hadron(truth);
      dec_parent_barcode(*track) = truth_parent ? truth_parent->barcode() : -2;

      // store the truth origin of the track
      int trackTruthOrigin = m_trackTruthOriginTool->getTrackOrigin(track);

      // get exclusive track origin
      int trackTruthLabel = InDet::ExclusiveOrigin::getExclusiveOrigin(trackTruthOrigin);
      dec_origin_label(*track) = trackTruthLabel;
      
      // get the truth vertex of the track and store for now
      auto truth_vertex = TruthDecoratorHelpers::get_truth_vertex(truth);
      trk_truth_vertex.push_back(truth_vertex);

      // decorate truth type
      dec_type_label(*track) = TruthDecoratorHelpers::get_truth_type(truth);
    }

    // decorate tracks with truth vertex info
    auto seen_vertices = std::vector<const xAOD::TruthVertex*>();
    for ( size_t i = 0; i != trk_truth_vertex.size(); i++) {
      auto this_vert  = trk_truth_vertex.at(i);
      auto this_track = sorted_tracks.at(i);
      dec_vertex_index(*this_track) = TruthDecoratorHelpers::get_vertex_index(this_vert, truth_PV, seen_vertices, m_truthVertexMergeDistance);
    }
    return StatusCode::SUCCESS;
  }
}


