/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "BeamSpotReweightingAlg.h"

// Athena headers
#include "GeneratorObjects/McEventCollection.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"

// CLHEP includes
#include "CLHEP/Vector/LorentzVector.h"

namespace Simulation
{

  BeamSpotReweightingAlg::BeamSpotReweightingAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : AthReentrantAlgorithm( name, pSvcLocator ),
      m_inputMcEventCollection("TruthEvent")
  {
    declareProperty("InputMcEventCollection", m_inputMcEventCollection,
                    "The name of the input McEventCollection");
    declareProperty( "Input_beam_sigma_z", m_input_beam_sigma_z, "Beam spot sigma of the input HIT file to be reweighted");
  }

  /** Athena algorithm's interface method initialize() */
  StatusCode BeamSpotReweightingAlg::initialize()
  {
    // This will check that the properties were initialized
    // properly by job configuration.
    ATH_CHECK( m_inputMcEventCollection.initialize() );
    ATH_CHECK( m_BeamSpotWeight.initialize() );
    ATH_CHECK( m_beamSpotKey.initialize() );
    ATH_CHECK( m_EventInfo.initialize() );
    
    ATH_MSG_INFO("Input_beam_sigma_z="<<m_input_beam_sigma_z);

    return StatusCode::SUCCESS;
  }

  StatusCode BeamSpotReweightingAlg::execute(const EventContext& ctx) const
  {
    float weight=1;
    if(m_input_beam_sigma_z<0) {
      ATH_MSG_DEBUG("Input_beam_sigma_z="<<m_input_beam_sigma_z<<"<0, do nothing");
      return StatusCode::SUCCESS;
    }

    // Construct handles from the keys.
    SG::ReadHandle<McEventCollection> h_inputMcEventCollection (m_inputMcEventCollection, ctx);
    if(!h_inputMcEventCollection.isValid()) {
      ATH_MSG_FATAL("No input McEventCollection called " << h_inputMcEventCollection.name() << " in StoreGate.");
      return StatusCode::FAILURE;
    }

    // loop over the event in the mc collection
    for (const auto& currentGenEvent : *h_inputMcEventCollection) {
      // skip empty events
      if ( !currentGenEvent ) {
        //the hard scatter event is always the first one. If not present, do nothing
        ATH_MSG_WARNING("No first event found in the McEventCollection input collection, use default weight="<<weight);
        break;
      }  
      HepMC::GenVertex *signalVertex = GetSignalProcessVertex(*currentGenEvent);
      
      if(signalVertex) {
        //now calculate weight
        weight=1.0;
        float z=signalVertex->position().z();
        SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
        float beamz=beamSpotHandle->beamPos().z();
        float newsigmaz=beamSpotHandle->beamSigma(2);
        float pullold=(z-beamz)/m_input_beam_sigma_z;
        float pullnew=(z-beamz)/newsigmaz;
        
        ATH_MSG_INFO("Beamspot z="<<beamz<<", z="<<z<<"; old sigma_z="<<m_input_beam_sigma_z<<", old pull="<<pullold<<"; new sigma_z="<<newsigmaz<<", new pull="<<pullnew);
        
        //Use Gauss probability ratio to calculate the weight:
        //weight=TMath::Gaus(z,0,35,true)/TMath::Gaus(z,0,42,true);
        weight=m_input_beam_sigma_z/newsigmaz * std::exp(-pullnew*pullnew/2) / std::exp(-pullold*pullold/2);
      } else {
        //set weight to 1
        ATH_MSG_WARNING("No signal vertex found, use default weight="<<weight);
      }  

      ATH_MSG_INFO("Beam weight="<<weight);
      break;
    }

    SG::WriteDecorHandle<xAOD::EventInfo,float> BeamSpotWeight(m_BeamSpotWeight,ctx);
    if (!BeamSpotWeight.isPresent()) {
      ATH_MSG_ERROR( "BeamSpotWeight.isPresent check fails" );
      return StatusCode::FAILURE;
    }
    if (!BeamSpotWeight.isAvailable()) {
      BeamSpotWeight(0) = weight;
    }
    
    SG::ReadHandle<xAOD::EventInfo> evt (m_EventInfo,ctx);
    if (!evt.isValid()) {
      ATH_MSG_FATAL( "Could not find event info"  );
      return(StatusCode::FAILURE);
    }
    ATH_MSG_INFO( "EventInfo event: " << evt->eventNumber() 
                          << " run: " << evt->runNumber()
                          << " beamSpotWeight: "<< evt->beamSpotWeight()  );

    return StatusCode::SUCCESS;
  }

  HepMC::GenVertex* BeamSpotReweightingAlg::GetSignalProcessVertex(const HepMC::GenEvent& ge) const
  {
    //Ensure that we have a valid signal_process_vertex
#ifdef HEPMC3
    if( !HepMC::signal_process_vertex(ge) ) {
      if (!ge.vertices_empty()) {
        ATH_MSG_DEBUG("No signal_process_vertex found - using the first GenVertex in the event.");
        HepMC::GenVertexPtr signalVertex = ge.vertices().front();
        return signalVertex;
      }
      if( !HepMC::signal_process_vertex(ge) ) { // Insanity check
        if (!ge.vertices().empty()) {
          ATH_MSG_ERROR("Failed to set signal_process_vertex for GenEvent!!");
          return nullptr;
        }
        ATH_MSG_WARNING("No signal_process_vertex found. Empty GenEvent!");
        return nullptr;
      }
    }
    else {
      ATH_MSG_DEBUG("signal_process_vertex set by Generator.");
      return HepMC::signal_process_vertex(ge);
    }
#else    
    if( !ge.signal_process_vertex() ) {
      if (!ge.vertices_empty()) {
        ATH_MSG_DEBUG("No signal_process_vertex found - using the first GenVertex in the event.");
        HepMC::GenVertex *signalVertex = *(ge.vertices_begin());
        return signalVertex;
      }
      if( !ge.signal_process_vertex() ) { // Insanity check
        if (!ge.vertices_empty()) {
          ATH_MSG_ERROR("Failed to set signal_process_vertex for GenEvent!!");
          return nullptr;
        }
        ATH_MSG_WARNING("No signal_process_vertex found. Empty GenEvent!");
        return nullptr;
      }
    }
    else {
      ATH_MSG_DEBUG("signal_process_vertex set by Generator.");
      return ge.signal_process_vertex();
    }
#endif
    return nullptr;
  }

} // namespace Simulation
