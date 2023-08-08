/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthEventAuxContainer.h"

#include "xAODTruth/TruthPileupEvent.h"
#include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTruth/TruthPileupEventAuxContainer.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODTruth/TruthVertexAuxContainer.h"

#include "xAODTruth/TruthMetaDataAuxContainer.h"
#include "xAODTruth/TruthMetaData.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "IOVDbDataModel/IOVMetaDataContainer.h"

#include "xAODTruthCnvAlg.h"

bool isSeparatorGenEvent(const HepMC::GenEvent *genEvt) {
  // Separator defined by pid==0 and eventNumber==-1 as per
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/PileupDigitization#Arrangement_of_Truth_Information
  const int pid = HepMC::signal_process_id(genEvt);
  const int eventNumber = genEvt->event_number();
  return (pid==0 && eventNumber==-1);
}

using namespace std;

namespace xAODMaker {
    
    
    xAODTruthCnvAlg::xAODTruthCnvAlg( const string& name, ISvcLocator* svcLoc )
      : AthReentrantAlgorithm( name, svcLoc )
      , m_metaStore( "MetaDataStore", name )
      , m_firstBeginRun(true)
    {
      // leaving metadata alone for now--to be updated
        declareProperty( "MetaObjectName", m_metaName = "TruthMetaData" );
        declareProperty( "MetaDataStore", m_metaStore );
    }
    
    
    StatusCode xAODTruthCnvAlg::initialize() {
        if (m_doAllPileUp && m_doInTimePileUp) {
            ATH_MSG_FATAL( "Contradictory xAOD truth pile-up setting: all pile-up AND in-time alone requested simultaneously. Check settings." );
            return StatusCode::FAILURE;
        }

        if (m_writeMetaData) {
            ATH_CHECK( m_meta.initialize (m_metaStore, m_metaName) );
        }
        
        // initialize handles
        ATH_CHECK(m_truthLinkContainerKey.initialize());

        // only if doing full truth
        ATH_CHECK(m_aodContainerKey.initialize());
        ATH_CHECK(m_xaodTruthEventContainerKey.initialize());
        ATH_CHECK(m_xaodTruthPUEventContainerKey.initialize(m_doAllPileUp || m_doInTimePileUp));
        ATH_CHECK(m_xaodTruthParticleContainerKey.initialize());
        ATH_CHECK(m_xaodTruthVertexContainerKey.initialize());

        ATH_CHECK(m_evtInfo.initialize());

        ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
        ATH_CHECK(incSvc.retrieve());
        incSvc->addListener( this, "BeginRun", 10);

        ATH_MSG_DEBUG("AODContainerName = " << m_aodContainerKey.key() );
        ATH_MSG_DEBUG("xAOD TruthEventContainer name = " << m_xaodTruthEventContainerKey.key() );
        ATH_MSG_DEBUG("xAOD TruthPileupEventContainer name = " << m_xaodTruthPUEventContainerKey.key());
        ATH_MSG_DEBUG("xAOD TruthParticleContainer name = " << m_xaodTruthParticleContainerKey.key() );
        ATH_MSG_DEBUG("xAOD TruthVertexContainer name = " << m_xaodTruthVertexContainerKey.key() );

        if (m_doAllPileUp) ATH_MSG_INFO( "All pile-up truth (including out-of-time) will be written" );
        if (m_doInTimePileUp) ATH_MSG_INFO( "In-time pile-up truth (but not out-of-time) will be written" );
        if (!m_doAllPileUp && !m_doInTimePileUp) ATH_MSG_INFO( "No pile-up truth will be written" );

        return StatusCode::SUCCESS;
    }
    
    
    StatusCode xAODTruthCnvAlg::execute (const EventContext& ctx) const {
        
        SG::WriteHandle<xAODTruthParticleLinkVector> truthLinkVec(m_truthLinkContainerKey, ctx);
        ATH_CHECK(truthLinkVec.record(std::make_unique<xAODTruthParticleLinkVector>()));
        
        // Retrieve the HepMC truth:
        SG::ReadHandle<McEventCollection> mcColl(m_aodContainerKey, ctx);
        // validity check is only really needed for serial running. Remove when MT is only way.
        if (!mcColl.isValid()) {
          ATH_MSG_ERROR("Could not retrieve HepMC with key:" << m_aodContainerKey.key());
          return StatusCode::FAILURE;
        } else {
          ATH_MSG_DEBUG( "Retrieved HepMC with key: " << m_aodContainerKey.key() );
        }

        // **************************************************************
        // Create the xAOD containers and their auxiliary stores:
        // **************************************************************
        // Signal event
        SG::WriteHandle<xAOD::TruthEventContainer> xTruthEventContainer(m_xaodTruthEventContainerKey, ctx);
        ATH_CHECK(xTruthEventContainer.record(std::make_unique<xAOD::TruthEventContainer>(),
                                              std::make_unique<xAOD::TruthEventAuxContainer>()));
        ATH_MSG_DEBUG( "Recorded TruthEventContainer with key: " << m_xaodTruthEventContainerKey.key() );

        // Pile-up events
        SG::WriteHandle<xAOD::TruthPileupEventContainer> xTruthPileupEventContainer;
        if (m_doAllPileUp || m_doInTimePileUp) {
          xTruthPileupEventContainer = SG::WriteHandle<xAOD::TruthPileupEventContainer>(m_xaodTruthPUEventContainerKey, ctx);
          ATH_CHECK(xTruthPileupEventContainer.record(std::make_unique<xAOD::TruthPileupEventContainer>(),
                                                      std::make_unique<xAOD::TruthPileupEventAuxContainer>()));
          ATH_MSG_DEBUG( "Recorded TruthPileupEventContainer with key: " << m_xaodTruthPUEventContainerKey.key() );
        }

        // Particles
        SG::WriteHandle<xAOD::TruthParticleContainer> xTruthParticleContainer(m_xaodTruthParticleContainerKey, ctx);
        ATH_CHECK(xTruthParticleContainer.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                                 std::make_unique<xAOD::TruthParticleAuxContainer>()));
        ATH_MSG_DEBUG( "Recorded TruthParticleContainer with key: " << m_xaodTruthParticleContainerKey.key() );
        // Vertices
        SG::WriteHandle<xAOD::TruthVertexContainer> xTruthVertexContainer(m_xaodTruthVertexContainerKey, ctx);
        ATH_CHECK(xTruthVertexContainer.record(std::make_unique<xAOD::TruthVertexContainer>(),
                                               std::make_unique<xAOD::TruthVertexAuxContainer>()));
        ATH_MSG_DEBUG( "Recorded TruthVertexContainer with key: " << m_xaodTruthVertexContainerKey.key() );
             
