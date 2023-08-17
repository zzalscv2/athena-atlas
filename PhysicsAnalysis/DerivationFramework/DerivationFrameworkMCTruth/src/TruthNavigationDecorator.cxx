/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthNavigationDecorator.cxx
// Add navigation information to small truth collections

#include "TruthNavigationDecorator.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <string>
#include <vector>
#include <map>

// Constructor
DerivationFramework::TruthNavigationDecorator::TruthNavigationDecorator(const std::string& t,
        const std::string& n,
        const IInterface* p ) :
    AthAlgTool(t,n,p)
{
  declareInterface<DerivationFramework::IAugmentationTool>(this);
}

// Destructor
DerivationFramework::TruthNavigationDecorator::~TruthNavigationDecorator() {
}

// Initialise
StatusCode DerivationFramework::TruthNavigationDecorator::initialize() {

  // Initialise input keys
  ATH_CHECK( m_inputKeys.initialize() ); 
  ATH_CHECK( m_truthEventKey.initialize() );  

  // Decorations - dependent on the name of the input keys 
  // Loop over the container names provided by the user
  for (auto key : m_inputKeys) {
    m_parentLinksDecorKeys.emplace_back(key.key()+".parentLinks");   
    m_childLinksDecorKeys.emplace_back(key.key()+".childLinks");  
  }
  
  ATH_CHECK( m_parentLinksDecorKeys.initialize() );
  ATH_CHECK( m_childLinksDecorKeys.initialize() );

  return StatusCode::SUCCESS;

}

// Function to do dressing, implements interface in IAugmentationTool
StatusCode DerivationFramework::TruthNavigationDecorator::addBranches() const
{
  // Event context 
  const EventContext& ctx = Gaudi::Hive::currentContext();   

  // Retrieve the truth collections
  SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_truthEventKey, ctx);
  if (!truthEvents.isValid()) {
    ATH_MSG_ERROR("Couldn't retrieve TruthEvent collection with name " << m_truthEventKey);
    return StatusCode::FAILURE;
  }

  // Retrieve all the individual particle collections
  std::vector<SG::ReadHandle<xAOD::TruthParticleContainer> > inputParticles;
  inputParticles.reserve(m_inputKeys.size());
  for (const SG::ReadHandleKey<xAOD::TruthParticleContainer>& inputKey : m_inputKeys) {
    inputParticles.push_back(SG::ReadHandle<xAOD::TruthParticleContainer>(inputKey, ctx));
  }  

  // Build a dictionary of barcodes and element links
  std::map<int,ElementLink<xAOD::TruthParticleContainer> > linkMap;
  for (auto& coll : inputParticles){
    for (size_t p=0;p<coll.ptr()->size();++p){
      if (!coll.ptr()->at(p)) continue; // Protection against null ptrs
      if (linkMap.find(coll.ptr()->at(p)->barcode())!=linkMap.end()) continue; // Particle in multiple collections
      linkMap[coll.ptr()->at(p)->barcode()] = ElementLink<xAOD::TruthParticleContainer>(*coll,p);
    } // Loop over particles in the collection
  } // Loop over collections

  // Now loop over the collections and for each one decorate children and parents
  // The list of particles we keep is small-ish, and the list of particles in the
  // original truth record is large-ish, so I think it will be more efficient to
  // do a loop (O(N)) over the big record and a search (O(Nlog(N))) over the small
  // container.  Future performance optimization is welcome...

  // Keep maps, do the decoration last.  This ensures that duplicates all get decorated.
  std::map< int , std::vector<ElementLink<xAOD::TruthParticleContainer> > > parentMap;
  std::map< int , std::vector<ElementLink<xAOD::TruthParticleContainer> > > childMap;

  // Loop protection
  std::vector<int> seen_particles(20);
  // As usual, only consider the first truth event
  const xAOD::TruthEvent * event = truthEvents->at(0);
  for (size_t p=0;p<event->nTruthParticles();++p){
    if (!event->truthParticle(p)) continue; // Protection against null ptrs
    if (linkMap.find(event->truthParticle(p)->barcode())==linkMap.end()) continue; // Not a particle we are interested in
    // Make parent and child lists
    std::vector<ElementLink<xAOD::TruthParticleContainer> > parents;
    std::vector<ElementLink<xAOD::TruthParticleContainer> > children;
    // Populate the lists - include loop protection
    seen_particles.clear();
    find_parents( event->truthParticle(p) , parents , linkMap , seen_particles );
    seen_particles.clear();
    find_children( event->truthParticle(p) , children , linkMap , seen_particles );
    // Set the maps, so that we can decorate later
    parentMap[event->truthParticle(p)->barcode()] = parents;
    childMap[event->truthParticle(p)->barcode()] = children;
  } // Loop over truth particles in the big truth collection

  // Now final loop over the collections and setting all the decorators
  auto parent_decorator = m_parentLinksDecorKeys.makeHandles (ctx);
  auto child_decorator  = m_childLinksDecorKeys.makeHandles (ctx);
  unsigned int pCntr{0};
  for (auto coll : inputParticles){
    if (parent_decorator.at(pCntr).isAvailable()) {
      ++pCntr;
      continue;
    }
    for (size_t p=0;p<coll.ptr()->size();++p){
      if (!coll.ptr()->at(p)) continue; // Protection against null ptrs
      parent_decorator.at(pCntr)(*coll.ptr()->at(p)) = parentMap[ coll->at(p)->barcode() ];
      child_decorator.at(pCntr)(*coll.ptr()->at(p)) = childMap[ coll->at(p)->barcode() ];
    } // Loop over the particles in each collection
    ++pCntr;
  } // Loop over the collections

  return StatusCode::SUCCESS;
}

