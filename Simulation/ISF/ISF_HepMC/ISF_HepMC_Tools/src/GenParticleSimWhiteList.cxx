/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "GenParticleSimWhiteList.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

// Helper function
#include "TruthUtils/HepMCHelpers.h"

// For finding that file
#include "PathResolver/PathResolver.h"

// Units
#include <fstream>
#include <cstdlib>

/** Constructor **/
ISF::GenParticleSimWhiteList::GenParticleSimWhiteList( const std::string& t,
                                                       const std::string& n,
                                                       const IInterface* p )
  : base_class(t,n,p)
{
}

// Athena algtool's Hooks
StatusCode  ISF::GenParticleSimWhiteList::initialize()
{
    ATH_MSG_VERBOSE("Initializing ...");

    // Initialize the list
    m_pdgId.clear();

    for (auto &whiteList : m_whiteLists) {
      // Get the appropriate file handle
      std::string resolvedFilename = PathResolver::find_file( whiteList , "DATAPATH" );
      std::ifstream white_list;
      white_list.open( resolvedFilename );
      if (!white_list.is_open()){
        ATH_MSG_ERROR("Could not find white list " << whiteList);
        return StatusCode::FAILURE;
      }

      // Parse the list into the vector
      std::string a_line;
      char * pEnd;
      while (!white_list.eof()){
        getline( white_list , a_line );
        long int pdg = strtol( a_line.c_str() , &pEnd , 10 );
        if (std::find(m_pdgId.begin(), m_pdgId.end(), pdg) == m_pdgId.end()) {
          m_pdgId.push_back(pdg);
        } else {
          ATH_MSG_DEBUG("pdgId " << pdg << " already in whitelist. Will not add it again.");
        }
      }

      // Sort the list for use later
      std::sort( m_pdgId.begin() , m_pdgId.end() );
    }

    // All done!
    return StatusCode::SUCCESS;
}

/** passes through to the private version of the filter */
#ifdef HEPMC3
bool ISF::GenParticleSimWhiteList::pass(const HepMC::ConstGenParticlePtr& particle) const
{

  ATH_MSG_VERBOSE( "Checking whether " << particle << " passes the filter." );

  std::vector<int> vertices(500);
  bool so_far_so_good = pass( particle , vertices );

  // Test all parent particles
  const int id_of_particle = particle->id();
  if (so_far_so_good && particle->production_vertex() && m_qs){
    for (auto& pit: particle->production_vertex()->particles_in()){
      // Loop breaker
      if ( pit->id() == id_of_particle ) continue;
      // Check this particle
      vertices.clear();
      bool parent_all_clear = pass( pit , vertices );
      ATH_MSG_VERBOSE( "Parent all clear: " << parent_all_clear <<
         "\nIf true, will not pass the daughter because it should have been picked up through the parent already (to avoid multi-counting)." );
      so_far_so_good = so_far_so_good && !parent_all_clear;
    } // Loop over parents
  } // particle had parents

  return so_far_so_good;
}
#else
bool ISF::GenParticleSimWhiteList::pass(const HepMC::GenParticle& particle) const
{

  ATH_MSG_VERBOSE( "Checking whether " << particle << " passes the filter." );

  std::vector<int> vertices(500);
  bool so_far_so_good = pass( particle , vertices );

  // Test all parent particles
  if (so_far_so_good && particle.production_vertex() && m_qs){
    for (HepMC::GenVertex::particle_iterator it = particle.production_vertex()->particles_begin(HepMC::parents);
                                             it != particle.production_vertex()->particles_end(HepMC::parents); ++it){
      // Loop breaker
      if ( (*it)->barcode() == particle.barcode() ) continue;
      // Check this particle
      vertices.clear();
      bool parent_all_clear = pass( **it , vertices );
      ATH_MSG_VERBOSE( "Parent all clear: " << parent_all_clear <<
         "\nIf true, will not pass the daughter because it should have been picked up through the parent already (to avoid multi-counting)." );
      so_far_so_good = so_far_so_good && !parent_all_clear;
    } // Loop over parents
  } // particle had parents

  return so_far_so_good;
}
#endif