        // ***********************************************************************************
        // Create the xAOD objects
        // This consists of three parts:
        // (1)  For each Athena event, loop over the GenEvents and build TruthEvent collections
        //      In principle there can be more than one GenEvent per event
        // (2)  For each GenEvent, loop over the GenParticles. For each GenParticle:
        //      (a) Create a TruthParticle.
        //      (b) Call fillParticle
        //      (c) Add the TruthParticle to the TruthParticle container, and add
        //          an EL to this TruthParticle to the truthParticles in TruthEvent
        //          (call this EL eltp)
        //      (d) For the GenVertex * that's this particle's production vertex,
        //          (i) see if it is in tempMap. If not, add it.
        //          (ii) add a copy of eltp (the EL to this truth particle) to map[gv].second
        //      (e) For the GenVertex * that's this particle's decay vertex,
        //          (i) see if it is in tempMap. If not, add it.
        //          (ii) add a copy of eltp (the EL to this truth particle) to map[gv].first
        // (3) Iterate over tempMap. For each GenVertex *:
        //      (a) Create a TruthVertex
        //      (b) Call fillVertex
        //      (c) Add the TruthVertex to the TruthTruth container, and add an EL to this TruthVertex
        //          to the truthVertices in TruthEvent. (call this EL eltv)
        //      (d) call tv->setIncomingParticles(mapiter.second.first)  <- I think mapiter.second.first is the first of the pair
        //      (e) call tv->setOutgoingParticles(mapiter.second.second)
        //      (f) Iterate through the incomingParticles, and set the decay vertex EL as eltv.
        //      (g) Iterate through the outgoingParticles, and set the incoming vertex EL as eltv.
        //
        // Comment lines below follow this recipe
        // ************************************************************************************
            
