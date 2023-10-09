///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv_p7.h
// Header file for class McEventCollectionCnv_p7
// Author: J.Chapman, P.Clark and A.Verbytskyi
///////////////////////////////////////////////////////////////////
#ifndef GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_p7_H
#define GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_p7_H

// STL includes
#include <unordered_map>

#ifdef HEPMC3
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "HepMC3/Data/GenRunInfoData.h"
#else
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#define protected public
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#undef private
#undef protected
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif
#include "GeneratorObjects/McEventCollection.h"

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollection_p7.h"

#include "GaudiKernel/ServiceHandle.h"

class IHepMCWeightSvc;

// Forward declaration
class MsgStream;
namespace HepMC { struct DataPool; }

class McEventCollectionCnv_p7 : public T_AthenaPoolTPCnvBase<
                                          McEventCollection,
                                          McEventCollection_p7
                                       >
{

  typedef T_AthenaPoolTPCnvBase<McEventCollection,
                                McEventCollection_p7> Base_t;

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:

  /** Default constructor:
   */
  McEventCollectionCnv_p7();

  /** Copy constructor
   */
  McEventCollectionCnv_p7( const McEventCollectionCnv_p7& rhs );

  /** Assignement operator
   */
  McEventCollectionCnv_p7& operator=( const McEventCollectionCnv_p7& rhs );

  /** Destructor
   */
  virtual ~McEventCollectionCnv_p7();

  void setPileup();

  /** Method creating the transient representation of @c McEventCollection
   *  from its persistent representation @c McEventCollection_p7
   */
  virtual void persToTrans( const McEventCollection_p7* persObj,
                            McEventCollection* transObj,
                            MsgStream &log ) ;

  /** Method creating the persistent representation @c McEventCollection_p7
   *  from its transient representation @c McEventCollection
   */
  virtual void transToPers( const McEventCollection* transObj,
                            McEventCollection_p7* persObj,
                            MsgStream &log ) ;

  ///////////////////////////////////////////////////////////////////
  // Protected method:
  ///////////////////////////////////////////////////////////////////
 protected:

  typedef std::unordered_map<HepMC::GenParticlePtr,int> ParticlesMap_t;

  /** @brief Create a transient @c GenVertex from a persistent one (version 1)
   *  It returns the new @c GenVertex.
   *  This method calls @c createGenParticle for each of the out-going
   *  particles and only for the in-going particle which are orphans (no
   *  production vertex): for optimisation purposes.
   *  Note that the map being passed as an argument is to hold the association
   *  of barcodes to particle so that we can reconnect all the (non-orphan)
   *  particles to their decay vertex (if any).
   */
  HepMC::GenVertexPtr
  createGenVertex( const McEventCollection_p7& persEvts,
                   const GenVertex_p7& vtx,
                   ParticlesMap_t& bcToPart,
                   HepMC::DataPool& datapools, HepMC::GenEvent* parent=nullptr ) const;

  /** @brief Create a transient @c GenParticle from a persistent one (vers.1)
   *  It returns the new @c GenParticle. Note that the map being passed as an
   *  argument is to hold the association of barcodes to particle so that
   *  we can reconnect all the particles to their decay vertex (if any).
   */
  HepMC::GenParticlePtr
  createGenParticle( const GenParticle_p7& p,
                     ParticlesMap_t& partToEndVtx,
                     HepMC::DataPool& datapools, const HepMC::GenVertexPtr& parent=nullptr, bool add_to_output = true ) const;

  /** @brief Method to write a persistent @c GenVertex object. The persistent
   *  vertex is added to the persistent is added to the persistent
   *  @c GenEvent.
   */
#ifdef HEPMC3
  static void writeGenVertex( const HepMC::ConstGenVertexPtr& vtx, McEventCollection_p7& persEvt );
#else
  void writeGenVertex( const HepMC::GenVertex& vtx, McEventCollection_p7& persEvt ) const;
#endif
  /** @brief Method to write a persistent @c GenParticle object
   *  It returns the index of the persistent @c GenParticle into the
   *  collection of persistent of @c GenParticles from the
   *  persistent @c GenEvent
   */
#ifdef HEPMC3
  static int writeGenParticle( const HepMC::ConstGenParticlePtr& p, McEventCollection_p7& persEvt );
#else
  int writeGenParticle( const HepMC::GenParticle& p, McEventCollection_p7& persEvt ) const;
#endif

  bool m_isPileup;
  ServiceHandle<IHepMCWeightSvc> m_hepMCWeightSvc;
};
#endif //> GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_p7_H
