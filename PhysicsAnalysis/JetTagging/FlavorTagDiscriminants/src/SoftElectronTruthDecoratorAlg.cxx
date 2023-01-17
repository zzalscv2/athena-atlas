/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/SoftElectronTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

namespace FlavorTagDiscriminants {

  SoftElectronTruthDecoratorAlg::SoftElectronTruthDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode SoftElectronTruthDecoratorAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Initializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_ElectronContainerKey   );
    ATH_MSG_DEBUG( "    ** " << m_TruthEventsKey         );

    ATH_CHECK( m_ElectronContainerKey.initialize() );
    ATH_CHECK( m_TruthEventsKey.initialize() );

    // Prepare decorators
    m_dec_origin_label = m_ElectronContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_ElectronContainerKey.key() + "." + m_dec_type_label.key();    
    m_dec_vertex_index = m_ElectronContainerKey.key() + "." + m_dec_vertex_index.key();

    // Initialize decorators
    ATH_MSG_DEBUG( "Initializing decorators:"  );
    ATH_MSG_DEBUG( "    ** " << m_dec_origin_label );
    ATH_MSG_DEBUG( "    ** " << m_dec_type_label );
    ATH_MSG_DEBUG( "    ** " << m_dec_vertex_index );

    ATH_CHECK( m_dec_origin_label.initialize() );
    ATH_CHECK( m_dec_type_label.initialize() );
    ATH_CHECK( m_dec_vertex_index.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode SoftElectronTruthDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using EC = xAOD::ElectronContainer;

    // read collections
    SG::ReadHandle<EC> electrons(m_ElectronContainerKey,ctx);
    ATH_CHECK( electrons.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << electrons->size() << " electrons..." );
    SG::ReadHandle<xAOD::TruthEventContainer> truth_events(m_TruthEventsKey,ctx);
    ATH_CHECK( truth_events.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << truth_events->size() << " truth events..." );
    
    // instantiate decorators
    SG::WriteDecorHandle<EC, int> dec_origin_label(m_dec_origin_label, ctx);
    SG::WriteDecorHandle<EC, int> dec_type_label(m_dec_type_label, ctx);
    SG::WriteDecorHandle<EC, int> dec_vertex_index(m_dec_vertex_index, ctx);

    // get the truth primary vertex
    const xAOD::TruthVertex* truth_PV = truth_events->at(0)->truthVertex(0);

    // sort the tracks by pt to ensure the vertex clustering is deterministic
    std::vector<const xAOD::Electron*> sorted_electrons;
    for (const auto electron : *electrons) { sorted_electrons.push_back(electron); }
    std::sort(sorted_electrons.begin(), sorted_electrons.end(), TruthDecoratorHelpers::sort_particles);

    // first loop - decorate origin label, just store truth vertex for now
    auto el_truth_vertex = std::vector<const xAOD::TruthVertex*>();
    const xAOD::TruthParticle *matched_truth = 0;

    for ( const auto& electron : sorted_electrons ) {
      // get the truth origin of the electron
      ElementLink<xAOD::TruthParticleContainer> truth_link = m_truthParticleLink(*electron);
      if (!truth_link || !truth_link.isValid()){
          dec_origin_label(*electron) = 0;
          dec_type_label(*electron) = TruthDecoratorHelpers::get_truth_type(nullptr);
          el_truth_vertex.push_back(nullptr);
      }
      else {
          matched_truth = *truth_link;
          int electron_type = m_classifierParticleType(*matched_truth);

          if (m_valid_types.count(electron_type)){
              int electron_origin = m_trackTruthOriginTool->getTruthOrigin(matched_truth);
              int electronTruthLabel = InDet::ExclusiveOrigin::getExclusiveOrigin(electron_origin);
              dec_origin_label(*electron) = electronTruthLabel;
          }
          else {
              dec_origin_label(*electron) = 1;
          }
          auto truth_vertex = TruthDecoratorHelpers::get_truth_vertex(matched_truth);
          el_truth_vertex.push_back(truth_vertex);
          // decorate truth type
          dec_type_label(*electron) = TruthDecoratorHelpers::get_truth_type(matched_truth);
      }
    }

    // decorate electrons with truth vertex info
    auto seen_vertices = std::vector<const xAOD::TruthVertex*>();
    for ( size_t i = 0; i != el_truth_vertex.size(); i++) {
      auto this_vert  = el_truth_vertex.at(i);
      auto this_electron = sorted_electrons.at(i);
      dec_vertex_index(*this_electron) = TruthDecoratorHelpers::get_vertex_index(this_vert, truth_PV, seen_vertices, m_truthVertexMergeDistance);
    }

    return StatusCode::SUCCESS;
  }
}