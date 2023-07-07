/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// ParticleCandidateList.cxx 
// Implementation file for class ParticleCandidateList
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "GaudiKernel/GaudiException.h"

// AnalysisUtils includes
#include "AnalysisUtils/ParticleCandidateList.h"
#include "TruthUtils/HepMCHelpers.h"


ParticleCandidateList::ParticleCandidateList()
  : AthMessaging ("ParticleCandidateList")
{}

ParticleCandidateList::ParticleCandidateList( const ParticleCandidateList& rhs)
  : AthMessaging ("ParticleCandidateList"),
    m_list(rhs.m_list)
{}


bool ParticleCandidateList::hasInList( const int& pdgID,
                                       const bool tightMatch ) const
{
  ATH_MSG_VERBOSE( "In ParticleCandidateList::hasInList( pdgID ) : " << this
                   << endmsg
                   << "Size of List = " << size() );
  if ( size() <= unsigned(0) ) {
    static const std::string error( "List size <=0 !!" );
    ATH_MSG_ERROR( error );
    throw GaudiException( error, "ParticleCandidateList", 
			  StatusCode::FAILURE );
  }

  for ( auto itrPart = m_list.begin();
	itrPart != m_list.end();
	++itrPart ) {//> Loop over the list of intern particles
    ATH_MSG_VERBOSE( "in loop over list of intern particle candidates"
                     << "\t>>>Comparing pid-s..." );
    if ( tightMatch && ( (*itrPart) == pdgID ) ) {
      ATH_MSG_VERBOSE( ">>> " << pdgID << " is in list (" << (*itrPart) << ")" );
      return true;
    } else if ( !tightMatch && 
		std::abs( (*itrPart) ) == std::abs( pdgID ) ) {
      return true;
    }
  }//> end loop over the list of intern particles

  return false;
}

void ParticleCandidateList::dropList() const
{
  ATH_MSG_VERBOSE("---------------------------------------------------\n"
                  << "Added those particles : " );
  for ( auto itrPart = m_list.begin();
	itrPart != m_list.end();
	++itrPart ) {
    ATH_MSG_VERBOSE( "\tpdgID= " << (*itrPart) );
  }

  ATH_MSG_VERBOSE ("---------------------------------------------------" );
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 
void ParticleCandidateList::addLeptons()
{
  m_list.push_back( MC::ELECTRON   ); //>e-
  m_list.push_back( MC::POSITRON    ); //>e+
  m_list.push_back( MC::NU_E      ); //>nu_e
  m_list.push_back( -MC::NU_E ); //>nu_e_bar

  m_list.push_back( MC::MUON  ); //>mu-
  m_list.push_back( -MC::MUON   ); //>mu+
  m_list.push_back( MC::NU_MU     ); //>nuMu
  m_list.push_back( -MC::NU_MU ); //>nuMu_bar

  m_list.push_back( MC::TAU  ); //>tau-
  m_list.push_back( -MC::TAU   ); //>tau+
  m_list.push_back( MC::NU_TAU     ); //>nuTau
  m_list.push_back( -MC::NU_TAU ); //>nuTau_bar
}

void ParticleCandidateList::addLightJets()
{
  /// To cope with Full Reconstruction scheme :
  /// a light-jet, is a jet which could not have been tagged
  /// One has also to add PDG::null to the matching list
  m_list.push_back( 0 ); //> untagged jet : so taken light

  m_list.push_back( MC::DQUARK      ); //> d
  m_list.push_back( -MC::DQUARK ); //> d_bar
  m_list.push_back( MC::UQUARK      ); //> u
  m_list.push_back( -MC::UQUARK ); //> u_bar
  m_list.push_back( MC::CQUARK      ); //> c
  m_list.push_back( -MC::CQUARK ); //> c_bar
  m_list.push_back( MC::SQUARK      ); //> s
  m_list.push_back( -MC::SQUARK ); //> s_bar
}

void ParticleCandidateList::addLightQuarks()
{
  m_list.push_back( MC::DQUARK      ); //> d
  m_list.push_back( -MC::DQUARK ); //> d_bar
  m_list.push_back( MC::UQUARK      ); //> u
  m_list.push_back( -MC::UQUARK ); //> u_bar
  m_list.push_back( MC::CQUARK      ); //> c
  m_list.push_back( -MC::CQUARK ); //> c_bar
  m_list.push_back( MC::SQUARK      ); //> s
  m_list.push_back( -MC::SQUARK ); //> s_bar
}

void ParticleCandidateList::add( const std::string& list )
{
  ATH_MSG_VERBOSE( "add( " << list << " )" );
  if ( list == "LightQuarks" )    addLightQuarks();
  else if ( list == "BQuark" )    addBQuark();
  else if ( list == "BbarQuark" ) addBbarQuark();
  else if ( list == "BQuarks" )   addBQuarks();
  else if ( list == "Bjet" ||
	    list == "BJet"      ) addBjet();
  else if ( list == "Leptons" || 
	    list == "leptons" )   addLeptons();
  else if ( list == "W+/-" )      addWBosons();
  else if ( list == "Z0" )        addZBoson();
  else {
    static const std::string error( "Unknown Candidate List Name !!" );
    ATH_MSG_ERROR( error );
    throw GaudiException( error, "ParticleCandidateList", 
			  StatusCode::FAILURE );
  }

  if ( msg().level() <= MSG::VERBOSE ) { dropList(); 
  }

}