void DerivationFramework::TruthNavigationDecorator::find_parents( const xAOD::TruthParticle* part ,
                                std::vector<ElementLink<xAOD::TruthParticleContainer> >& parents ,
                                std::map<int,ElementLink<xAOD::TruthParticleContainer> >& linkMap ,
                                std::vector<int>& seen_particles ) const {
  // Null pointer protection
  if (!part) return;
  // Check if we've seen the particle before, otherwise add it to our list
  if (std::find(seen_particles.begin(),seen_particles.end(),part->barcode())!=seen_particles.end()) return;
  seen_particles.push_back(part->barcode());
  // Loop through the parents and see if we know about them; otherwise iterate through the list
  for (size_t parent=0;parent<part->nParents();++parent){
    if (!part->parent(parent)) continue; // Null pointer check
    if (linkMap.find(part->parent(parent)->barcode())!=linkMap.end()){
      // Hit!  Add it to the list
      parents.push_back( linkMap[part->parent(parent)->barcode()] );
    } else {
      // Not a hit yet, keep iterating
      find_parents( part->parent(parent) , parents , linkMap , seen_particles );
    }
  } // Loop over parents
}

void DerivationFramework::TruthNavigationDecorator::find_children( const xAOD::TruthParticle* part ,
                                std::vector<ElementLink<xAOD::TruthParticleContainer> >& children ,
                                std::map<int,ElementLink<xAOD::TruthParticleContainer> >& linkMap ,
                                std::vector<int>& seen_particles ) const {
  // Null pointer protection
  if (!part) return;
  // Check if we've seen the particle before, otherwise add it to our list
  if (std::find(seen_particles.begin(),seen_particles.end(),part->barcode())!=seen_particles.end()) return;
  seen_particles.push_back(part->barcode());
  // Look through the children and see if we know about them; otherwise iterate through the list
  for (size_t child=0;child<part->nChildren();++child){
    if (!part->child(child)) continue; // Null pointer check
    if (linkMap.find(part->child(child)->barcode())!=linkMap.end()){
      // Hit!  Add it to the list
      children.push_back( linkMap[part->child(child)->barcode()] );
    } else {
      // Not a hit yet, keep iterating
      find_children( part->child(child) , children , linkMap , seen_particles );
    }
  } // Loop over parents
}
