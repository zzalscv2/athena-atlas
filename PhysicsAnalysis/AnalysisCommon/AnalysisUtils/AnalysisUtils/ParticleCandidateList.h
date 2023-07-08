/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// ParticleCandidateList.h 
// Header file for class ParticleCandidateList
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ANALYSISUTILS_PARTICLECANDIDATELIST_H 
#define ANALYSISUTILS_PARTICLECANDIDATELIST_H 

/** A simple wrapper for std::list<int> to model a list of particle
 *  identity candidates.
 *  This candidate list is used by the PdgIdFilter and McVtxFilter to select 
 *  for particles which might fulfill some criterion (well in our case this is
 *  its Particle Data Group identity.
 */

// STL includes
#include <list>

// EventKernel includes
#include "TruthUtils/HepMCHelpers.h"
#include "AthenaBaseComps/AthMessaging.h"

// Gaudi includes

class ParticleCandidateList
  : public AthMessaging
{ 

  /////////////////////////////////////////////////////////////////// 
  // typedefs:
  /////////////////////////////////////////////////////////////////// 
 public:

  ///\name list typedefs: it behaves like a std::list<int>
  //@{
  typedef std::list<int>::iterator iterator;
  typedef std::list<int>::const_iterator const_iterator;
  typedef std::list<int>::size_type size_type;
  //@}

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  ParticleCandidateList();

  /** Copy constructor: 
   */
  ParticleCandidateList( const ParticleCandidateList& rhs );

  // Constructor with parameters: 

  /** Destructor: 
   */
  virtual ~ParticleCandidateList(); 

  /** Assignment operator: 
   */
  ParticleCandidateList& operator=( const ParticleCandidateList& rhs ); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** Tells if a given particle or pID is in the list
   *  if tightMatch = false : look only if std::abs() of
   *  particle data group id-s matches
   */
  bool hasInList( const int& pdgID, 
                  const bool tightMatch = false ) const;
  void dropList() const;

  /** Return the wrapped STL list
   */
  const std::list<int>& list() const;

  /// \name forward some of the std::list<int> const methods
  //@{
  const_iterator begin()  const;
  const_iterator end()    const;

  bool      empty()    const;
  size_type size()     const;
  size_type max_size() const;

  //@}

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** forward the std::list<int>::push_back() method
   */
  void push_back(const int& id);

  /** forward the std::list<int>::clear() method
   */
  void clear();

  void addLeptons();
  void addLightJets();
  void addLightQuarks();
  void addBQuark();
  void addBbarQuark();
  void addBQuarks();
  void addBjet();
  void addWBosons();
  void addZBoson();
  void add( const int& partID );
  void add( const std::string& listOfParticlesName = "LightQuarks" );

 protected: 
  std::list<int> m_list;
}; 


inline ParticleCandidateList::~ParticleCandidateList() 
{}

inline 
ParticleCandidateList&
ParticleCandidateList::operator=( const ParticleCandidateList& rhs )
{
  if ( this != &rhs ) {
    m_list = rhs.m_list;
  }
  return *this;
}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
/////////////////////////////////////////////////////////////////// 
inline const std::list<int>& ParticleCandidateList::list() const
{
  return m_list;
}

inline ParticleCandidateList::const_iterator ParticleCandidateList::begin()  const
{
  return m_list.begin();
}

inline ParticleCandidateList::const_iterator ParticleCandidateList::end()    const
{
  return m_list.end();
}

inline bool ParticleCandidateList::empty() const
{
  return m_list.empty();
}

inline ParticleCandidateList::size_type ParticleCandidateList::size()     const
{
  return m_list.size();
}

inline ParticleCandidateList::size_type ParticleCandidateList::max_size() const
{
  return m_list.max_size();
}


/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

inline void ParticleCandidateList::push_back( const int& partID )
{
  m_list.push_back( partID );
}

inline void ParticleCandidateList::clear()
{
  m_list.clear();
}

inline void ParticleCandidateList::add( const int& partID )
{
  push_back( partID );
}

inline void ParticleCandidateList::addBQuark()
{
  add( MC::BQUARK ); //> b
}

inline void ParticleCandidateList::addBbarQuark()
{
  add( -MC::BQUARK ); //> b_bar
}

inline void ParticleCandidateList::addBQuarks()
{
  add( MC::BQUARK      ); //> b
  add( -MC::BQUARK ); //> b_bar
}

inline void ParticleCandidateList::addBjet()
{
  add( MC::BQUARK      ); //> b
  add( -MC::BQUARK ); //> b_bar
}

inline void ParticleCandidateList::addWBosons()
{
  add( MC::WPLUSBOSON  ); //> W+
  add( -MC::WPLUSBOSON ); //> W-
 }

inline void ParticleCandidateList::addZBoson()
{
  add( MC::Z0BOSON );
}

#endif //> ANALYSISUTILS_PARTICLECANDIDATELIST_H
