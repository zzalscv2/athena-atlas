/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOROBJECTS_HEPMCPARTICLELINK_H
#define GENERATOROBJECTS_HEPMCPARTICLELINK_H
/** @class HepMcParticleLink
 * @brief a link optimized in size for a GenParticle in a McEventCollection
 *
 * @see McEventCollection, GenEvent, ElementLink
 * @author Paolo Calafiura
 **/

#include "SGTools/DataProxy.h"
#include "SGTools/CurrentEventStore.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "CxxUtils/CachedValue.h"
#include "CxxUtils/no_unique_address.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/EventContext.h"
#include <cassert>
#include <cstdint> /*int32_t etc*/
#include <iostream>
#include <limits>  /* numeric_limits */
#include <string>
#include <vector>

#include "AtlasHepMC/GenEvent_fwd.h"
#include "AtlasHepMC/GenParticle.h"

class IProxyDict;
class McEventCollection;

enum EBC_EVCOLL : unsigned char {
  EBC_MAINEVCOLL=0,   //Usually contains the HS GenEvent
  EBC_FIRSTPUEVCOLL,  //Low pt minbias PU, or full PU collection where applicable (i.e. Fast Chain)
  EBC_SECONDPUEVCOLL, //High pt minbias PU
  EBC_THIRDPUEVCOLL,  //Reserved for cavern background
  EBC_NCOLLKINDS      //Just contains the maximum of the enum (for loops)
};


/**
 * @brief a link optimized in size for a GenParticle in a McEventCollection
 * 
 * A link is defined by three items:
 *  - A target McEventCollection;
 *  - A particular GenEvent within this collection; and
 *  - A particular particle within the GenEvent.
 *
 * The target McEventCollection is identified by an enum of type EBC_EVCOLL.
 * These enums may also be represented as the characters 'a'..'d'.
 * Each of these enums has an associated list of StoreGate keys (see
 * the initialization of @c s_keys).  When we want to dereference the link,
 * we try to look up each possible key in turn until we find one that works.
 * (The position of that key in the list is then remembered as a hint
 * for where to start searching next time.)
 *
 * The particular GenEvent within the collection can be represented as either
 * an event number (as returned by GenEvent::event_number()) or by a position
 * within the collection.  Which case is desired is indicated by the value
 * of the isIndexEventPosition argument to the constructors (defaulting
 * to the event number case).  In order to convert between these, we need
 * to retrieve the McEventCollection from the store, but we don't have
 * the store pointer once the link has been dereferenced.  Therefore,
 * when a position-based link is dereferenced, it is automatically changed
 * to be event-number-based.  This also happens when a link is constructed
 * with an explicit ConstGenParticlePtr and a position-based GenEvent.
 *
 * As a special case, a GenEvent number of 0 is interpreted as the first
 * GenEvent in the collection.
 *
 * The particle within the GenEvent is represented by a barcode.
 * A barcode of 0 implies a null link.
 *
 * In order to dereference a link, we need to reference a particular
 * event store.  By default, the current store (as defined by
 * SG::CurrentEventStore::store()) is stored when the link is constructed;
 * however, an explicit IProxyDict* or EventContext may also be specified.
 */
class HepMcParticleLink
{
public:
  typedef uint32_t barcode_type;
  typedef uint32_t index_type;


  enum PositionFlag {
    IS_EVENTNUM = 0,
    IS_POSITION = 1,
  };


  /**
   * @brief Persistent representation of a link.
   *        See @c HepMcParticleLink for details.
   */
  class ExtendedBarCode {
  public:
    /// All 1's.  Used to represent an undefined index/position.
    constexpr static index_type UNDEFINED = ~static_cast<index_type>(0);


    /** 
     * @brief Default constructor.  Makes a null link (barcode 0).
     */
    ExtendedBarCode();


