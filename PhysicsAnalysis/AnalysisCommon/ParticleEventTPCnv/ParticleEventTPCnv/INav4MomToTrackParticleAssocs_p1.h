///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// INav4MomToTrackParticleAssocs_p1.h 
// Header file for class INav4MomToTrackParticleAssocs_p1
// Author: Karsten Koeneke
/////////////////////////////////////////////////////////////////// 
#ifndef PARTICLEEVENTTPCNV_INAV4MOMTOTRACKPARTICLEASSOCS_p1_H 
#define PARTICLEEVENTTPCNV_INAV4MOMTOTRACKPARTICLEASSOCS_p1_H

// STL includes
#include <string>
#include <vector>
#include <utility>

// DataModelAthenaPool
#include "DataModelAthenaPool/DataLink_p2.h"
#include "DataModelAthenaPool/ElementLink_p3.h"

// Forward declaration
class INav4MomToTrackParticleAssocsCnv_p1;

class INav4MomToTrackParticleAssocs_p1
{ 
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class INav4MomToTrackParticleAssocsCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public typedefs: 
  /////////////////////////////////////////////////////////////////// 
public:
  // some typedefs as a workaround for templated classes with a long name
  typedef std::vector<ElementLinkInt_p3>                ElemLinkVect_t;
  typedef std::pair<ElementLinkInt_p3, ElemLinkVect_t>  AssocElem_t;
  typedef std::vector<AssocElem_t>                      Assocs_t;
  typedef DataLinkVector_p2                             TrackParticleStores_t;

   /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  INav4MomToTrackParticleAssocs_p1();

  /** Destructor: 
   */
  ~INav4MomToTrackParticleAssocs_p1() = default;

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
protected: 
  Assocs_t                m_assocs;
  TrackParticleStores_t   m_assocStores;

}; 

/////////////////////////////////////////////////////////////////// 
/// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline 
INav4MomToTrackParticleAssocs_p1::INav4MomToTrackParticleAssocs_p1() :
  m_assocs(),
  m_assocStores()
{}

#endif //> PARTICLEEVENTTPCNV_INAV4MOMTOTRACKPARTICLEASSOCS_p1_H
