/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// McVtxFilterTool.cxx 
// Implementation file for class McVtxFilterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <algorithm>

// FrameWork includes
#include "Gaudi/Property.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "GeneratorObjects/McEventCollection.h"

// McParticleTools includes
#include "McVtxFilterTool.h"
#include "copyBeamParticles.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////
McVtxFilterTool::McVtxFilterTool( const std::string& type, 
				  const std::string& name, 
				  const IInterface* parent ) : 
  AthAlgTool( type, name, parent )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty( "McEvents",
		   m_mcEventsName = "TruthEvent",
		   "Location of the McEventCollection to be filtered" );

  declareProperty( "McEventsOutput", 
		   m_mcEventsOutputName = "GEN_AOD",
		   "Output McEventCollection location (filtered from the "
		   "McEventCollection)" );

  declareProperty( "DecayPatterns", 
		   m_decayPatterns,
		   "List of decay patterns the tool will try to match. \n\t"
		   "ex: 23 -> -5 + 5 to select Z bosons into bbbar pairs" );

  m_decayPatterns.declareUpdateHandler( &McVtxFilterTool::setupFilters, 
					this );

  declareProperty( "MatchSigns", 
		   m_matchSigns,
		   "List of booleans with which the tool will setup "
		   "McVtxFilters. This will setup the McVtxFilter::matchSign "
		   "data member." );
  m_matchSigns.declareUpdateHandler( &McVtxFilterTool::setupFilters, 
				     this );

  declareProperty( "MatchBranches", 
		   m_matchBranches,
		   "List of booleans with which the tool will setup "
		   "McVtxFilters. This will setup the "
		   "McVtxFilter::matchBranches data member." );

  m_matchBranches.declareUpdateHandler( &McVtxFilterTool::setupFilters, 
					this );

  declareProperty( "DoSignalProcessVtx", 
		   m_doSignalProcessVtx = true,
		   "Switch to include or not the signal_process_vertex into "
		   "the McEventCollection (default = true).\n" 
		   "Note that this is close to useless as the HEPEVT structure"
		   " does not hold this kind of informations..." );

  declareProperty( "FillTree",        
		   m_fillTree = false,
		   "Switch to fill the entire decay tree from a decay "
		   "vertex.\n"
		   "This can be usefull to record for example the whole shower "
		   "from a gluon or a quark. In that case, you may want to add "
		   "the according PDG ids into the list of particles to keep" );

  declareProperty( "ParticlesToKeep", 
		   m_particles,
		   "List of outgoing particles from the decay vertex to "
		   "include. This property has to be configured in sync. with "
		   "the 'FillTree' one." );

  declareInterface<IMcVtxFilterTool>(this);
}

/// Destructor
///////////////
McVtxFilterTool::~McVtxFilterTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");
}

/// Athena Algorithm's Hooks
////////////////////////////
StatusCode McVtxFilterTool::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  if ( msgLvl(MSG::DEBUG) ) {
    displayOptions();
  }

  // configure the filters with the provided DecayPatterns (from
  // jobO or via the property interface
  setupFilters( m_decayPatterns );

  return StatusCode::SUCCESS;
}

StatusCode McVtxFilterTool::execute()
{  
  StatusCode sc = StatusCode::SUCCESS;

  ATH_MSG_DEBUG("Executing " << name() << "...");

  ATH_MSG_VERBOSE("Retrieve the McEventCollection to be filtered for");
  const McEventCollection * mcColl = nullptr;
  sc = evtStore()->retrieve( mcColl, m_mcEventsName );

  if ( sc.isFailure() || nullptr == mcColl ) {
    msg(MSG::ERROR)
      << "Failed to retrieve McEventCollection at : "
      << m_mcEventsName
      << endmsg
      << "because : sc= " << ( sc.isFailure() ? "FAILURE" : "OK" ) 
      << " and mcEvent= " << mcColl
      << endmsg;
    return StatusCode::SUCCESS;
  } 

  ATH_MSG_VERBOSE("Create a new McEventCollection which will hold the "\
		  "filtered GenEvent");
  McEventCollection * filteredColl = new McEventCollection;
  sc = evtStore()->record(filteredColl, m_mcEventsOutputName, true);
  sc = evtStore()->setConst(filteredColl);
  ATH_MSG_VERBOSE("Recorded and locked in StoreGate");
  
  filterMcEventCollection( mcColl, filteredColl );
  return sc;
}

/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////

void McVtxFilterTool::displayOptions() const
{
  msg(MSG::INFO)
    << "Options for " << name() << " :" << endmsg
    << "\tDecay Patterns: [ ";
  for (const auto & itr : m_decayPatterns.value()) {
    msg(MSG::INFO) << itr << "; ";
  }
  msg(MSG::INFO) << "]" << endmsg;

  msg(MSG::INFO) << "Particles to keep: [";
  for (long itr : m_particles.value()) {
    msg(MSG::INFO) << itr << ", ";
  }
  msg(MSG::INFO) << "]" << endmsg;
}