    /** 
     * @brief Constructor.
     * @param barcode Barcode of target particle.
     * @param eventIndex Identifies the target GenEvent in a McEventCollection,
     *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
     *        or the position in the container
     *        if isIndexEventPosition is IS_POSITION.
     *        0 always means the first event in the collection.
     * @param evtColl Identifies which McEventCollection to use.
     * @param isIndexEventPosition: See @c eventIndex.
     */
    ExtendedBarCode (barcode_type barcode,
                     index_type eventIndex,
                     EBC_EVCOLL evtColl=EBC_MAINEVCOLL,
                     PositionFlag isIndexEventPosition = IS_EVENTNUM);

                    
    /** 
     * @brief Constructor.
     * @param barcode Barcode of target particle.
     * @param eventIndex Identifies the target GenEvent in a McEventCollection,
     *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
     *        or the position in the container
     *        if isIndexEventPosition is IS_POSITION.
     *        0 always means the first event in the collection.
     * @param evtColl Identifies which McEventCollection to use,
     *        as a character ('a'..'d').
     * @param isIndexEventPosition: See @c eventIndex.
     */
    ExtendedBarCode (barcode_type barcode,
                     index_type eventIndex,
                     char evtColl,
                     PositionFlag isIndexEventPosition = IS_EVENTNUM);


    /**
     * @brief Copy constructor.  (Can't be defaulted due to the atomic member.)
     */
    ExtendedBarCode(const ExtendedBarCode& rhs);


    /**
     * @brief Move constructor.  (Can't be defaulted due to the atomic member.)
     */
    ExtendedBarCode(ExtendedBarCode&& rhs) noexcept;


    /**
     * @brief Assignment.  (Can't be defaulted due to the atomic member.)
     */
    ExtendedBarCode& operator= (const ExtendedBarCode& rhs);

    /**
     * @brief Move Assignment.  (Can't be defaulted due to the atomic member.)
     */
    ExtendedBarCode& operator= (ExtendedBarCode&& rhs) noexcept;

    /**
     * @brief Barcode of target variable (0 for a null link).
     */
    barcode_type barcode() const;


    /**
     * @brief Return the event index/position.
     * @param index[out] Event index (number), or @c UNDEFINED.
     * @param position[out] Event position, or @c UNDEFINED.
     *
     * The GenEvent within the McEventCollection is identified either by
     * the GenEvent number or by the position within the collection.
     * This method will return this by setting either @c index or @c position;
     * the other one is set to @c UNDEFINED.
     */
    void eventIndex (index_type& index, index_type& position) const;


    /**
     * @brief Return which collection we're targeting, as an enum.
     */
    EBC_EVCOLL getEventCollection() const;


    /**
     * @brief Return which collection we're targeting, as a char ('a'..'d').
     */
    char getEventCollectionAsChar() const;


    /**
     * @brief Translate event collection enum to a char ('a'..'d').
     */
    static char eventCollectionAsChar (EBC_EVCOLL evtColl);


    /**
     * @brief Translate event char ('a'..'d') to an enum.
     */
    static EBC_EVCOLL eventCollectionFromChar (char evtColl);


    /**
     * @brief Equality test.
     *
     * Be aware: if one EBC holds the target GenEvent by number and the
     * other by position, then this will always return false, even if they
     * reference the same GenEvent.
     * To avoid this, use HepMcParticleLink::operator=.
     */
    bool operator == (const ExtendedBarCode& rhs ) const;


    /**
     * @brief Inequality test.
     *
     * Be aware: if one EBC holds the target GenEvent by number and the
     * other by position, then this will always return true, even if they
     * reference the same GenEvent.
     * To avoid this, use HepMcParticleLink::operator=.
     */
    bool operator != (const ExtendedBarCode& rhs ) const;


    /**
     * @brief Ordering test.
     *
     * Be aware: if one EBC holds the target GenEvent by number and the
     * other by position, then this will not work as expected.
     * To avoid this, use HepMcParticleLink::operator=.
     */
    bool operator < (const ExtendedBarCode& rhs ) const;


