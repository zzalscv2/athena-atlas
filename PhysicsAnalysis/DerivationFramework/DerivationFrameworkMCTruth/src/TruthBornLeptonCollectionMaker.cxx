/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthBornLeptonCollectionMaker.cxx
// Makes a special collection of Born leptons

// R/W/D handles
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
// My own header file
#include "TruthBornLeptonCollectionMaker.h"
// EDM includes for the particles we need
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
// To look up which generator is being used
#include "StoreGate/StoreGateSvc.h"
#include "xAODTruth/TruthMetaDataContainer.h"
// STL includes
#include <string>

// Constructor
DerivationFramework::TruthBornLeptonCollectionMaker::TruthBornLeptonCollectionMaker(const std::string& t,
                                const std::string& n,
                                const IInterface* p)
  : AthAlgTool(t,n,p)
  , m_metaStore( "MetaDataStore", n )
{
  declareInterface<DerivationFramework::IAugmentationTool>(this);
  declareProperty( "MetaDataStore", m_metaStore );
}

// Destructor
DerivationFramework::TruthBornLeptonCollectionMaker::~TruthBornLeptonCollectionMaker() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::TruthBornLeptonCollectionMaker::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

   // Input truth particles
   ATH_CHECK( m_particlesKey.initialize() );
   ATH_MSG_INFO("Using " << m_particlesKey.key() << " as the input truth container key");

  // Output truth particles
  if (m_collectionName.empty()) {
    ATH_MSG_FATAL("No key provided for the new truth particle collection");
    return StatusCode::FAILURE;
  } else {ATH_MSG_INFO("New truth particle collection key: " << m_collectionName.key() );}
  ATH_CHECK( m_collectionName.initialize());

  // Decoration keys
  m_originDecoratorKey = m_collectionName.key() + ".classifierParticleOrigin";
  ATH_CHECK(m_originDecoratorKey.initialize());
  m_typeDecoratorKey = m_collectionName.key() + ".classifierParticleType";
  ATH_CHECK(m_typeDecoratorKey.initialize());
  m_outcomeDecoratorKey = m_collectionName.key() + ".classifierParticleOutCome";
  ATH_CHECK(m_outcomeDecoratorKey.initialize());
  m_classificationDecoratorKey = m_collectionName.key() + ".Classification";
  ATH_CHECK(m_classificationDecoratorKey.initialize());
  
  // TODO: needs to be made MT-friendly
  ATH_CHECK( m_metaStore.retrieve() );

  return StatusCode::SUCCESS;
}

