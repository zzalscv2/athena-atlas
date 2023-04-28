/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

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

    ATH_CHECK( m_TrackContainerKey.initialize() );

    // Initialize decorators
    m_dec_origin_label = m_TrackContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_TrackContainerKey.key() + "." + m_dec_type_label.key();    
    m_dec_vertex_index = m_TrackContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_barcode = m_TrackContainerKey.key() + "." + m_dec_barcode.key();
    m_dec_parent_barcode = m_TrackContainerKey.key() + "." + m_dec_parent_barcode.key();

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
    SG::ReadHandle<TPC> tracks(m_TrackContainerKey, ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " tracks..." );
    
    // instantiate decorators
    SG::WriteDecorHandle<TPC, int> dec_origin_label(m_dec_origin_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_type_label(m_dec_type_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_vertex_index(m_dec_vertex_index, ctx);
    SG::WriteDecorHandle<TPC, int> dec_barcode(m_dec_barcode, ctx);
    SG::WriteDecorHandle<TPC, int> dec_parent_barcode(m_dec_parent_barcode, ctx);

    // decorate loop
    std::vector<const xAOD::TrackParticle*> tracks_vector(tracks->begin(), tracks->end());
    for ( const auto& track : tracks_vector ) {
      const auto truth = m_trackTruthOriginTool->getTruth(track);
      dec_barcode(*track) = truth ? truth->barcode() : -2;
      dec_parent_barcode(*track) = truth ? m_acc_truthParentBarcode(*truth) : -2;
      dec_origin_label(*track) = truth ? m_acc_truthOriginLabel(*truth) : -2;
      dec_type_label(*track) = truth ? m_acc_truthTypeLabel(*truth) : -2;
      dec_vertex_index(*track) = truth ? m_acc_truthVertexIndex(*truth) : -2;
    }
    return StatusCode::SUCCESS;
  }
}


