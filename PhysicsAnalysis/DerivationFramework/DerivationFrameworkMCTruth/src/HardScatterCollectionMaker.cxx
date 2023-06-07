/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HardScatterCollectionMaker.cxx
// Keeps the first N generations of particles from the hard scatter
// Based on TruthDecayCollectionMaker (but simpler)

#include "HardScatterCollectionMaker.h"
#include "CollectionMakerHelpers.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthVertexAuxContainer.h"

// To look up which generator is being used
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODTruth/TruthMetaDataContainer.h"

// STL includes
#include <vector>
#include <string>
#include <algorithm>

// For string manipulation
#include <boost/algorithm/string.hpp>

// Constructor
DerivationFramework::HardScatterCollectionMaker::HardScatterCollectionMaker(const std::string& t,
                                                                          const std::string& n,
                                                                          const IInterface* p)
  : AthAlgTool(t,n,p)
{
    declareInterface<DerivationFramework::IAugmentationTool>(this);
}

// Destructor
DerivationFramework::HardScatterCollectionMaker::~HardScatterCollectionMaker()  = default;
// Athena initialize
StatusCode DerivationFramework::HardScatterCollectionMaker::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    ATH_CHECK(m_eventsKey.initialize());
    ATH_MSG_INFO("Using " << m_eventsKey << " as the source collections for new truth collections");

    if (m_collectionName.value().empty()) {
        ATH_MSG_FATAL("No key provided for the new truth particle collections");
        return StatusCode::FAILURE;
    } 
    ATH_MSG_INFO("New truth particle collection key: " << m_collectionName );
    m_outPartKey = m_collectionName.value() + "Particles";
    m_outVtxKey = m_collectionName.value() + "Vertices";
    ATH_CHECK(m_outVtxKey.initialize());
    ATH_CHECK(m_outPartKey.initialize());
    return StatusCode::SUCCESS;
}