// Selection and collection creation
StatusCode DerivationFramework::TruthBornLeptonCollectionMaker::addBranches() const
{
  // Event context
  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  // Set up for some metadata handling
  static const bool is_sherpa = [this]() {
    bool is_sherpa = false;
    // TODO: needs to be made MT-friendly
    if (m_metaStore->contains<xAOD::TruthMetaDataContainer>("TruthMetaData")){
      // Note that I'd like to get this out of metadata in general, but it seems that the
      // metadata isn't fully available in initialize, and since this is a const function
      // I can only do the retrieve every event, rather than lazy-initializing, since this
      // metadata ought not change during a run
      const xAOD::TruthMetaDataContainer* truthMetaData(nullptr);
      // Shamelessly stolen from the file meta data tool

      if (m_metaStore->retrieve(truthMetaData).isSuccess() && !truthMetaData->empty()){
        // Let's just be super sure...
        const std::string gens = truthMetaData->at(0)->generators();
        is_sherpa = (gens.find("sherpa")==std::string::npos &&
                     gens.find("Sherpa")==std::string::npos &&
                     gens.find("SHERPA")==std::string::npos) ? false : true;
      } // Seems to be the only sure way...
      else {
        ATH_MSG_WARNING("Found xAODTruthMetaDataContainer empty! Configuring to be NOT Sherpa.");
      }
      ATH_MSG_INFO("From metadata configured: Sherpa? " << is_sherpa);
    } {
      ATH_MSG_WARNING("Could not find metadata container in storegate; assuming NOT Sherpa");
    }
    return is_sherpa;
  }();

  // Retrieve truth collections
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);    
  if (!truthParticles.isValid()) {        
    ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);        
    return StatusCode::FAILURE;    
  }

  // Create the new particle containers and WriteHandles
  SG::WriteHandle<xAOD::TruthParticleContainer> newParticlesWriteHandle(m_collectionName, ctx);
  ATH_CHECK(newParticlesWriteHandle.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                           std::make_unique<xAOD::TruthParticleAuxContainer>()));
  ATH_MSG_DEBUG( "Recorded new TruthParticleContainer with key: " << (m_collectionName.key()));

  // Set up decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > originDecorator(m_originDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeDecorator(m_typeDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeDecorator(m_outcomeDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationDecorator(m_classificationDecoratorKey, ctx);

  // add relevant particles to new collection
  for (unsigned int i=0; i<truthParticles->size(); ++i) {
    // Grab the particle
    const xAOD::TruthParticle* theParticle = (*truthParticles)[i];
    if (!theParticle) continue; // Protection against null pointers
    if (!theParticle->isLepton()) continue; // Only include leptons!

    if (is_sherpa>0 && theParticle->status()!=11){
      // If Sherpa, take leptons with status 11
      continue;
    } else if (is_sherpa==0) {
      // Some generators, look for leptons with status 3 coming from vertices with other leptons
      bool has_status_n3=false, has_status_3=false, has_V=false;
      if (theParticle->status()==3){
        // Look for other leptons in the production vertex... carefully
        if (theParticle->hasProdVtx()){
          const xAOD::TruthVertex * prod = theParticle->prodVtx();
          for (size_t p=0;p<prod->nOutgoingParticles();++p){
            if (prod->outgoingParticle(p) &&
              prod->outgoingParticle(p)->isLepton()){
              has_status_n3 = has_status_n3 || prod->outgoingParticle(p)->status()!=3;
              has_status_3 = has_status_3 || prod->outgoingParticle(p)->status()==3;
            }
          } // Loop over particles from the same production vertex
          for (size_t p=0;p<prod->nIncomingParticles();++p){
            // See if there was a boson going *into* the vertex
            if (prod->incomingParticle(p) &&
              (prod->incomingParticle(p)->isZ() || prod->incomingParticle(p)->isW() || prod->incomingParticle(p)->isHiggs()) ){
              has_V=true;
              break;
            } // Found a vector boson
          } // Loop over particles going into the same production vertex
        } // Doesn't have a production vertex
      } // Had status 3

      // Now we have all the information for the special case of V->l(born) l(bare) l(born) l(bare)
      if ( !(has_status_3 && has_status_n3 && has_V && theParticle->status()==3) &&
         theParticle->status()!=23){
        // If not a special case, deal with the standard: has a boson parent, is a lepton, and has a descendent that is a bare lepton
        if (!theParticle->parent()) continue;
        if (!theParticle->parent()->isZ() && !theParticle->parent()->isW() && !theParticle->parent()->isHiggs()) continue;
        if (!hasBareDescendent( theParticle ) ) continue;
      }
    } // End of treatment for generators that are not Sherpa

    // Add this particle to the new collection
    xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
    newParticlesWriteHandle->push_back( xTruthParticle );
    // Fill with numerical content
    xTruthParticle->setPdgId(theParticle->pdgId());
    xTruthParticle->setBarcode(theParticle->barcode());
    xTruthParticle->setStatus(theParticle->status());
    xTruthParticle->setM(theParticle->m());
    xTruthParticle->setPx(theParticle->px());
    xTruthParticle->setPy(theParticle->py());
    xTruthParticle->setPz(theParticle->pz());
    xTruthParticle->setE(theParticle->e());
    // Copy over the decorations if they are available
    if (theParticle->isAvailable<unsigned int>("classifierParticleType")) {
      typeDecorator(*xTruthParticle) = theParticle->auxdata< unsigned int >( "classifierParticleType" );
    } else {typeDecorator(*xTruthParticle) = 0;}
    if (theParticle->isAvailable<unsigned int>("classifierParticleOrigin")) {
      originDecorator(*xTruthParticle) = theParticle->auxdata< unsigned int >( "classifierParticleOrigin" );
    } else {originDecorator(*xTruthParticle) = 0;}
    if (theParticle->isAvailable<unsigned int>("classifierParticleOutCome")) {
      outcomeDecorator(*xTruthParticle) = theParticle->auxdata< unsigned int >( "classifierParticleOutCome" );
    } else {outcomeDecorator(*xTruthParticle) = 0;}
    if (theParticle->isAvailable<unsigned int>("Classification")) {
      classificationDecorator(*xTruthParticle) = theParticle->auxdata< unsigned int >( "Classification" );
    } else {classificationDecorator(*xTruthParticle) = 0;}
  } // Loop over alll particles

  return StatusCode::SUCCESS;
}

// Find out if a particle has a bare descendent
bool DerivationFramework::TruthBornLeptonCollectionMaker::hasBareDescendent( const xAOD::TruthParticle* p ) const
{
  // Null pointer check
  if (!p) return false;
  // If we hit a bare descendent, then we're a winnner
  if (p->isLepton() && p->status()==1) return true;
  // Otherwise look through all the children
  for (size_t c=0;c<p->nChildren();++c){
    if (!p->child(c)) continue; // Null pointer protection
    if (p->pdgId()!=p->child(c)->pdgId()) continue; // Different particle child
    if (hasBareDescendent( p->child(c) )) return true;
  }
  // No luck -- this branch is a dead end
  return false;
}

