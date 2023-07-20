/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// McVtxFilter.cxx 
// Implementation file for class McVtxFilter
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

// AnalysisUtils includes
#include "AnalysisUtils/AnalysisPermutation.h"

// McParticleUtils includes
#include "McParticleUtils/McVtxFilter.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////
McVtxFilter::McVtxFilter() : 
  IFilterCuts(),
  AthMessaging ( "McVtxFilter" ),
  m_matchSign( false ),
  m_matchBranches( false ),
  m_decayPattern( "" ),
  m_parentList(),
  m_childList()
{}

McVtxFilter::McVtxFilter( const std::string& decayPattern,
			  const bool matchSign, 
			  const bool matchBranches ) : 
  IFilterCuts(),
  AthMessaging ( "McVtxFilter" ),
  m_matchSign( matchSign ),
  m_matchBranches( matchBranches ),
  m_decayPattern( decayPattern ),
  m_parentList(),
  m_childList()
{
  setDecayPattern(decayPattern);
}

McVtxFilter::McVtxFilter( const bool matchSign, 
			  const bool matchBranches ) : 
  IFilterCuts(),
  AthMessaging ( "McVtxFilter" ),
  m_matchSign( matchSign ),
  m_matchBranches( matchBranches ),
  m_decayPattern( "" ),
  m_parentList(),
  m_childList()
{}

McVtxFilter::McVtxFilter( const McVtxFilter& rhs ) : 
  IFilterCuts    ( rhs                 ),
  AthMessaging ( "McVtxFilter" ),
  m_matchSign    ( rhs.m_matchSign     ),
  m_matchBranches( rhs.m_matchBranches ),
  m_decayPattern ( rhs.m_decayPattern  ),
  m_parentList(),
  m_childList()
{
  // deep copy of the parent branch
  m_parentList.reserve( rhs.m_parentList.size() );
  for ( DataVector<const ParticleCandidateList>::const_iterator itr = 
	  rhs.m_parentList.begin();
	itr != rhs.m_parentList.end();
	++itr ) {
    m_parentList.push_back( new ParticleCandidateList(**itr) );
  }

  // deep copy of the child branch
  m_childList.reserve( rhs.m_childList.size() );
  for ( DataVector<const ParticleCandidateList>::const_iterator itr = 
	  rhs.m_childList.begin();
	itr != rhs.m_childList.end();
	++itr ) {
    m_childList.push_back( new ParticleCandidateList(**itr) );
  }
}

McVtxFilter & McVtxFilter::operator=(const McVtxFilter &rhs)
{
  if ( this != &rhs ) {
    IFilterCuts::operator=(rhs);
    m_matchSign     = rhs.m_matchSign;
    m_matchBranches = rhs.m_matchBranches;
    m_decayPattern  = rhs.m_decayPattern;

    // deep copy of the parent branch
    m_parentList.reserve( rhs.m_parentList.size() );
    for ( DataVector<const ParticleCandidateList>::const_iterator itr = 
	    rhs.m_parentList.begin();
	  itr != rhs.m_parentList.end();
	  ++itr ) {
      m_parentList.push_back( new ParticleCandidateList(**itr) );
    }

    // deep copy of the child branch
    m_childList.reserve( rhs.m_childList.size() );
    for ( DataVector<const ParticleCandidateList>::const_iterator itr = 
	    rhs.m_childList.begin();
	  itr != rhs.m_childList.end();
	  ++itr ) {
      m_childList.push_back( new ParticleCandidateList(**itr) );
    }
  }
  return *this;
}