        // (1) Build TruthEvents
        ATH_MSG_DEBUG("Number of GenEvents in this Athena event = " << mcColl->size());
#ifdef HEPMC3
        bool newAttributesPresent(false);
#endif
        for (unsigned int cntr = 0; cntr < mcColl->size(); ++cntr) {
          const HepMC::GenEvent* genEvt = (*mcColl)[cntr];
          bool isSignalProcess(false);
          if (cntr==0) {
            isSignalProcess=true;
#ifdef HEPMC3
            auto bunchCrossingTime = genEvt->attribute<HepMC3::IntAttribute>("BunchCrossingTime");
            if (bunchCrossingTime) {
              newAttributesPresent = true;
              ATH_MSG_VERBOSE("New attributes present.");
            }
            else {
              ATH_MSG_VERBOSE("New attributes missing.");
            }
#else
              ATH_MSG_VERBOSE("New attributes missing.");
#endif
          }
          if (cntr>0) {
            // Handle pile-up events
            if (!m_doInTimePileUp && !m_doAllPileUp) break;
            isSignalProcess=false;
#ifdef HEPMC3
            auto bunchCrossingTime = genEvt->attribute<HepMC3::IntAttribute>("BunchCrossingTime");
            if (bunchCrossingTime) {
              // New approach based on checking the bunch crossing
              // time directly.
              if (m_doInTimePileUp && std::abs(bunchCrossingTime->value()) > 0) {
                // Skip out-of-time pile-up events
                continue;
              }
            }
            else {
              // Old approach based on McEventCollection structure. If
              // in-time pileup only is requested, loop stops when the
              // separator GenEvent between out-of-time and in-time is
              // reached
              if (m_doInTimePileUp && isSeparatorGenEvent(genEvt)) {
                if (newAttributesPresent) {
                  // Old structure with new Attributes?! Check all
                  // GenEvents just in case.
                  ATH_MSG_VERBOSE("New-style, but seeing separator GenEvents");
                  continue;
                }
                // Old structure - stop at the first separator
                // GenEvent.
                break;
              }
            }
#else
              // Old approach based on McEventCollection structure. If
              // in-time pileup only is requested, loop stops when the
              // separator GenEvent between out-of-time and in-time is
              // reached
              if (m_doInTimePileUp && isSeparatorGenEvent(genEvt)) {
                // Old structure - stop at the first separator
                // GenEvent.
                break;
              }
#endif
          }
                
          xAOD::TruthEvent* xTruthEvent = new xAOD::TruthEvent();
          xAOD::TruthPileupEvent* xTruthPileupEvent = new xAOD::TruthPileupEvent();
                
                
          if (isSignalProcess) {
            xTruthEventContainer->push_back( xTruthEvent );
            // Cross-section
            auto crossSection = genEvt->cross_section();
#ifdef HEPMC3
            xTruthEvent->setCrossSection(crossSection ? (float)crossSection->xsec() : -1);
            xTruthEvent->setCrossSectionError(crossSection ? (float)crossSection->xsec_err() : -1);
#else
            xTruthEvent->setCrossSection(crossSection ? (float)crossSection->cross_section() : -1);
            xTruthEvent->setCrossSectionError(crossSection ? (float)crossSection->cross_section_error() : -1);
#endif
                    
            if (m_writeMetaData) {
              //The mcChannelNumber is used as a unique identifier for which truth meta data belongs to
              uint32_t mcChannelNumber = 0;
              SG::ReadHandle<xAOD::EventInfo> evtInfo (m_evtInfo,ctx);
              if (evtInfo.isValid()) {
                mcChannelNumber = evtInfo->mcChannelNumber();
                if (mcChannelNumber==0) mcChannelNumber = evtInfo->runNumber();
              }
              else {
                ATH_MSG_FATAL("Faied to retrieve EventInfo");
                return StatusCode::FAILURE;
              }

              ATH_CHECK( m_meta.maybeWrite (mcChannelNumber, *genEvt, m_metaFields) );
            }
            // Event weights
            vector<float> weights;
            for (const double& w : genEvt->weights()) weights.push_back((float)(w));
            //AV This to be decided. It is always a good idea to have a default weight 1.0. 
            //if (weights.empty()) weights.push_back(1.0); 
            xTruthEvent->setWeights(weights);
                    
            // Heavy ion info
            auto const hiInfo = genEvt->heavy_ion();
            if (hiInfo) {
#ifdef HEPMC3
              /* Please note HepMC3 as well as more recent HePMC2 versions   have more Hi parameters */
              xTruthEvent->setHeavyIonParameter(hiInfo->Ncoll_hard, xAOD::TruthEvent::NCOLLHARD);
              xTruthEvent->setHeavyIonParameter(hiInfo->Npart_proj, xAOD::TruthEvent::NPARTPROJ);
              xTruthEvent->setHeavyIonParameter(hiInfo->Npart_targ, xAOD::TruthEvent::NPARTTARG);
              xTruthEvent->setHeavyIonParameter(hiInfo->Ncoll, xAOD::TruthEvent::NCOLL);
              xTruthEvent->setHeavyIonParameter(hiInfo->spectator_neutrons, xAOD::TruthEvent::SPECTATORNEUTRONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->spectator_protons, xAOD::TruthEvent::SPECTATORPROTONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->N_Nwounded_collisions, xAOD::TruthEvent::NNWOUNDEDCOLLISIONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->Nwounded_N_collisions, xAOD::TruthEvent::NWOUNDEDNCOLLISIONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->Nwounded_Nwounded_collisions, xAOD::TruthEvent::NWOUNDEDNWOUNDEDCOLLISIONS);
              xTruthEvent->setHeavyIonParameter((float)hiInfo->impact_parameter, xAOD::TruthEvent::IMPACTPARAMETER);
              xTruthEvent->setHeavyIonParameter((float)hiInfo->event_plane_angle, xAOD::TruthEvent::EVENTPLANEANGLE);
              xTruthEvent->setHeavyIonParameter((float)hiInfo->eccentricity, xAOD::TruthEvent::ECCENTRICITY);
              xTruthEvent->setHeavyIonParameter((float)hiInfo->sigma_inel_NN, xAOD::TruthEvent::SIGMAINELNN);
#else
              xTruthEvent->setHeavyIonParameter(hiInfo->Ncoll_hard(), xAOD::TruthEvent::NCOLLHARD);
              xTruthEvent->setHeavyIonParameter(hiInfo->Npart_proj(), xAOD::TruthEvent::NPARTPROJ);
              xTruthEvent->setHeavyIonParameter(hiInfo->Npart_targ(), xAOD::TruthEvent::NPARTTARG);
              xTruthEvent->setHeavyIonParameter(hiInfo->Ncoll(), xAOD::TruthEvent::NCOLL);
              xTruthEvent->setHeavyIonParameter(hiInfo->spectator_neutrons(), xAOD::TruthEvent::SPECTATORNEUTRONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->spectator_protons(), xAOD::TruthEvent::SPECTATORPROTONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->N_Nwounded_collisions(), xAOD::TruthEvent::NNWOUNDEDCOLLISIONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->Nwounded_N_collisions(), xAOD::TruthEvent::NWOUNDEDNCOLLISIONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->Nwounded_Nwounded_collisions(), xAOD::TruthEvent::NWOUNDEDNWOUNDEDCOLLISIONS);
              xTruthEvent->setHeavyIonParameter(hiInfo->impact_parameter(), xAOD::TruthEvent::IMPACTPARAMETER);
              xTruthEvent->setHeavyIonParameter(hiInfo->event_plane_angle(), xAOD::TruthEvent::EVENTPLANEANGLE);
              xTruthEvent->setHeavyIonParameter(hiInfo->eccentricity(), xAOD::TruthEvent::ECCENTRICITY);
              xTruthEvent->setHeavyIonParameter(hiInfo->sigma_inel_NN(), xAOD::TruthEvent::SIGMAINELNN);
#endif
              // This doesn't yet exist in our version of HepMC
              // xTruthEvent->setHeavyIonParameter(hiInfo->centrality(),xAOD::TruthEvent::CENTRALITY);
            }
                    
            // Parton density info
            // This will exist 99% of the time, except for e.g. cosmic or particle gun simulation
            auto const pdfInfo = genEvt->pdf_info();
            if (pdfInfo) {
#ifdef HEPMC3
              xTruthEvent->setPdfInfoParameter(pdfInfo->parton_id[0], xAOD::TruthEvent::PDGID1);
              xTruthEvent->setPdfInfoParameter(pdfInfo->parton_id[1], xAOD::TruthEvent::PDGID2);
              xTruthEvent->setPdfInfoParameter(pdfInfo->pdf_id[1], xAOD::TruthEvent::PDFID1);
              xTruthEvent->setPdfInfoParameter(pdfInfo->pdf_id[1], xAOD::TruthEvent::PDFID2);
                        
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->x[0], xAOD::TruthEvent::X1);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->x[1], xAOD::TruthEvent::X2);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->scale, xAOD::TruthEvent::Q);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->xf[0], xAOD::TruthEvent::XF1);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->xf[1], xAOD::TruthEvent::XF2);
#else
              xTruthEvent->setPdfInfoParameter(pdfInfo->id1(), xAOD::TruthEvent::PDGID1);
              xTruthEvent->setPdfInfoParameter(pdfInfo->id2(), xAOD::TruthEvent::PDGID2);
              xTruthEvent->setPdfInfoParameter(pdfInfo->pdf_id1(), xAOD::TruthEvent::PDFID1);
              xTruthEvent->setPdfInfoParameter(pdfInfo->pdf_id2(), xAOD::TruthEvent::PDFID2);
                        
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->x1(), xAOD::TruthEvent::X1);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->x2(), xAOD::TruthEvent::X2);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->scalePDF(), xAOD::TruthEvent::Q);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->pdf1(), xAOD::TruthEvent::XF1);
              xTruthEvent->setPdfInfoParameter((float)pdfInfo->pdf2(), xAOD::TruthEvent::XF2);