/** returns true if the the particle and all daughters are on the white list */
#ifdef HEPMC3
bool ISF::GenParticleSimWhiteList::pass(const HepMC::ConstGenParticlePtr& particle , std::vector<int> & used_vertices ) const
{
  // See if the particle is in the white list
  bool passFilter = std::binary_search( m_pdgId.begin() , m_pdgId.end() , particle->pdg_id() ) || MC::isNucleus( particle->pdg_id() );
  // Remove documentation particles
  passFilter = passFilter && particle->status()<3;
  // Test all daughter particles
  if (particle->end_vertex() && m_qs && passFilter){
    // Primarily interested in passing particles decaying outside
    // m_minDecayRadiusQS (nomimally the inner radius of the
    // beampipe). However, it is also interesting to pass particles
    // which start outside m_minDecayRadiusQS, but decay inside it.
    passFilter = passFilter && ( (m_minDecayRadiusQS < particle->end_vertex()->position().perp()) || (m_minDecayRadiusQS < particle->production_vertex()->position().perp()) );
    if (passFilter) {
      // Break loops
      if ( std::find( used_vertices.begin() , used_vertices.end() , particle->end_vertex()->id() )==used_vertices.end() ){
        used_vertices.push_back( particle->end_vertex()->id() );
        for (auto& pit: particle->end_vertex()->particles_out()){
          passFilter = passFilter && pass( pit , used_vertices );
          if (!passFilter) {
            ATH_MSG_VERBOSE( "Daughter particle " << pit << " does not pass." );
            break;
          }
        } // Loop over daughters
      } // Break loops
    } // particle decayed before the min radius to be considered for simulation
    else {
      ATH_MSG_VERBOSE( "Particle " << particle << " was produced and decayed within a radius of " << m_minDecayRadiusQS << " mm.");
    }
  } // particle had daughters
  if (!m_useShadowEvent && !particle->end_vertex() && particle->status()==2) { // no daughters... No end vertex... Check if this isn't trouble
    ATH_MSG_ERROR( "Found a particle with no end vertex that does not appear in the white list." );
    ATH_MSG_ERROR( "This is VERY likely pointing to a problem with either the configuration you ");
    ATH_MSG_ERROR( "are using, or a bug in the generator.  Either way it should be fixed.  The");
    ATH_MSG_ERROR( "particle will come next, and then we will throw.");
    ATH_MSG_ERROR( particle );
    throw std::runtime_error("GenParticleSimWhiteList: Particle with no end vertex and not in whitelist");
  }

  return passFilter;
}
#else
bool ISF::GenParticleSimWhiteList::pass(const HepMC::GenParticle& particle , std::vector<int> & used_vertices ) const
{
  // See if the particle is in the white list
  bool passFilter = std::binary_search( m_pdgId.begin() , m_pdgId.end() , particle.pdg_id() ) || MC::isNucleus( particle.pdg_id() );
  // Remove documentation particles
  passFilter = passFilter && particle.status()<3;
  // Test all daughter particles
  if (particle.end_vertex() && m_qs && passFilter){
    // Primarily interested in passing particles decaying outside
    // m_minDecayRadiusQS (nomimally the inner radius of the
    // beampipe). However, it is also interesting to pass particles
    // which start outside m_minDecayRadiusQS, but decay inside it.
    passFilter = passFilter && ( (m_minDecayRadiusQS < particle.end_vertex()->position().perp()) || (m_minDecayRadiusQS < particle.production_vertex()->position().perp()) );
    if (passFilter) {
      // Break loops
      if ( std::find( used_vertices.begin() , used_vertices.end() , particle.end_vertex()->barcode() )==used_vertices.end() ){
        used_vertices.push_back( particle.end_vertex()->barcode() );
        for (HepMC::GenVertex::particle_iterator it = particle.end_vertex()->particles_begin(HepMC::children);
             it != particle.end_vertex()->particles_end(HepMC::children); ++it){
          passFilter = passFilter && pass( **it , used_vertices );
          if (!passFilter) {
            ATH_MSG_VERBOSE( "Daughter particle " << **it << " does not pass." );
            break;
          }
        } // Loop over daughters
      } // Break loops
    } // particle decayed before the min radius to be considered for simulation
    else {
      ATH_MSG_VERBOSE( "Particle " << particle << " was produced and decayed within a radius of " << m_minDecayRadiusQS << " mm.");
    }
  } // particle had daughters
  if (!m_useShadowEvent && !particle.end_vertex() && particle.status()==2) { // no daughters... No end vertex... Check if this isn't trouble
    ATH_MSG_ERROR( "Found a particle with no end vertex that does not appear in the white list." );
    ATH_MSG_ERROR( "This is VERY likely pointing to a problem with either the configuration you ");
    ATH_MSG_ERROR( "are using, or a bug in the generator.  Either way it should be fixed.  The");
    ATH_MSG_ERROR( "particle will come next, and then we will throw.");
    ATH_MSG_ERROR( particle );
    throw std::runtime_error("GenParticleSimWhiteList: Particle with no end vertex and not in whitelist");
  }

  return passFilter;
}
#endif

StatusCode  ISF::GenParticleSimWhiteList::finalize()
{
    ATH_MSG_VERBOSE("Finalizing ...");
    return StatusCode::SUCCESS;
}
