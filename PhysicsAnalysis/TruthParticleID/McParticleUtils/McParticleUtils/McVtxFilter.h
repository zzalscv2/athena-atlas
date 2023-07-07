/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// McVtxFilter.h 
// Header file for class McVtxFilter
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLEUTILS_MCVTXFILTER_H 
#define MCPARTICLEUTILS_MCVTXFILTER_H 

/** McVtxFilter allows to select HepMC::GenParticle based on the decay pattern.
 *  Hopefully one will be able to also select the particles by pt (in a coming
 *  patch).
 *  It uses internally the DecayPattern class to build the filtering decision
 *  The basic <i>modus operandi</i> is to loop on HepMC::GenVertex (of a given
 *  HepMC::GenEvent) to select the ones which are of interest (and fulfill 
 *  the decay pattern).
 */

// STL includes
#include <sstream>
#include <ostream>

// Framework includes
#include "AthenaBaseComps/AthMessaging.h"
#include "AthContainers/DataVector.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

// AnalysisUtils includes
#include "AnalysisUtils/IFilterCuts.h"
#include "AnalysisUtils/ParticleCandidateList.h"

// McParticleUtils includes
#include "McParticleUtils/DecayParser.h"

// Forward declaration

class McVtxFilter : virtual public IFilterCuts, public AthMessaging
{ 
  
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
  public: 
  
  /** Default constructor: 
   */
  McVtxFilter();
  
  /** Copy constructor: 
   */
  McVtxFilter( const McVtxFilter& rhs );
  
  /** Constructor with parameters: 
   */
  McVtxFilter( const std::string& decayPattern,
	       const bool matchSign = false, 
	       const bool matchBranches = false );
  
  /** Constructor with parameters: 
   */
  McVtxFilter( const bool matchSign, const bool matchBranches );
  
  /** Destructor: 
   */
  virtual ~McVtxFilter(); 
  
  /** Assignment operator: 
   */
  McVtxFilter &operator=(const McVtxFilter &rhs); 
  
  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** Main filter method. This method takes the decision whether or not
   *  the filter has to accept the vertex.
   *  It checks if the vertex has the required number of ingoing particles
   *  the required number of outgoing particles. Then for each branch (parent
   *  child) it checks if the particle can match an asked for particle.
   */
  virtual bool isAccepted( HepMC::ConstGenVertexPtr vtx ) const;

  /// Dump to std::ostream (default = std::cout) the decay pattern the 
  /// filter one is looking for
  void dump( std::ostream& out = std::cout ) const;

  /** returns true if the vertex filter requires a parent AND a child list
   *  <i>ie:</i> returns false if the filter looks only for a lonely particle
   *  (eg: a photon or an electron, see TruthParticleAlgs for a concrete example)
   */
  bool isFullVtx() const;

  /** return true if the filter discriminates the sign of the particles
   */
  bool matchSign() const;

  /** return true if the filter strictly matchs the number of output particles
   *  eg: a vertex t->Wbgg won't be selected if matchBranches == true and
   *      decay pattern = "#id 6[24,5]"
   */
  bool matchBranches() const;

  /** return the decay pattern used by this filter
   */
  const std::string& decayPattern() const;

  /** return the list of particle candidates for the input particles (parent)
   */
  const DataVector<const ParticleCandidateList>& parentList() const;
    
  /** return the list of particle candidates for the output particles (child)
   */
  const DataVector<const ParticleCandidateList>& childList() const;
    
  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** Set filter cut options
   */
  virtual void setFilter( const IFilterCuts * filter );
  
  /**Set if one wants to discriminate the sign of the particles
   */
  void setMatchSign( const bool matchSign );
  
  /**Set if one wants to strictly match the number of output particles
   */
  void setMatchBranches( const bool matchVtxBranches );
  
  /**Set the decay pattern to look for : it correctly setups the parent 
   * and children according to the LaTeX-like string which is given as
   * argument
   */
  void setDecayPattern( const std::string& decayPattern );

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 
  protected: 
  
  /**Check if the parent branch fulfills the requirements
   */
  bool checkParentBranch( HepMC::ConstGenVertexPtr vtx ) const;

  /**Check if the child branch fulfills the requirements
   */
  bool checkChildBranch ( HepMC::ConstGenVertexPtr vtx ) const;
  
  /// Check the branches for the special case of a 1->2 body decay
  /// (there is some room for optimisation in that particular case)
  bool checkTwoBodyDecay( HepMC::ConstGenVertexPtr vtx ) const;
  
  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
  protected: 

  /// Discrimination between particles and conjugates
  bool m_matchSign;
  
  ///  Tell if one wants a strict vertex branches matching.
  ///  This is to cope with radiations in the output branches
  ///  If true then a decay t->Wbggg will not be kept if one asked for
  ///  a top into W+b decay...
  ///  Default : false
  bool m_matchBranches;

  /// The decay pattern one looks for.
  /// Ex: 23 -> -11 + 11 to model a Z0 -> e+ e- decay
  std::string m_decayPattern;
  
  /// The list of the matching particles Ids : each item in DataVector
  /// stands for an entering branch to the vertex
  DataVector<const ParticleCandidateList> m_parentList;
  
  /// The list of the matching particles Ids : each item in DataVector
  /// stands for an out-going branch to the vertex
  DataVector<const ParticleCandidateList> m_childList;
  
}; 

/// I/O operators
//////////////////////
MsgStream& operator<<( MsgStream & msg, const McVtxFilter &obj );
std::ostream& operator<<( std::ostream& out, const McVtxFilter &obj );

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline McVtxFilter::~McVtxFilter(){}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

inline bool McVtxFilter::isFullVtx() const
{
  if ( m_parentList.empty() || m_childList.empty() ) {
    return false;
  } else {
    return true;
  }
}

inline bool McVtxFilter::matchSign() const
{
  return m_matchSign;
}

inline bool McVtxFilter::matchBranches() const
{
  return m_matchBranches;
}

inline const std::string& McVtxFilter::decayPattern() const
{
  return m_decayPattern;
}

inline 
const DataVector<const ParticleCandidateList>& McVtxFilter::parentList() const
{
  return m_parentList;
}

inline    
const DataVector<const ParticleCandidateList>& McVtxFilter::childList() const
{
  return m_childList;
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

inline void McVtxFilter::setMatchSign( const bool matchSign )
{
  m_matchSign = matchSign;
}

inline void McVtxFilter::setMatchBranches( const bool matchBranches )
{
  m_matchBranches = matchBranches;
}

#endif //> MCPARTICLEUTILS_MCVTXFILTER_H