    /**
     * @brief Compare the event index part of two links.
     * @param lhs First link to compare.
     * @param rhs Second link to compare.
     * @returns -1, 0, or 1, depending on the result of the comparison.
     *
     * The event index part of the link can be represented as either
     * an event number or the position within the container.
     * If necessary, the links will be normalized so that they
     * both refer to an event number.
     */
    static
    int compareIndex (const HepMcParticleLink& lhs,
                      const HepMcParticleLink& rhs);


    /**
     * @brief Dump in textual format to a stream.
     */
    void print(std::ostream& os) const;


    /**
     * @brief Dump in textual format to a MsgStream.
     */
    void print(MsgStream& os) const;


    /**
     * @brief Change index from position to number.
     * @param index Event number to set.
     * @param position Existing event position.
     *
     * If the link is currently referencing the GenEvent at @c position,
     * update it so that it instead references the GenEvent
     * with number @c index.
     *
     * This may be called concurrently, as long as all such concurrent
     * calls have the same arguments.
     */
    void makeIndex (index_type index, index_type position) const;


  private:
    /// Flag marking that an index refers to an event position.
    constexpr static index_type POSITION_MASK = UNDEFINED ^ (UNDEFINED>>1);


    /**
     * @brief Initialize the event index part of the link.
     * @param idx The index or position.
     * @param positionFlag If IS_POSITION, @c idx represents a position
     *        in the collection; otherwise, it represents an event number.
     */
    void setIndex (index_type idx, PositionFlag isPosition);


    /// Barcode of the target particle.  0 means a null link.
    barcode_type m_BC;

    /// Identifies the target GenEvent within the event collection.
    /// If the high bit is set, then this (with the high bit clear)
    /// is a position within the collection; otherwise, it is the
    /// target GenEvent number.
    mutable std::atomic<index_type> m_evtIndex;

    /// Identifies which event collection is targeted.
    EBC_EVCOLL m_evtColl;
  };


  /**
   * @brief Default constructor.  Makes a null link.
   * @param sg Optional specification of a specific store to reference.
   */
  HepMcParticleLink (IProxyDict* sg = nullptr);


  /**
   * @brief Default constructor.  Makes a null link.
   * @param ctx Context of the store to reference.
   */
  HepMcParticleLink (const EventContext& ctx);


  /**
   * @brief Constructor.
   * @param barCode Barcode of the target particle.  0 means a null link.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evColl The targeted event collection, as an enum.
   * @param positionFlag: See @c eventIndex.
   * @param sg Optional specification of a specific store to reference.
   */
  HepMcParticleLink (barcode_type barCode,
                     uint32_t eventIndex = 0,
                     EBC_EVCOLL evColl = EBC_MAINEVCOLL,
                     PositionFlag positionFlag = IS_EVENTNUM,
                     IProxyDict* sg = SG::CurrentEventStore::store());


  /**
   * @brief Constructor.
   * @param barCode Barcode of the target particle.  0 means a null link.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evColl The targeted event collection, as an enum.
   * @param positionFlag: See @c eventIndex.
   * @param ctx Context of the store to reference.
   */
  HepMcParticleLink (barcode_type barCode,
                     uint32_t eventIndex,
                     EBC_EVCOLL evColl,
                     PositionFlag positionFlag,
                     const EventContext& ctx);


 /**
   * @brief Constructor.
   * @param barCode Barcode of the target particle.  0 means a null link.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evCollName The name of the targeted event collection.
   * @param positionFlag: See @c eventIndex.
   * @param sg Optional specification of a specific store to reference.
   */
  HepMcParticleLink (barcode_type barCode,
                     uint32_t eventIndex,
                     const std::string& evCollName,
                     PositionFlag positionFlag = IS_EVENTNUM,
                     IProxyDict* sg = SG::CurrentEventStore::store());


 /**
   * @brief Constructor.
   * @param p Particle to reference.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evColl The targeted event collection, as an enum.
   * @param positionFlag: See @c eventIndex.
   * @param sg Optional specification of a specific store to reference.
   */
  HepMcParticleLink (const HepMC::ConstGenParticlePtr& p,
                     uint32_t eventIndex = 0,
                     EBC_EVCOLL evColl=EBC_MAINEVCOLL,
                     PositionFlag positionFlag = IS_EVENTNUM,
                     IProxyDict* sg = SG::CurrentEventStore::store());