// Selection and collection creation
StatusCode DerivationFramework::HardScatterCollectionMaker::addBranches() const {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    // Set up for some metadata handling
    static const bool is_pure_pythia8 = [this]() {
        bool is_pure_pythia8 = false;
        if (m_metaStore->contains<xAOD::TruthMetaDataContainer>("TruthMetaData")){
            // Note that I'd like to get this out of metadata in general, but it seems that the
            // metadata isn't fully available in initialize, and since this is a const function
            // I can only do the retrieve every event, rather than lazy-initializing, since this
            // metadata ought not change during a run
            const xAOD::TruthMetaDataContainer* truthMetaData(nullptr);
            // Shamelessly stolen from the file meta data tool
            if (m_metaStore->retrieve(truthMetaData).isSuccess() && !truthMetaData->empty()){
                // Let's just be super sure...
                const std::string gens = boost::algorithm::to_lower_copy(truthMetaData->at(0)->generators());
                // Check if it has Pythia8 in it
                is_pure_pythia8 = (gens.find("pythia8")==std::string::npos) ? false : true;
                // Check if it has something *else* in it
                std::string remainder = boost::algorithm::erase_all_copy(gens,"pythia8");
                boost::algorithm::erase_all(remainder,"evtgen");
                boost::algorithm::erase_all(remainder,"+");
                if (!remainder.empty()){
                    ATH_MSG_INFO("Ideentified sample as not pure-Pythia8. Gen info was " << gens);
                    is_pure_pythia8=false;
                } else if (is_pure_pythia8){
                    ATH_MSG_INFO("Identified sample as pure-Pythia8. Reconfiguring accordingly. Gen info was " << gens);
                }
            } // Seems to be the only sure way...
            else {
                ATH_MSG_WARNING("Found xAODTruthMetaDataContainer empty! Configuring to be NOT pure Pythia8.");
            }
        } else {
            ATH_MSG_WARNING("Could not find metadata container in storegate; assuming NOT pure Pythia8");
        }
        return is_pure_pythia8;
    }();

    // Retrieve truth collections
    SG::ReadHandle<xAOD::TruthEventContainer> importedTruthEvents{m_eventsKey, ctx};
    if (!importedTruthEvents.isValid()) {
        ATH_MSG_ERROR("No TruthEvent collection with name " << m_eventsKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    // We only care about the first event
    if (importedTruthEvents->empty()){
        ATH_MSG_ERROR("TruthEvent collection with name " << m_eventsKey.fullKey() << " is empty!");
        return StatusCode::FAILURE;
    }
    // Check that it has a hard scatter process defined
    const xAOD::TruthVertex* my_tv = importedTruthEvents->at(0)->signalProcessVertex();
    if (my_tv==nullptr){
        [[maybe_unused]] static const bool warn_once = [this]() {
            ATH_MSG_WARNING("TruthEvent collection with name " << m_eventsKey.fullKey() << " has a null signal process vertex!");
            return true;
        }();
        size_t i_vtx=0;
        // Sometimes we're unlucky and the first vertex is null!
        while (!my_tv && i_vtx<importedTruthEvents->at(0)->nTruthVertices()){
          my_tv = importedTruthEvents->at(0)->truthVertex(i_vtx);
          i_vtx++;
        }
        if (my_tv==nullptr){
            ATH_MSG_ERROR("TruthEvent collection had no vertices at all? Something is wrong with your truth record!");
            return StatusCode::FAILURE;
        }
    }

    // Create the new particle containers
    SG::WriteHandle<xAOD::TruthParticleContainer> writeHandlePart{m_outPartKey, ctx};
    ATH_CHECK(writeHandlePart.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                           std::make_unique<xAOD::TruthParticleAuxContainer>()));
    xAOD::TruthParticleContainer* newParticleCollection = writeHandlePart.ptr();
    ATH_MSG_DEBUG( "Recorded new TruthParticleContainer with key: " << m_outPartKey.fullKey());
    // Create the new vertex containers
    SG::WriteHandle<xAOD::TruthVertexContainer> writeHandleVtx{m_outVtxKey, ctx};
    ATH_CHECK(writeHandleVtx.record(std::make_unique<xAOD::TruthVertexContainer>(),
                                         std::make_unique<xAOD::TruthVertexAuxContainer>()));
    xAOD::TruthVertexContainer* newVertexCollection = writeHandleVtx.ptr();
    ATH_MSG_DEBUG( "Recorded new TruthVertexContainer with key: " << m_outVtxKey.fullKey());

    std::vector<int> seen_particles; // Loop protection

    // If this was pure Pythia8, we are going to fake the collection
    if (is_pure_pythia8){
        // Make a dummy vertex
        xAOD::TruthVertex* xTruthVertex0 = new xAOD::TruthVertex();
        newVertexCollection->push_back( xTruthVertex0 );
        // Get a link to this vertex -- will be used to set production vertices on all the next particles
        int myIndex0 = newVertexCollection->size()-1;
        ElementLink<xAOD::TruthVertexContainer> eltv0(*newVertexCollection, myIndex0);
        // Set properties. Cheating here; if we had a signal process vertex, use its properties
        xTruthVertex0->setId(my_tv->id());
        xTruthVertex0->setBarcode(my_tv->barcode());
        xTruthVertex0->setX(my_tv->x());
        xTruthVertex0->setY(my_tv->y());
        xTruthVertex0->setZ(my_tv->z());
        xTruthVertex0->setT(my_tv->t());
        // Same story, but for the completed intermediate particles
        xAOD::TruthVertex* xTruthVertex1 = new xAOD::TruthVertex();
        newVertexCollection->push_back( xTruthVertex1 );
        // Get a link to this vertex -- will be used to set production vertices on all the next particles
        int myIndex1 = newVertexCollection->size()-1;
        ElementLink<xAOD::TruthVertexContainer> eltv1(*newVertexCollection, myIndex1);
        // Set properties. Cheating here; if we had a signal process vertex, use its properties
        xTruthVertex1->setId(my_tv->id());
        xTruthVertex1->setBarcode(my_tv->barcode()-1);
        xTruthVertex1->setX(my_tv->x());
        xTruthVertex1->setY(my_tv->y());
        xTruthVertex1->setZ(my_tv->z());
        xTruthVertex1->setT(my_tv->t());

        // Now run through the particles...
        for (size_t np=0;np<importedTruthEvents->at(0)->nTruthParticles();++np){
            // Get the particle
            const xAOD::TruthParticle* tp = importedTruthEvents->at(0)->truthParticle(np);
            // Better not be a null pointer
            if (!tp) continue;
            // Attach based on status codes!
            // status 21 means incoming
            if (tp->status()==21){
                xAOD::TruthParticle *xTruthParticle = CollectionMakerHelpers::setupTruthParticle( *tp , newParticleCollection );
                // Make a link to this particle
                int myIndex = newParticleCollection->size()-1;
                ElementLink<xAOD::TruthParticleContainer> eltp(*newParticleCollection, myIndex);
                // Add it to the vertex
                xTruthParticle->setDecayVtxLink( eltv0 );
                (*newVertexCollection)[myIndex0]->addIncomingParticleLink( eltp );
                seen_particles.push_back(tp->barcode());
            }
            // Status 23 means outgoing - include requested generations (less this one)
            else if (tp->status()==23){
                int handyIndex = CollectionMakerHelpers::addTruthParticle( *tp, newParticleCollection, newVertexCollection, seen_particles, m_generations-1 );
                // Make a link to this particle
                ElementLink<xAOD::TruthParticleContainer> eltp(*newParticleCollection, handyIndex);
                // Add it to the vertex
                (*newParticleCollection)[handyIndex]->setProdVtxLink( eltv1 );
                (*newVertexCollection)[myIndex1]->addOutgoingParticleLink( eltp );
                seen_particles.push_back(tp->barcode());
            }
            // Status 22 or 62 means intermediate - only save bosons
            else if ((tp->status()==22 || tp->status()==62) && (tp->isW() || tp->isZ() || tp->isHiggs() || tp->isBSM())){
                xAOD::TruthParticle *xTruthParticle = CollectionMakerHelpers::setupTruthParticle( *tp , newParticleCollection );
                // Make a link to this particle
                int myIndex = newParticleCollection->size()-1;
                ElementLink<xAOD::TruthParticleContainer> eltp(*newParticleCollection, myIndex);
                // Add it to both the vertices
                xTruthParticle->setProdVtxLink( eltv0 );
                (*newVertexCollection)[myIndex0]->addOutgoingParticleLink( eltp );
                xTruthParticle->setDecayVtxLink( eltv1 );
                (*newVertexCollection)[myIndex1]->addIncomingParticleLink( eltp );
                seen_particles.push_back(tp->barcode());
            }
        }
        // Done with my new collection for Pythia8
        return StatusCode::SUCCESS;
    }

    // Get the signal process vertex.  Get the incoming particles and outgoing particles and 
    // make a mini truth collection based on those
    bool first_particle=true;

    // Are there any other incoming particles we need to add?
    for (size_t i=0;i<my_tv->nIncomingParticles();++i){
        // Check for a null pointer
        if (!my_tv->incomingParticle(i)) continue;
        // See if this is the first particle we're adding
        if (first_particle){
          CollectionMakerHelpers::addTruthParticle( *(my_tv->incomingParticle(i)), newParticleCollection, newVertexCollection, seen_particles, m_generations );
          first_particle=false;
          continue;
        }
        // Otherwise we're going to be adding to the existing collection
        // Set up the truth particle
        xAOD::TruthParticle* xTruthParticle = CollectionMakerHelpers::setupTruthParticle(*(my_tv->incomingParticle(i)),newParticleCollection);
        if (!xTruthParticle){
          ATH_MSG_WARNING("setupTruthParticle returned nullptr...");
          continue;
        }
        // Make a link to this particle
        int my_index = newParticleCollection->size()-1;
        ElementLink<xAOD::TruthParticleContainer> eltp(*newParticleCollection, my_index);
        // Decay vertex link
        ElementLink<xAOD::TruthVertexContainer> eltv( *newVertexCollection, 0 ); // First one we added in
        xTruthParticle->setDecayVtxLink( eltv );
        (*newVertexCollection)[0]->addIncomingParticleLink( eltp );
        // Note that we did it
        seen_particles.push_back( xTruthParticle->barcode() );
    } // Done adding incoming particles -- all finished!

    // Extra check, because truth records are complicated, based on studies by Dan Hayden (thanks!)
    for (size_t np=0;np<importedTruthEvents->at(0)->nTruthParticles();++np){
        // Get the particle
        const xAOD::TruthParticle* tp = importedTruthEvents->at(0)->truthParticle(np);
        // Better not be a null pointer
        if (!tp) continue;
        // Attach based on status codes!
        // status 21 means incoming
        if (tp->status()!=21) continue;
        // See if it's the first particle
        if (first_particle){
          CollectionMakerHelpers::addTruthParticle( *tp, newParticleCollection, newVertexCollection, seen_particles, m_generations );
          first_particle=false;
          continue;
        }
        // See if we already got this one
        if (std::find(seen_particles.begin(),seen_particles.end(),tp->barcode())!=seen_particles.end()){
            continue;
        } // Found it in the old container
        // Otherwise, add it
        xAOD::TruthParticle *xTruthParticle = CollectionMakerHelpers::setupTruthParticle( *tp , newParticleCollection );
        // Make a link to this particle
        int myIndex = newParticleCollection->size()-1;
        ElementLink<xAOD::TruthParticleContainer> eltp(*newParticleCollection, myIndex);
        // Add it to the vertex
        ElementLink<xAOD::TruthVertexContainer> eltv( *newVertexCollection, 0 ); // First one we added in
        xTruthParticle->setDecayVtxLink( eltv );
        (*newVertexCollection)[0]->addIncomingParticleLink( eltp );
        // Note that we did it
        seen_particles.push_back( xTruthParticle->barcode() );
    }

    return StatusCode::SUCCESS;
}