#endif
            }
          }else{//not isSignalProcess
      xTruthPileupEventContainer->push_back( xTruthPileupEvent );
    }
                
          // (2) Build particles and vertices
          // Map for building associations between particles and vertices
          // The pair in the map is the (incomingParticles . outgoingParticles) of the given vertex
          // If signal process vertex is a disconnected vertex (no incoming/outgoing particles), add it manually
          VertexMap vertexMap;
          VertexMap::iterator mapItr;
          vector<HepMC::ConstGenVertexPtr> vertices;
                
          // Check signal process vertex
          // If this is a disconnected vertex, add it manually or won't be added from the loop over particles below.
           auto disconnectedSignalProcessVtx = HepMC::signal_process_vertex(genEvt); // Get the signal process vertex
          if (disconnectedSignalProcessVtx) {
#ifdef HEPMC3
            if (disconnectedSignalProcessVtx->particles_in().empty() &&
                disconnectedSignalProcessVtx->particles_out().empty() ) {
#else
            if (disconnectedSignalProcessVtx->particles_in_size() == 0 &&
                disconnectedSignalProcessVtx->particles_out_size() == 0 ) {
#endif
              //This is a disconnected vertex, add it manually
              vertices.push_back (disconnectedSignalProcessVtx);
            }
          } else {
            ATH_MSG_WARNING("Signal process vertex pointer not valid in HepMC Collection for GenEvent #" << cntr << " / " << mcColl->size());
          }
                
          // Get the beam particles
          pair<HepMC::ConstGenParticlePtr,HepMC::ConstGenParticlePtr> beamParticles;
          bool genEvt_valid_beam_particles=false;
#ifdef HEPMC3
        auto beamParticles_vec = genEvt->beams();
        genEvt_valid_beam_particles=(beamParticles_vec.size()>1);
        if (genEvt_valid_beam_particles){beamParticles.first=beamParticles_vec[0]; beamParticles.second=beamParticles_vec[1]; }
        // We want to process particles in barcode order.
        auto bcmapatt = genEvt->attribute<HepMC::GenEventBarcodes>("barcodes");
        if (!bcmapatt) ATH_MSG_ERROR("TruthParticleCnvTool.cxx: Event does not contain barcodes attribute"); 
        std::map<int, HepMC3::ConstGenParticlePtr> bcmap = bcmapatt->barcode_to_particle_map();
        xTruthParticleContainer->reserve(bcmap.size());
        for (const auto &[k,part]: bcmap) {
#else
        genEvt_valid_beam_particles=genEvt->valid_beam_particles();
        if ( genEvt_valid_beam_particles ) beamParticles = genEvt->beam_particles();
        xTruthParticleContainer->reserve(genEvt->particles_size());
        for (auto part: *genEvt) {
            int k = part->barcode();
#endif
            // (a) create TruthParticle
            xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
      // (b) Put particle into container;
            xTruthParticleContainer->push_back( xTruthParticle );
            fillParticle(xTruthParticle, part); // (c) Copy HepMC info into the new particle
            // (d) Build Event<->Particle element link
            const ElementLink<xAOD::TruthParticleContainer> eltp(*xTruthParticleContainer, xTruthParticleContainer->size()-1);
            if (isSignalProcess) xTruthEvent->addTruthParticleLink(eltp);
            if (!isSignalProcess) xTruthPileupEvent->addTruthParticleLink(eltp);
                    
            // Create link between HepMC and xAOD truth
            if (isSignalProcess) truthLinkVec->push_back(new xAODTruthParticleLink(HepMcParticleLink(k,0,EBC_MAINEVCOLL,HepMcParticleLink::IS_POSITION), eltp));
            if (!isSignalProcess) truthLinkVec->push_back(new xAODTruthParticleLink(HepMcParticleLink(k,genEvt->event_number()), eltp));
                    
            // Is this one of the beam particles?
            if (genEvt_valid_beam_particles) {
              if (isSignalProcess) {
          if (part == beamParticles.first) xTruthEvent->setBeamParticle1Link(eltp);
          if (part == beamParticles.second) xTruthEvent->setBeamParticle2Link(eltp);
              }
            }
            // (e) Particle's production vertex
            auto productionVertex = part->production_vertex();
            // Skip the dummy vertex that HepMC3 adds
            // Can distinguish it from real vertices because it has
            // a null event pointer.
            if (productionVertex && productionVertex->parent_event() != nullptr) {
              VertexParticles& parts = vertexMap[productionVertex];
              if (parts.incoming.empty() && parts.outgoing.empty())
          vertices.push_back (productionVertex);
              parts.outgoingEL.push_back(eltp);
              parts.outgoing.push_back(xTruthParticle);
            }
            //
            // else maybe want to keep track that this is the production vertex
            //
            // (f) Particle's decay vertex
            auto decayVertex = part->end_vertex();
            if (decayVertex) {
              VertexParticles& parts = vertexMap[decayVertex];
              if (parts.incoming.empty() && parts.outgoing.empty())
          vertices.push_back (decayVertex);
              parts.incomingEL.push_back(eltp);
              parts.incoming.push_back(xTruthParticle);
            }
                    
          } // end of loop over particles
                
          // (3) Loop over the map
          auto signalProcessVtx = HepMC::signal_process_vertex(genEvt); // Get the signal process vertex
    xTruthVertexContainer->reserve(vertices.size());
          for (const auto&  vertex : vertices) {
            const auto& parts = vertexMap[vertex];
            // (a) create TruthVertex
            xAOD::TruthVertex* xTruthVertex = new xAOD::TruthVertex();
      // (b) Put particle into container (so has store)
            xTruthVertexContainer->push_back( xTruthVertex );
            fillVertex(xTruthVertex, vertex); // (c) Copy HepMC info into the new vertex
            // (d) Build Event<->Vertex element link
            ElementLink<xAOD::TruthVertexContainer> eltv(*xTruthVertexContainer, xTruthVertexContainer->size()-1);
            // Mark if this is the signal process vertex
            if (vertex == signalProcessVtx && isSignalProcess) xTruthEvent->setSignalProcessVertexLink(eltv);
            if (isSignalProcess) xTruthEvent->addTruthVertexLink(eltv);
            if (!isSignalProcess) xTruthPileupEvent->addTruthVertexLink(eltv);
            // (e) Assign incoming particles to the vertex, from the map
            xTruthVertex->setIncomingParticleLinks( parts.incomingEL );
            // (f) Assign outgoing particles to the vertex, from the map
            xTruthVertex->setOutgoingParticleLinks( parts.outgoingEL );
            // (g) Set Particle<->Vertex links for incoming particles
            for (xAOD::TruthParticle* p : parts.incoming) p->setDecayVtxLink(eltv);
            // (h) Set Particle<->Vertex links for incoming particles
            for (xAOD::TruthParticle* p : parts.outgoing) p->setProdVtxLink(eltv);
          } //end of loop over vertices
                
          // Delete the event that wasn't used
          if (isSignalProcess) delete xTruthPileupEvent;
          if (!isSignalProcess) delete xTruthEvent;
                
        } // end of loop over McEventCollection
            
                
        std::stable_sort(truthLinkVec->begin(), truthLinkVec->end(), SortTruthParticleLink());
        ATH_MSG_VERBOSE("Summarizing truth link size: " << truthLinkVec->size() );
        
        return StatusCode::SUCCESS;
    }
    
        void xAODTruthCnvAlg::handle(const Incident& incident) {
          if (m_firstBeginRun && incident.type()==IncidentType::BeginRun) {
            m_firstBeginRun = false;
            ServiceHandle<StoreGateSvc> inputStore("StoreGateSvc/InputMetaDataStore", name());
            if(inputStore.retrieve().isFailure()) {
              ATH_MSG_ERROR("Failed to retrieve Input Metadata Store");
              return;
            }
            const IOVMetaDataContainer* tagInfo{nullptr};
            if(inputStore->retrieve(tagInfo,"/TagInfo").isFailure()) {
              ATH_MSG_WARNING("Failed to retrieve /TagInfo metadata from the input store");
              return;
            }
            if(tagInfo->payloadContainer()->size()>0) {
              CondAttrListCollection* tagInfoPayload = tagInfo->payloadContainer()->at(0);
              if(tagInfoPayload->size()>0) {
                const CondAttrListCollection::AttributeList& al = tagInfoPayload->attributeList(0);
                if (al.exists("lhefGenerator")){
                  m_metaFields.lhefGenerator = al["lhefGenerator"].data<std::string>();
                }

                if (al.exists("generators")){
                  m_metaFields.generators = al["generators"].data<std::string>();
                }

                if (al.exists("evgenProcess")){
                  m_metaFields.evgenProcess = al["evgenProcess"].data<std::string>();
                }

                if (al.exists("evgenTune")){
                  m_metaFields.evgenTune = al["evgenTune"].data<std::string>();
                }

                if (al.exists("hardPDF")){
                  m_metaFields.hardPDF = al["hardPDF"].data<std::string>();
                }

                if (al.exists("softPDF")){
                  m_metaFields.softPDF = al["softPDF"].data<std::string>();
                }
              }
            }
            else {
              ATH_MSG_WARNING("Empty Tag Info metadata!");
            }
          }
        }


    // A helper to set up a TruthVertex (without filling the ELs)
    void xAODTruthCnvAlg::fillVertex(xAOD::TruthVertex* tv, const HepMC::ConstGenVertexPtr& gv) {
        // id was renamed to status in HepMC3.
#ifdef HEPMC3
        tv->setId(gv->status());
#else
        tv->setId(gv->id());
#endif
        tv->setBarcode(HepMC::barcode(gv));
        tv->setX(gv->position().x());
        tv->setY(gv->position().y());
        tv->setZ(gv->position().z());
        tv->setT(gv->position().t());
    }
    
    
    // A helper to set up a TruthParticle (without filling the ELs)
    void xAODTruthCnvAlg::fillParticle(xAOD::TruthParticle* tp, const HepMC::ConstGenParticlePtr& gp) {
        tp->setPdgId(gp->pdg_id());
        tp->setBarcode(HepMC::barcode(gp));
        tp->setStatus(gp->status());
        
        auto pol = HepMC::polarization(gp);
        if (pol.is_defined()) {
            tp->setPolarizationParameter(pol.theta(), xAOD::TruthParticle::polarizationTheta);
            tp->setPolarizationParameter(pol.phi(), xAOD::TruthParticle::polarizationPhi);
        }
        
        tp->setM(gp->generated_mass());
        tp->setPx(gp->momentum().px());
        tp->setPy(gp->momentum().py());
        tp->setPz(gp->momentum().pz());
        tp->setE(gp->momentum().e());
    }


    StatusCode
    xAODTruthCnvAlg::MetaDataWriter::initialize (ServiceHandle<StoreGateSvc>& metaStore,
                                                 const std::string& metaName)
    {
      ATH_CHECK( metaStore.retrieve() );

      auto md = std::make_unique<xAOD::TruthMetaDataContainer>();
      m_tmd = md.get();

      auto aux = std::make_unique<xAOD::TruthMetaDataAuxContainer>();
      md->setStore( aux.get() );
            
      // Record the trigger configuration metadata into it:
      CHECK( metaStore->record( std::move (aux), metaName + "Aux." ) );
      CHECK( metaStore->record( std::move (md),  metaName ) );
      return StatusCode::SUCCESS;
    }


    StatusCode
    xAODTruthCnvAlg::MetaDataWriter::maybeWrite (uint32_t mcChannelNumber,
                                                 const HepMC::GenEvent& genEvt,
                                                 const MetadataFields& metaFields)
    {
      // This bit needs to be serialized.
      lock_t lock (m_mutex);
      
      //Inserting in a (unordered_)set returns an <iterator, boolean> pair, where the boolean
      //is used to check if the key already exists (returns false in the case it exists)
      if( m_existingMetaDataChan.insert(mcChannelNumber).second ) {
        m_tmd->push_back (std::make_unique <xAOD::TruthMetaData>());
        xAOD::TruthMetaData* md = m_tmd->back();
        
#ifdef HEPMC3
        ///Here comes the fix. Note that HepMC2.06.11 also contains the fix
        md->setMcChannelNumber(mcChannelNumber);
        std::vector<std::string> orderedWeightNameVec;
        if (!genEvt.run_info()) {
          for (size_t i=0; i<genEvt.weights().size();i++) orderedWeightNameVec.push_back(std::to_string(i));
        } else {
          if (!genEvt.run_info()->weight_names().empty()) {
            orderedWeightNameVec = genEvt.weight_names();
          } else {
            //AV This to be decided. It is always a good idea to have a default weight 1.0.
            //orderedWeightNameVec.push_back("0");
          }
        }
        md->setWeightNames(orderedWeightNameVec);
#else 
        // FIXME: class member protection violation here.
        // This appears to be because WeightContainer has no public methods
        // to get information about the weight names.
        const auto& weightNameMap = genEvt.weights().m_names;
        std::vector<std::string> orderedWeightNameVec;
        orderedWeightNameVec.reserve( weightNameMap.size() );
        for (const auto& entry: weightNameMap) {
          orderedWeightNameVec.push_back(entry.first);
        }
                            
        //The map from the HepMC record pairs the weight names with a corresponding index,
        //it is not guaranteed that the indices are ascending when iterating over the map
        std::sort(orderedWeightNameVec.begin(), orderedWeightNameVec.end(),
                  [&](const std::string& i, const std::string& j){return weightNameMap.at(i) < weightNameMap.at(j);});
                            
        md->setMcChannelNumber(mcChannelNumber);
        md->setWeightNames( orderedWeightNameVec );
#endif

        if(!metaFields.lhefGenerator.empty()) {
          md->setLhefGenerator(metaFields.lhefGenerator);
        }
        if(!metaFields.generators.empty()) {
          md->setGenerators(metaFields.generators);
        }
        if(!metaFields.evgenProcess.empty()) {
          md->setEvgenProcess(metaFields.evgenProcess);
        }
        if(!metaFields.evgenTune.empty()) {
          md->setEvgenTune(metaFields.evgenTune);
        }
        if(!metaFields.hardPDF.empty()) {
          md->setHardPDF(metaFields.hardPDF);
        }
        if(!metaFields.softPDF.empty()) {
          md->setSoftPDF(metaFields.softPDF);
        }
      }

      return StatusCode::SUCCESS;
    }

    
    
} // namespace xAODMaker