/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////
bool McVtxFilter::isAccepted( HepMC::ConstGenVertexPtr vtx ) const
{
  ATH_MSG_VERBOSE("In McVtxFilter::isAccepted(...)");
//AV TODO: add here a check to prevent null pointer dereference
  unsigned int number_particles_out = vtx->particles_out_size();
  unsigned int number_particles_in  = vtx->particles_in_size();
  ////////////////////////////////////////////////////////////////
  /// First handle special case where the filter has only 1 child
  /// and no parent : one is looking for a stable particle with
  /// no end_vertex
  if ( m_childList.size()        == static_cast<unsigned int>( 1 ) &&
       m_parentList.size()       == static_cast<unsigned int>( 0 ) &&
       number_particles_out == static_cast<unsigned int>( 1 ) ) {
#ifdef HEPMC3
    const HepMC::ConstGenParticlePtr& part = vtx->particles_out().front();
#else
    HepMC::ConstGenParticlePtr part = *(vtx->particles_out_const_begin());
#endif
    const ParticleCandidateList * item = *( m_childList.begin() );
    if ( item->hasInList( part->pdg_id(),  m_matchSign ) ) {
      return true;
    } else { 
      return false;
    }
  }

  /// Skip vertices which don't match the number of branches
  /// we are looking for : check if number of parents 
  /// and of children is OK
  if ( !m_matchBranches ) {

    //
    // Test only if there is enough *output* branches to match for
    //
    if ( number_particles_in  < m_parentList.size() ||
	 number_particles_out < m_childList.size()  ) {
      return false;
    }

  } else {
    //
    // Strict match of output branches
    //
    if ( number_particles_in  != m_parentList.size() ||
	 number_particles_out != m_childList.size()  ) {
      return false;
    }
  }

  //////////////////////////////////////////////////////////////
  /// Then handle the case we are looking for a 1->2 body decay
  ///
  if ( m_matchBranches                                           &&
       m_parentList.size()       == static_cast<unsigned int>(1) &&
       m_childList.size()        == static_cast<unsigned int>(2) &&
       number_particles_out >= 2 ) {
    return checkTwoBodyDecay( vtx );
  } //> two-body decay


  ////////////////////////////////
  /// Handle other generic cases
  ///
  ATH_MSG_VERBOSE("trying checkParentBranch(...)");
  if ( checkParentBranch( vtx ) == false ) return false;

  ATH_MSG_VERBOSE("trying checkChildBranch(...)");
  if ( checkChildBranch ( vtx ) == false ) return false;

  ATH_MSG_VERBOSE("McVtxFilter::isAccepted(...) => DONE");
  return true;
}

void McVtxFilter::dump( std::ostream& out ) const
{
  out << ">>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<" << std::endl;
  out << ">>> Parents : " << std::endl;
  for ( DataVector<const ParticleCandidateList>::const_iterator itr = m_parentList.begin();
	itr != m_parentList.end();
	++itr ) {
    out << "\nList for : " << *itr  
	<< " (" << (*itr)->size() << ")" << std::endl;
    (*itr)->dropList();
  }

  out << ">>> Children : " << std::endl;
  for ( DataVector<const ParticleCandidateList>::const_iterator itr = m_childList.begin();
	itr != m_childList.end();
	++itr ) {
    out << "\nList for : " << *itr  
	<< " (" << (*itr)->size() << ")" << std::endl;
    (*itr)->dropList();
  }
  out << ">>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<" << std::endl;

  return;
}


/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/// Set filter cut options
void McVtxFilter::setFilter( const IFilterCuts * filter )
{
  if ( filter ) {
    const McVtxFilter * vtxFilter = 
      dynamic_cast<const McVtxFilter*>(filter);
    
    if ( vtxFilter ) {
      operator=(*vtxFilter);
    } else {
      ATH_MSG_ERROR("Can't dynamic_cast " << typeid(filter).name() << " to a McVtxFilter");
    }
  } //> filter is a valid pointer
}