 /**
   * @brief Constructor.
   * @param p Particle to reference.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evCollName The name of the targeted event collection.
   * @param positionFlag: See @c eventIndex.
   * @param sg Optional specification of a specific store to reference.
   */
  HepMcParticleLink (const HepMC::ConstGenParticlePtr& part,
                     uint32_t eventIndex,
                     const std::string& evCollName,
                     PositionFlag positionFlag = IS_EVENTNUM,
                     IProxyDict* sg = SG::CurrentEventStore::store());


 /**
   * @brief Constructor.
   * @param p Particle to reference.
   * @param eventIndex Identifies the target GenEvent in a McEventCollection,
   *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
   *        or the position in the container
   *        if isIndexEventPosition is IS_POSITION.
   *        0 always means the first event in the collection.
   * @param evColl The targeted event collection, as an enum.
   * @param positionFlag: See @c eventIndex.
   * @param ctx Context of the store to reference.
   */
  HepMcParticleLink (const HepMC::ConstGenParticlePtr& part,
                     uint32_t eventIndex,
                     EBC_EVCOLL evColl,
                     PositionFlag positionFlag,
                     const EventContext& ctx);


  /**
   * @brief Copy constructor.
   */
  HepMcParticleLink(const HepMcParticleLink& rhs) = default;


  /**
   * @brief Move constructor.
   */
  HepMcParticleLink(HepMcParticleLink&& rhs) noexcept = default;

  /**
   * @brief Assignment.
   */
  HepMcParticleLink& operator=(const HepMcParticleLink& rhs) = default;

  /**
   * @brief Move Assignment.
   */
  HepMcParticleLink& operator=(HepMcParticleLink&& rhs) = default;


  /// \name pointer interface
  //@{


  /**
   * @brief Dereference.
   */
  const HepMC::GenParticle& operator* () const;


  /**
   * @brief Dereference.
   */
  HepMC::ConstGenParticlePtr operator->() const;


  /**
   * @brief Dereference.
   */
  operator HepMC::ConstGenParticlePtr() const;


  /**
   * @brief Dereference.
   */
  HepMC::ConstGenParticlePtr cptr() const;


  /**
   * @brief Dereference/smart pointer.
   */
  HepMC::ConstGenParticlePtr scptr() const { return cptr(); }


  /** 
   * @brief Validity check.  Dereference and check for null.
   */
  bool isValid() const;


  /** 
   * @brief Validity check.  Dereference and check for null.
   */
  bool operator!() const;

  /** 
   * @brief Validity check.  Dereference and check for null.
   */
  operator bool() const;

  //@}

  /// \name Comparison operators.
  //@{


  /**
   * @brief Equality comparison.
   */
  bool operator == (const HepMcParticleLink& rhs) const;


  /**
   * @brief Inequality comparison.
   */
  bool operator != (const HepMcParticleLink& rhs) const;


  /**
   * @brief Ordering comparison.
   */
  bool operator < (const HepMcParticleLink& rhs) const;


  //@}

  /// \name ParticleLink-specific methods.
  //@{


  /**
   * @brief Return the barcode of the target particle.  0 for a null link.
   * FIXME: return type.
   */
  int barcode() const;


  /**
   * @brief Return the event number of the referenced GenEvent.
   *        0 means the first GenEvent in the collection.
   */
  index_type eventIndex() const;


  /**
   * @brief Return the event number of the GenEvent at the specified
   *        position in the McEventCollection.
   * @param position in the McEventCollection
   * @param evColl McEventCollection type
   * @param sg Target event store.
   * Returns -999 when position is larger than the McEventCollection size
   */
  static int getEventNumberAtPosition (index_type position, EBC_EVCOLL evColl, const IProxyDict* sg);