void McVtxFilterTool::stats() const
{
  msg(MSG::INFO) << "Statistics for each filter : " << endmsg;
  for( unsigned int i = 0; i < m_counter.size(); ++i ) {
    msg(MSG::INFO)
      << "\t==> [" << m_decayPatterns.value()[i] << "] accepted " 
      << m_counter[i] << " vertices" << endmsg;
  }
  }

bool McVtxFilterTool::isAccepted( const HepMC::ConstGenVertexPtr& vtx ) const
{
  for(const auto *filter : m_filters) {
    if ( filter->isAccepted( vtx ) ) {
      return true;
    }
  }//> end loop over McVtxFilters

  return false;
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

void 
McVtxFilterTool::filterMcEventCollection( const McEventCollection* mcColl,
					  McEventCollection* filterColl )
{
  if ( nullptr == mcColl ) {
    ATH_MSG_ERROR("McEventCollection is NULL pointer !!");
    return;
  }

  /// Create a set with ids of copied vertices.
  std::set<int> bcToFullVtx;

  /// Local copy of the GenEvent from StoreGate
  const HepMC::GenEvent * evtSrc = (*mcColl->begin());
  
  /// Create the event container, with Signal Process 
  /// and event number from the GenEvent source
  HepMC::GenEvent * evt = HepMC::copyemptyGenEvent(evtSrc);
  for ( const auto& itrPart: *evtSrc) {
    auto dcyVtx = itrPart->end_vertex();
    if ( !dcyVtx ) continue;
    int vtxBC = HepMC::barcode_or_id(dcyVtx);
    if (bcToFullVtx.count(vtxBC)!=0) continue;
    ATH_MSG_VERBOSE("Doing vtx: " << dcyVtx);

    int i = 0;
    for( DataVector<McVtxFilter>::const_iterator filter = m_filters.begin(); filter != m_filters.end(); ++filter,++i ) {
      ATH_MSG_VERBOSE("Processing with filter[" << i << "]...");
      if ( !(*filter)->isAccepted( dcyVtx ) ) continue;
      m_counter[i] += 1;
      ATH_MSG_VERBOSE("Filter[" << i << "] accepted this vtx : " << dcyVtx);
      bcToFullVtx.insert(vtxBC);
      addVertex( dcyVtx, evt, VtxType::IsRootVertex );
      break;
    }//> end loop over filters
  }//> loop over particles

  if ( m_doSignalProcessVtx ) {
    /// Handling the signal_process_vertex stuff
    /// Signal_process_vertex pointer is not set by all generators, but in
    /// case it has been set up, one should keep this usefull information,
    /// shouldn't we ?
    /// So first we check it is present in the original GenEvent, and then
    /// we check if it as not already added by any previous McVtxFilter (by 
    /// chance or because it was just meant to be)
    auto sigProcVtx = HepMC::signal_process_vertex(evtSrc);
    if ( sigProcVtx ) {
      if ( bcToFullVtx.count(HepMC::barcode_or_id(sigProcVtx)) == 0) {
        addVertex( sigProcVtx, evt,VtxType::IsNotRootVertex, true );
      }//> signal process vertex has to be added
    } else {
      //> Original GenEvent has a NO signal process vertex set-up
      ATH_MSG_DEBUG("You asked to record signal_process_vertex but :" << endmsg << " there is NO signal_process_vertex in this event !!");
    }
  
  } //> end do SignalProcessVtx

  TruthHelper::copyBeamParticles (*evtSrc, *evt);

  filterColl->push_back(evt);
}


/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////

void McVtxFilterTool::addVertex( const HepMC::ConstGenVertexPtr& srcVtx, 
				 HepMC::GenEvent * evt,
				 const VtxType::Flag vtxType, bool isSignal ) const
{
  ATH_MSG_VERBOSE("In McVtxFilterTool::addVertex( vtxType= "<<vtxType<< " )");
#ifdef HEPMC3
  HepMC::GenVertexPtr vtx = (evt == srcVtx->parent_event()) ? std::const_pointer_cast<HepMC3::GenVertex>(srcVtx) : nullptr ;
  if ( !vtx ) {
    vtx = HepMC::newGenVertexPtr();
    evt->add_vertex(vtx);
    vtx->set_position( srcVtx->position() );
    vtx->set_status( srcVtx->status() );
    HepMC::suggest_barcode(vtx, HepMC::barcode(srcVtx) );
    vtx->add_attribute("weights",srcVtx->attribute<HepMC3::VectorDoubleAttribute> ("weights"));
  }
  if (isSignal)  HepMC::set_signal_process_vertex(evt, vtx );
  /// Fill the parent branch
  for ( const auto& parent:  srcVtx->particles_in()) {
    HepMC::GenParticlePtr  mother = (evt == parent->parent_event()) ? std::const_pointer_cast<HepMC3::GenParticle>(parent) : nullptr ;
    if ( ! mother ) {
      mother = HepMC::newGenParticlePtr();
      vtx->add_particle_in( mother );
      mother->set_momentum( parent->momentum() );
      mother->set_generated_mass( parent->generated_mass() );
      mother->set_pdg_id( parent->pdg_id() );
      mother->set_status( parent->status() );
      HepMC::set_flow(mother, HepMC::flow(parent) );
      HepMC::set_polarization(mother, HepMC::polarization(parent) );
	  HepMC::suggest_barcode(mother,HepMC::barcode(parent) );

    } else {
    // set the mother's decay to our (new) vertex
    vtx->add_particle_in( mother );
    }
  }//> loop over ingoing particles
  
  /// Fill the children branch
  for ( const auto& child: srcVtx->particles_out()) {
    HepMC::GenParticlePtr daughter = (evt == child->parent_event()) ? std::const_pointer_cast<HepMC3::GenParticle>(child) : nullptr ;
    if ( !daughter ) {
      if ( !keepParticle( vtxType, child ) ) {
	// only include selected particles via the "ParticlesToKeep" property
	ATH_MSG_VERBOSE("Skipping outgoing particle id|particle: ["
			<< child->pdg_id() << "|" 
			<< child << "]");
      } else {
	daughter = HepMC::newGenParticlePtr();
      // set the daughter's production vertex to our new vertex
        vtx->add_particle_out( daughter );
	daughter->set_momentum( child->momentum() );
        daughter->set_generated_mass( child->generated_mass() );
	daughter->set_pdg_id( child->pdg_id() );
	daughter->set_status( child->status() );
    HepMC::set_flow(daughter, HepMC::flow(child) );
    HepMC::set_polarization(daughter, HepMC::polarization(child) );
	HepMC::suggest_barcode(daughter,HepMC::barcode(child) );

      }
    }
  
    if ( m_fillTree && keepParticle( vtxType, child ) ) {
      auto decayVertex = child->end_vertex();
      if ( decayVertex ) {
	// recursively fill the tree with all decay vertices and final state
	// particles of selected outgoing particles lines
	// => We are no longer sitting at the decay vertex so we tell it
	// via the IsNotRootVertex flag
	addVertex( decayVertex, evt, VtxType::IsNotRootVertex );
      }
    }
  }//> loop over outgoing particles
#else
  HepMC::GenVertex * vtx = evt->barcode_to_vertex(srcVtx->barcode());
  if ( 0 == vtx ) {
    vtx = HepMC::newGenVertexPtr();
    vtx->set_position( srcVtx->position() );
    vtx->set_id( srcVtx->id() );
    vtx->suggest_barcode( srcVtx->barcode() );
    vtx->weights() = srcVtx->weights();
    evt->add_vertex(vtx);
  }
  if (isSignal)  HepMC::set_signal_process_vertex(evt, vtx );
  /// Fill the parent branch
  for ( HepMC::GenVertex::particles_in_const_iterator parent = srcVtx->particles_in_const_begin();
	parent != srcVtx->particles_in_const_end();
	++parent ) {
    HepMC::GenParticle * mother = evt->barcode_to_particle( (*parent)->barcode() );
    if ( 0 == mother ) {
      mother = HepMC::newGenParticlePtr();
      mother->set_momentum( (*parent)->momentum() );
      mother->set_generated_mass( (*parent)->generated_mass() );
      mother->set_pdg_id( (*parent)->pdg_id() );
      mother->set_status( (*parent)->status() );
      mother->set_flow( (*parent)->flow() );
      mother->set_polarization( (*parent)->polarization() );
      mother->suggest_barcode( (*parent)->barcode() );

    }
    // set the mother's decay to our (new) vertex
    vtx->add_particle_in( mother );
    
  }//> loop over ingoing particles
  
  /// Fill the children branch
  for ( HepMC::GenVertex::particles_out_const_iterator child = srcVtx->particles_out_const_begin();
	child != srcVtx->particles_out_const_end();
	++child ) {
    HepMC::GenParticle * daughter = evt->barcode_to_particle( (*child)->barcode() );
    if ( 0 == daughter ) {
      if ( !keepParticle( vtxType, *child ) ) {
	// only include selected particles via the "ParticlesToKeep" property
	ATH_MSG_VERBOSE("Skipping outgoing particle : "<< (*child));
      } else {
	daughter = HepMC::newGenParticlePtr();
	daughter->set_momentum( (*child)->momentum() );
   daughter->set_generated_mass( (*child)->generated_mass() );
	daughter->set_pdg_id( (*child)->pdg_id() );
	daughter->set_status( (*child)->status() );
	daughter->set_flow( (*child)->flow() );
	daughter->set_polarization( (*child)->polarization() );
	daughter->suggest_barcode( (*child)->barcode() );

      }
    }
    if ( daughter ) {
      // set the daughter's production vertex to our new vertex
      vtx->add_particle_out( daughter );
    }

    if ( m_fillTree && keepParticle( vtxType, *child ) ) {
      const HepMC::GenVertex * decayVertex = (*child)->end_vertex();
      if ( 0 != decayVertex ) {
	// recursively fill the tree with all decay vertices and final state
	// particles of selected outgoing particles lines
	// => We are no longer sitting at the decay vertex so we tell it
	// via the IsNotRootVertex flag
	addVertex( decayVertex, evt, VtxType::IsNotRootVertex );
      }
    }
  }//> loop over outgoing particles
#endif

  }

bool McVtxFilterTool::keepParticle( const VtxType::Flag vtxType, 
				    const HepMC::ConstGenParticlePtr& part ) const 
{
  // no particle, so no particle to keep. Simple, isn't ?
  if ( nullptr == part ) {
    return false;
  }

  // By default, we keep all particles
  if ( m_particles.value().empty() ) {
    return true;
  }

  // only filter decay particles of the root vertex
  // ie: the vertex which matches a decay pattern setup by the
  // "DecayPatterns" property.
  if ( vtxType == VtxType::IsNotRootVertex ) {
    return true;

  } else if ( vtxType == VtxType::IsRootVertex ) {
    // we are at a Root vertex.
    // And we only keep the outgoing particles which are in our list
    const int pdgId = part->pdg_id();
    return std::find( m_particles.value().begin(), 
		    m_particles.value().end(), 
		    pdgId ) != m_particles.value().end();
  } else {
    
    // Humm.. we don't know anything about this VtxType...
    // by default we keep all particles
    // But we tell user that something is wrong !
    msg(MSG::WARNING)
      << "In keepParticle: Don't know anything about this VtxType ["
      << vtxType << "] !!"
      << endmsg
      << "We'll keep this particle [ " << part 
      << "] but : Check your jobOption !!"
      << endmsg;
    return true;
  }
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

void McVtxFilterTool::setupFilters( Gaudi::Details::PropertyBase& /*decayPatterns*/ )
{
  m_filters.clear();
  m_counter.clear();
  
  const std::vector<std::string>& decayPatterns = m_decayPatterns.value();
  std::vector<bool> matchSigns    = m_matchSigns.value();
  std::vector<bool> matchBranches = m_matchBranches.value();

  // Normally, using the Configurables, the lengths of properties should
  // be the same... but who knows. Better test it.
  // On the other hand, for compatibility reasons, we allow the situation
  // where *both* matchSigns and matchBranches are empty.
  // Only after having removed the "old-style" case we can raise an exception.
  const bool oldCfg = ( m_matchSigns.value().empty() &&
			m_matchBranches.value().empty() );
  if ( !oldCfg && 
       ( decayPatterns.size() != matchSigns.size() ||
	 decayPatterns.size() != matchBranches.size() ||
	 matchSigns.size()    != matchBranches.size() ) ) {
    std::string error;
    error  = "Lengths of ArrayProperty configuring McVtxFilters ";
    error += "are DIFFERING !!";
    msg(MSG::WARNING)
      << "In setupFilter: " << error 
      << endmsg
      << "\t# decayPatterns: " << decayPatterns.size() << endmsg
      << "\t# matchSigns:    " << matchSigns.size()    << endmsg
      << "\t# matchBranches: " << matchBranches.size() << endmsg
      << "Reference size is 'decayPatterns':" << endmsg
      << "\t==> Will fill holes with default values !" << endmsg
      << "\t==> Will drop extra values !"
      << endmsg;
    //throw std::runtime_error(error);
  }

  const unsigned int nDecayPatterns = decayPatterns.size();
  m_filters.resize( nDecayPatterns );
  m_counter.resize( nDecayPatterns, 0 );

  if ( nDecayPatterns > matchSigns.size() ) {
    // fill it  with default values
    const unsigned int nCount = nDecayPatterns - matchSigns.size();
    std::fill_n( std::back_inserter(matchSigns),
		 nCount, // number of times we fill it
		 false );// default value
  }

  if ( nDecayPatterns > matchBranches.size() ) {
    // fill it  with default values
    const unsigned int nCount = nDecayPatterns - matchBranches.size();
    std::fill_n( std::back_inserter(matchBranches),
		 nCount, // number of times we fill it
		 false );// default value
  }

  for ( unsigned int i = 0; i != nDecayPatterns; ++i ) {
    m_filters[i] = new McVtxFilter( decayPatterns[i],
				    bool(matchSigns[i]),
				    bool(matchBranches[i]) );
  }

  }