/// Set the filter from a decay pattern
void McVtxFilter::setDecayPattern( const std::string& decayPattern )
{
  // clear parent and child candidates
  m_parentList.clear();
  m_childList.clear();

  m_decayPattern = decayPattern;
  
  DecayParser parser( m_decayPattern );
  
  std::vector<std::vector<std::string> > parents = parser.getParents();
  for(std::vector<std::vector<std::string> >::const_iterator itr = parents.begin();
      itr != parents.end();
      ++itr ) {
    ParticleCandidateList * list = new ParticleCandidateList();
    const std::vector<std::string>::const_iterator candEnd  = itr->end();
    for( std::vector<std::string>::const_iterator candidate = itr->begin();
	 candidate != candEnd;
	 ++candidate ) {
      int pdgID = parser.pdgId( *candidate );
      list->push_back( pdgID );
    }
    if ( ! list->empty() ) {
      m_parentList.push_back( list );
    } else {
      delete list;
    }
  }
  std::vector<std::vector<std::string> > children = parser.getChildren();
  for(std::vector<std::vector<std::string> >::const_iterator itr = children.begin();
      itr != children.end();
      ++itr ) {
    ParticleCandidateList * list = new ParticleCandidateList();
    const std::vector<std::string>::const_iterator candEnd  = itr->end();
    for( std::vector<std::string>::const_iterator candidate = itr->begin();
	 candidate != candEnd;
	 ++candidate ) {
      int pdgID = parser.pdgId( *candidate );
      list->push_back( pdgID );
    }
    if ( ! list->empty() ) {
      m_childList.push_back( list );
    } else {
      delete list;
    }
  }

  return;
}


/////////////////////////////////////////////////////////////////// 
/// Protected methods: 
/////////////////////////////////////////////////////////////////// 

bool McVtxFilter::checkParentBranch( HepMC::ConstGenVertexPtr vtx ) const
{
  ATH_MSG_VERBOSE("In checkParentBranch...");

  /// Check we aren't in the "any particle" case
  if ( m_parentList.empty() ) {
    return true;
  }
  
  if ( msgLvl(MSG::VERBOSE) ) {
    HepMC::Print::line(std::cout,vtx);
  }

  /// Check if number of parents is OK
  if ( static_cast<unsigned int>(vtx->particles_in_size()) < m_parentList.size() ){
    return false;
  }

  if ( msgLvl(MSG::VERBOSE) ) {
    msg() << MSG::VERBOSE
          << "Number of list of parents : "
          << m_parentList.size()
          << endmsg;
    m_parentList.front()->dropList();
  }

  std::vector<int> parentIds;
#ifdef HEPMC3
  for ( const auto& Part: vtx->particles_in() ) {
    parentIds.push_back( Part->pdg_id() );
  }
#else
  for ( HepMC::GenVertex::particles_in_const_iterator itrPart = vtx->particles_in_const_begin();
	itrPart != vtx->particles_in_const_end();
	++itrPart ) {
    parentIds.push_back( (*itrPart)->pdg_id() );
  }
#endif

  AnalysisUtils::Permutation<std::vector<int> > permute( &parentIds, m_parentList.size() );
  std::vector<int> parents;

  bool accepted = false;
  while ( permute.get(parents) && !accepted ) {
    accepted = true;
    const unsigned int iMax = std::min( m_parentList.size(), parentIds.size() );
    for ( unsigned int i = 0; i != iMax; ++i ) {
      const bool hasInList = m_parentList[i]->hasInList( parents[i],m_matchSign );
      if ( !hasInList ) {
	// this permutation is not suiting, going to the next one (if any)
	accepted = false;
	break;
      }
    }

    // everything went fine for this permutation !
    if ( accepted ) {
      break;
    }
  }

  ATH_MSG_VERBOSE(">>> CheckParentBranch is DONE : "
                  << ( accepted ? "accept" : "reject" )
                  << " vtx= " << vtx);
  return accepted;
}

bool McVtxFilter::checkChildBranch( HepMC::ConstGenVertexPtr vtx ) const
{
  ATH_MSG_VERBOSE("In checkChildBranch...");

  if ( msgLvl(MSG::VERBOSE) ) {
    HepMC::Print::line(std::cout,vtx);
  }

  /// Check we aren't in the "any particle" case
  if ( m_childList.empty() ) {
    return true;
  }

  /// Check there is enough outgoing particles in the current vertex
  if ( static_cast<unsigned int>(vtx->particles_out_size()) < m_childList.size() ) return false;
  ATH_MSG_VERBOSE("Number of list of children : " << m_childList.size());

  std::vector<int> childIds;
  for ( auto Part: *vtx) {
    childIds.push_back( Part->pdg_id() );
  }

  AnalysisUtils::Permutation<std::vector<int> > permute( &childIds, m_childList.size() );
  std::vector<int> children;

  bool accepted = false;
  while ( permute.get(children) && !accepted ) {
    accepted = true;
    const unsigned int iMax = std::min( m_childList.size(), childIds.size() );
    for ( unsigned int i = 0; i != iMax; ++i ) {
      const bool hasInList = m_childList[i]->hasInList( children[i], m_matchSign );
      if ( !hasInList ) {
	// this permutation is not suiting, going to the next one (if any)
	accepted = false;
	break;
      }
    }
  }

  ATH_MSG_VERBOSE(">>> CheckChildBranch is DONE : "
                  << ( accepted ? "accept" : "reject" )
                  << " vtx= " << vtx);
  return accepted;
}