  /**
   * @brief Return a vector of the positions in the McEventCollection of the
   *        GenEvent(s) with a given event number.
   * @param index the event number of the required GenEvent
   * @param evColl McEventCollection type
   * @param sg Target event store.
   * Returns a vector containing only ExtendedBarCode::UNDEFINED FIXME when no event with the appropriate event_number was found.
   * (Multiple entries in the vector is technically a bug, but can't be fixed until the HepMC3 migration.)
   */
  static std::vector<index_type> getEventPositionInCollection (index_type index, EBC_EVCOLL evColl, const IProxyDict* sg);

  /**
   * @brief Return the position in the McEventCollection of the
   *        GenEvent pointed to by this HepMcParticleLink
   * @param sg Target event store.
   * FIXME - need to be able to flag when no event with the appropriate event_number was found.
   */
  index_type getEventPositionInCollection (const IProxyDict* sg) const;


  /**
   * @brief Return which collection we're targeting, as an enum.
   */
  EBC_EVCOLL getEventCollection() const;


  /**
   * @brief Return which collection we're targeting, as a char ('a'..'d').
   */
  char getEventCollectionAsChar() const;


  /**
   * @brief Hash the 32 bit barcode and 16 bit eventindex into a 32bit int.
   */
  barcode_type compress() const;


  /** 
   * @brief Return the corresponding enum from a McEventCollection name.
   */
  static EBC_EVCOLL find_enumFromKey (const std::string& evCollName);


  /** 
   * @brief Alter the persistent part of the link.
   */
  void setExtendedBarCode (const ExtendedBarCode& extBarcode);


  //@}


 private:
  friend std::ostream& operator << (std::ostream&, const HepMcParticleLink&);
  friend MsgStream& operator << (MsgStream&, const HepMcParticleLink&);


  /**
   * @brief Look up the event collection we're targeting.
   * @param sg Target event store.
   * @param evColl McEventCollection type
   * May return nullptr if the collection is not found.
   */
  static const McEventCollection*
  retrieveMcEventCollection (EBC_EVCOLL evColl, const IProxyDict* sg);


  /**
   * @brief Look up the event collection we're targeting.
   * @param sg Target event store.
   * May return nullptr if the collection is not found.
   */
  const McEventCollection*
  retrieveMcEventCollection (const IProxyDict* sg) const;


  /**
   * @brief Find the proxy for the target event collection.
   * @param evColl McEventCollection type
   * @param sg Target event store.
   * May return nullptr if the collection is not found.
   */
  static SG::DataProxy* find_proxy (EBC_EVCOLL evColl, const IProxyDict* sg);


  /**
   * @brief Return the most recent SG key used for a particular collection type.
   * @param evColl The targeted event collection, as an enum.
   */
  static std::string getLastEventCollectionName (EBC_EVCOLL evColl);


  /// Pointer to the store containing the event.
  IProxyDict* m_store;


  /// Transient part.  Pointer to the particle.
  // This qualifier allows m_extBarcode to overlap the trailing padding
  // of @c CachedValue.  On x86_64, this reduces the size
  // of @c HepMcParticleLink from 48 to 40.
  ATH_NO_UNIQUE_ADDRESS
  CxxUtils::CachedValue<HepMC::ConstGenParticlePtr> m_ptr;


  /// Persistent part: barcode and location of target GenEvent.
  ExtendedBarCode m_extBarcode;
};


/**
 * @brief Comparison with ConstGenParticlePtr.
 * Needed with c++20 to break an ambiguity.
 */
bool operator== (HepMC::ConstGenParticlePtr a,
                 const HepMcParticleLink& b);


/**
 * @brief Output operator.
 * @param os Stream to which to output.
 * @param link Link to dump.
 */
std::ostream& operator<< (std::ostream& os, const HepMcParticleLink& link);


/**
 * @brief Output operator.
 * @param os MsgStream to which to output.
 * @param link Link to dump.
 */
MsgStream& operator<< (MsgStream& os, const HepMcParticleLink& link);


#include "GeneratorObjects/HepMcParticleLink.icc"


#endif