bool McVtxFilter::checkTwoBodyDecay( HepMC::ConstGenVertexPtr vtx ) const
{
  ATH_MSG_VERBOSE("In checkTwoBodyDecay...");

  /// First check the parent branch matching decision
  /// if it doesn't fulfil our requirements, it is not worth
  /// continuing the process
  const bool parentTree = checkParentBranch( vtx );
  if ( parentTree == false ) {
    return false;
  }

  /// Now, handle the child branch
  ATH_MSG_VERBOSE(">>> Check child branch");

  /// Cache the 2 particle candidate lists
  const ParticleCandidateList * children1 = m_childList[0];
  const ParticleCandidateList * children2 = m_childList[1];

  /// Cache the id of the outgoing particles of the vertex being analysed
//AV It would be a very good idea to have a chack of the number of output particles here.
#ifdef HEPMC3
  const int pdgId1= vtx->particles_out().at(0)->pdg_id();
  const int pdgId2= vtx->particles_out().at(1)->pdg_id();
#else
  HepMC::GenVertex::particles_out_const_iterator itrPart = vtx->particles_out_const_begin();
  const int pdgId1 = (*itrPart)->pdg_id();
  ++itrPart;
  const int pdgId2 = (*itrPart)->pdg_id();
#endif

  /// Loop over candidates for the 1st child
  for( ParticleCandidateList::const_iterator itr1 = children1->begin();
       itr1 != children1->end();
       ++itr1 ) {
    /// Loop over candidate for the 2nd child
    for( ParticleCandidateList::const_iterator itr2 = children2->begin();
	 itr2 != children2->end();
	 ++itr2 ) {
      ATH_MSG_VERBOSE("Checking the pair : " << (*itr1) << "/" << (*itr2));

      /// If the strict match sign has been required, we check if
      /// the PDG ids are matching
      if ( m_matchSign && 
	   ( ( (*itr1) == pdgId1 && (*itr2) == pdgId2 ) ||
	     ( (*itr1) == pdgId2 && (*itr2) == pdgId1 ) ) ) {
	ATH_MSG_VERBOSE("Strict sign matching found !");
	return true;
      /// if we are in a loose sign request, we check only that the
      /// absolute values of the pdg IDs are matching
      } else if ( ( std::abs(*itr1) == std::abs(pdgId1) && 
		    std::abs(*itr2) == std::abs(pdgId2) )   ||
		  ( std::abs(*itr1) == std::abs(pdgId2) && 
		    std::abs(*itr2) == std::abs(pdgId1) ) ) {
	ATH_MSG_VERBOSE("Loose sign matching found !");
	return true;
      }
      ATH_MSG_VERBOSE("Checking next pair");
    }//> loop over 2nd child's candidates
  }//> loop over 1st child's candidates

  /// If we are here, then the vertex candidate didn't fulfil the 
  /// requirements we have setup
  ATH_MSG_VERBOSE(">>> CheckTwoBodyDecay is DONE.");
  return false;
}

/////////////////////////////////////////////////////////////////// 
// Operators 
/////////////////////////////////////////////////////////////////// 

MsgStream& operator<<( MsgStream & msg, const McVtxFilter &obj )
{
  std::stringstream out;
  obj.dump( out );
  msg << out.str() << endmsg;
  return msg;
}

std::ostream& operator<<( std::ostream& out, const McVtxFilter &obj )
{
  obj.dump( out );
  return out;
}
