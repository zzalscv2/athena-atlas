/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/CondCont.h
 * @author Vakho, Charles, Scott
 * @date 2017
 * @brief Hold mappings of ranges to condition objects.
 *
 * Conditions objects of type @c T are managed by @c CondCont<T>, which are
 * recorded in the conditions store.  @c CondCont<T> is a map of IOV ranges
 * to payload objects of type @c T.
 *
 * A conditions container must be declared as such and given a CLID using
 * the macro @c CONDCONT_DEF:
 *@code
 *  CONDCONT_DEF (MyType, 12345);
 @endcode
 *
 * If one payload class derives from another, it is possible declare
 * conditions containers so that they have the same inheritance by adding
 * the payload base class as a thir argument to @c CONDCONT_DEF.  For example,
 * if @c MyType derived from @c MyBase then you can use
 *@code
 *  CONDCONT_DEF (MyType, 12345, MyBase);
 @endcode
 *
 * A conditions container declared in this way should have keys that
 * are either ranges in run+lbn or timestamp; they cannot be mixed.
 * However, in the case where one has a conditions algorithm that takes
 * as input both a run+lbn and a timestamp condition, the result
 * will be IOVs that have both run+lbn and timestamp ranges.  Such conditions
 * may be stored in a mixed conditions container, declared as:
 @code
 *  CONDCONT_MIXED_DEF (MyType, 12345);
 @endcode
 *
 * @c CondCont<MyType> is now declared as a mixed container.
 * Mixed containers may also participate in inheritance, but the base class
 * declared for a mixed container must also have been declared as mixed.
 *
 * Implementation notes:
 * All conditions containers derive from @c CondContBase.  This defines
 * virtual functions that are used by the IOV services.  @c CondContSingleBase
 * derives from this and collects non-templated implementations used by
 * non-mixed containers; most virtual functions can be declared @c final here.
 * Finally, @c CondCont<T> derives from @c CondContSingleBase and implements
 * the type-specific part.
 *
 * We reduce the amount of templated code that must be instantiated by storing
 * the payloads as void* in the underlying @c ConcurrentRangeMap.  All this
 * needs to do with the payloads is to be able to delete them; we handle
 * this by giving @c ConcurrentRangeMap a function to call to delete a payload.
 *
 * For the case of mixed containers, rather than having the @c ConcurrentRangeMap
 * store the payload directly, it instead holds another @c ConcurrentRangeMap
 * which in turn hold the payload.  The top map is indexed by run+lbn
 * and the secondary ones by timestamp.
 *
 * For mixed containers, we have @c CondContMixedBase deriving from
 * @c CondContBase and @c CondContMixed<T> deriving from that.
 * The @c CONDCONT_MIXED_DEF macro then specializes @c CondCont<T>
 * so that it derives from @c CondContMixed<T>
 */

#ifndef ATHENAKERNEL_CONDCONT_H
#define ATHENAKERNEL_CONDCONT_H

#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/BaseInfo.h"
#include "AthenaKernel/RCUUpdater.h"
#include "AthenaKernel/IConditionsCleanerSvc.h"
#include "AthenaKernel/CondObjDeleter.h"
#include "CxxUtils/ConcurrentRangeMap.h"
#include "CxxUtils/ConcurrentPtrSet.h"
#include "CxxUtils/SimpleUpdater.h"
#include "CxxUtils/checker_macros.h"

#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventIDRange.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ServiceHandle.h"
#include "boost/preprocessor/facilities/overload.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <typeinfo>
#include <mutex>
#include <type_traits>


namespace SG {
  class DataProxy;
}
namespace Athena {
  class IRCUSvc;
}


/**
 * @brief Define extended status codes used by CondCont.
 *        We add DUPLICATE, OVERLAP, and EXTENDED.
 */
enum class CondContStatusCode : StatusCode::code_t
{
  FAILURE           = 0,
  SUCCESS           = 1,
  RECOVERABLE       = 2,

  // Attempt to insert an item in a CondCont with a range duplicating
  // an existing one.  The original contents of the container are unchanged,
  // and the new item has been deleted.
  // This is classified as Success.
  DUPLICATE         = 10,

  // Attempt to insert an item in a CondCont with a range that partially
  // overlaps with an existing one.
  // This is classified as Success.
  OVERLAP           = 11,

  // Attempt to insert an item in a CondCont where the new range is an extension
  // of the last range.  That is, the start time of the new range matches
  // that of the last range in the container, and the end time of the new range
  // is larger than that of the last range in the container.  The end time
  // of the last range has been extended to match the new range.
  // The payload of the existing range is unchanged, and the new
  // item has been deleted.
  EXTENDED          = 12
};
STATUSCODE_ENUM_DECL (CondContStatusCode)


/**
 * @brief Base class for all conditions containers.
 */
class CondContBase
{
public:
  /**
   * @brief Status code category for ContCont.
   *        This adds new codes DUPLICATE, OVERLAP, and EXTENDED,
   *        which are classified as success.
   */
  class Category : public StatusCode::Category
  {
  public:
    typedef StatusCode::code_t code_t;

    /// Name of the category
    virtual const char* name() const override;

    /// Description for code within this category.
    virtual std::string message (code_t code) const override;

    /// Is code considered success?
    virtual bool isSuccess (code_t code) const override;

    /// Helper to test whether a code is DUPLICATE.
    static bool isDuplicate (code_t code);
    /// Helper to test whether a code is DUPLICATE.
    static bool isDuplicate (StatusCode code);

    /// Helper to test whether a code is OVERLAP.
    static bool isOverlap (code_t code);
    /// Helper to test whether a code is OVERLAP.
    static bool isOverlap (StatusCode code);

    /// Helper to test whether a code is EXTENDED.
    static bool isExtended (code_t code);
    /// Helper to test whether a code is EXTENDED.
    static bool isExtended (StatusCode code);
  };


  /// Type of key used for this container.
  enum class KeyType
  {
    /// Either TIMESTAMP or RUNLBN, but nothing's been put in the container yet,
    /// so we don't know which one.
    SINGLE,

    /// Container uses timestamp keys.
    TIMESTAMP,

    /// Container uses run+lbn keys.
    RUNLBN,

    /// Mixed Run+lbn / timestamp container.
    MIXED,
  };


  /// Payload type held by this class.
  /// Need to define this here for @c cast() to work properly.
  typedef void Payload;

  
  /// Type used to store an IOV time internally.
  /// For efficiency, we pack two 32-bit words into a 64-bit word
  /// Can be either run+lbn or a timestamp.
  typedef uint64_t key_type;


  /// Destructor.
  virtual ~CondContBase() {};


  /**
   * @brief Return the CLID of the most-derived @c CondCont.
   */
  CLID clid() const;


  /**
   * @brief Return the key type for this container.
   */
  KeyType keyType() const;


  /**
   * @brief Return CLID/key corresponding to this container.
   */
  const DataObjID& id() const;


  /**
   * @brief Return the associated @c DataProxy, if any.
   */
  SG::DataProxy* proxy();


  /**
   * @brief Return the associated @c DataProxy, if any.
   */
  const SG::DataProxy* proxy() const;


  /**
   * @brief Set the associated @c DataProxy.
   * @param proxy The proxy to set.
   */
  void setProxy(SG::DataProxy*);


  /**
   * @brief Dump the container contents for debugging.
   * @param ost Stream to which to write the dump.
   */
  virtual
  void list (std::ostream& ost) const = 0;


  /**
   * @brief Dump the container to cout.  For calling from the debugger.
   */
  void print() const;

  
  /**
   * @brief Return the number of conditions objects in the container.
   */
  virtual size_t entries() const;

  
  /**
   * @brief Return all IOV validity ranges defined in this container.
   */
  virtual
  std::vector<EventIDRange> ranges() const = 0;


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param obj Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * @c obj must point to an object of type @c T,
   * except in the case of inheritance, where the type of @c obj must
   * correspond to the most-derived @c CondCont type.
   * The container will take ownership of this object.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * EXTENDED if the last existing range in the container was extended
   * to match the new range;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  virtual
  StatusCode typelessInsert (const EventIDRange& r,
                             void* obj,
                             const EventContext& ctx = Gaudi::Hive::currentContext()) = 0;


  /**
   * @brief Test to see if a given IOV time is mapped in the container.
   * @param t IOV time to check.
   */
  virtual
  bool valid( const EventIDBase& t) const = 0;


  /**
   * @brief Return the mapped validity range for an IOV time.
   * @param t IOV time to check.
   * @param r[out] The range containing @c t.
   *
   * Returns true if @c t is mapped; false otherwise.
   */
  virtual
  bool range (const EventIDBase& t, EventIDRange& r) const = 0;


  /**
   * @brief Erase the first element not less than @c t.
   * @param IOV time of element to erase.
   * @param ctx Event context for the current thread.
   */
  virtual
  StatusCode erase (const EventIDBase& t,
                    const EventContext& ctx = Gaudi::Hive::currentContext()) = 0;


  /**
   * @brief Remove unused entries from the front of the list.
   * @param runLbnKeys List of Run-Lumi keys that may still be in use.
   *             (Must be sorted.)
   * @param TSKeys List of time-stamp keys that may still be in use.
   *             (Must be sorted.)
   *
   * We examine the objects in the container, starting with the earliest one.
   * If none of the keys in @c keys match the range for this object, then
   * it is removed from the container.  We stop when we either find
   * an object with a range matching a key in @c keys or when there
   * is only one object left. Mixed containers are trimmed recursivly. 
   *
   * The list @c runLbnKeys should contain keys as computed by keyFromRunLBN,
   * the @c list TSKey should contain keys from keyFromTimestamp. Depending 
   * on the type (RUNLUMI, TIMESTAMP, MIXED) only one of the lists or both 
   * are used. The list must be sorted.
   *
   * Removed objects are queued for deletion once all slots have been
   * marked as quiescent.
   *
   * Returns the number of objects that were removed.
   */
  virtual
  size_t trim (const std::vector<key_type>& runLbnKeys, const std::vector<key_type>& TSKeys);


  /**
   * @brief Remove all entries in the container.
   *        Mostly for testing --- should not normally be used.
   */
  void clear();


  /**
   * @brief Mark that this thread is no longer accessing data from this container.
   * @param ctx Event context for the current thread.
   *
   * This would normally be done through RCU service.
   * Defined here for purposes of testing.
   */
  void quiescent (const EventContext& ctx = Gaudi::Hive::currentContext());


  /**
   * @brief Return the number times an item was inserted into the map.
   */
  size_t nInserts() const;


  /**
   * @brief Return the maximum size of the map.
   */
  size_t maxSize() const;


  /**
   * @brief Extend the range of the last IOV.
   * @param newRange New validity range.
   * @param ctx Event context.
   *
   * Returns failure if the start time of @c newRange does not match the start time
   * of the last IOV in the container.  Otherwise, the end time for the last
   * IOV is changed to the end time for @c newRange.  (If the end time for @c newRange
   * is before the end of the last IOV, then nothing is changed.)
   */
  virtual
  StatusCode extendLastRange (const EventIDRange& newRange,
                              const EventContext& ctx = Gaudi::Hive::currentContext()) = 0;


  /**
   * @brief Make a run+lbn key from an EventIDBase.
   * @param Event ID to convert.
   */
  static
  key_type keyFromRunLBN (const EventIDBase& b);


  /**
   * @brief Make a timestamp key from an EventIDBase.
   * @param Event ID to convert.
   */
  static
  key_type keyFromTimestamp (const EventIDBase& b);


  /**
   * @brief Range object to store in @c ConcurrentRangeMap.
   *
   * We need to store the original range as an @c EventIDRange.
   * For efficiency of comparisons, we also store the start and stop
   * times as packed key_types.
   */
  struct RangeKey
  {
    /// Default constructor.
    RangeKey();

    /// Constructor from range+start/stop.
    RangeKey (const EventIDRange& r,
              key_type start,
              key_type stop);


    /// Packed start time.
    key_type m_start;

    /// Packed stop time.
    key_type m_stop;

    /// Original range object.
    EventIDRange m_range;
  };


  /**
   * @brief Comparison object needed by ConcurrentRangeMap.
   */
  struct Compare
  {
    bool operator() (const RangeKey& r1, const RangeKey& r2) const
    { return r1.m_start < r2.m_start; }
    bool operator() (key_type t, const RangeKey& r2) const
    { return t < r2.m_start; }
    bool inRange (key_type t, const RangeKey& r) const
    {
      return t >= r.m_start && t< r.m_stop;
    }


    /**
     * @brief Test if two ranges overlap, and adjust if needed.
     * @param ctx Event context passed to emplace().
     * @param oldRange An existing range in the container.
     * @param newRange New range being added.
     *
     * Returns one of:
     *   0 -- no overlap between the ranges; NEWRANGE is unmodified.
     *   1 -- ranges overlap.  NEWRANGE has been adjusted to avoid the overlap.
     *        If the start of NEWRANGE is changed, it must
     *        only be moved forward (increased), never backwards.
     *  -1 -- duplicate: NEWRANGE is entirely inside OLDRANGE.
     *        Delete the new range.         
     */
    int overlap (const EventContext& ctx,
                 const RangeKey& oldRange, RangeKey& newRange) const;


    /**
     * @brief Possibly extend an existing range at the end.
     * @param range THe existing range.
     * @param newRange Range being added.
     *
     * Returns one of:
     *   0 -- no change was made to RANGE.
     *   1 -- RANGE was extended. 
     *  -1 -- newRange is a duplicate.
     */
    int extendRange (RangeKey& range, const RangeKey& newRange) const;
  };


  /**
   * @brief Declare another conditions container that depends on this one.
   * @param dep Conditions container that depends on this one.
   */
  void addDep (CondContBase* dep);


  /**
   * @brief Return the list of conditions containers that depend on this one.
   */
  std::vector<CondContBase*> getDeps();


  /**
   * @brief Allow overriding the name of the global conditions cleaner
   *        service (for testing purposes).
   * @param name The name of the global conditions cleaner service.
   */
  static void setCleanerSvcName ATLAS_NOT_THREAD_SAFE (const std::string& name);

  
protected:
  typedef CxxUtils::ConcurrentRangeMap<RangeKey, key_type, void, Compare,
    Athena::RCUUpdater>
    CondContSet;

  typedef CondContSet::Updater_t Updater_t;
  typedef CondContSet::delete_function delete_function;

  /**
   * @brief Internal constructor.
   * @param rcusvc RCU service instance.
   * @param keyType Key type for this container.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param payloadDeleter Object for deleting payload objects.
   * @param capacity Initial capacity of the container.
   */
  CondContBase (Athena::IRCUSvc& rcusvc,
                KeyType keytype,
                CLID clid,
                const DataObjID& id,
                SG::DataProxy* proxy,
                std::shared_ptr<CondContSet::IPayloadDeleter> payloadDeleter,
                size_t capacity);
                

  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param t Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * EXTENDED if the last existing range in the container was extended
   * to match the new range;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  StatusCode insertBase (const EventIDRange& r,
                         CondContSet::payload_unique_ptr t,
                         const EventContext& ctx = Gaudi::Hive::currentContext());


  /**
   * @brief Erase the first element not less than @c t.
   * @param IOV time of element to erase.
   * @param ctx Event context for the current thread.
   */
  StatusCode eraseBase (const EventIDBase& t,
                        const EventContext& ctx = Gaudi::Hive::currentContext());


  /**
   * @brief Extend the range of the last IOV.
   * @param newRange New validity range.
   * @param ctx Event context.
   *
   * Returns failure if the start time of @c newRange does not match the start time
   * of the last IOV in the container.  Otherwise, the end time for the last
   * IOV is changed to the end time for @c newRange.  (If the end time for @c newRange
   * is before the end of the last IOV, then nothing is changed.)
   */
  StatusCode
  extendLastRangeBase (const EventIDRange& newRange,
                       const EventContext& ctx = Gaudi::Hive::currentContext());


  /** 
   * @brief Internal lookup function.
   * @param t IOV time to find.
   * @param r If non-null, copy validity range of the object here.
   *
   * Looks up the conditions object corresponding to the IOV time @c t.
   * If found, return the pointer (as a pointer to the payload type
   * of the most-derived CondCont).  Otherwise, return nullptr.
   */
  const void* findBase (const EventIDBase& t,
                        EventIDRange const** r) const;


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   */
  const void* cast (CLID clid, const void* ptr) const;


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   *
   * This is a virtual function that calls @c cast from the most-derived class
   * of the hierarchy.
   */
  virtual const void* doCast (CLID clid, const void* ptr) const = 0;


  /**
   * @brief Call @c func on each entry in the container.
   * @param func Functional to call on each entry.
   *
   * @c func will be called on each container element
   * (being passed const CondContSet::value_type&).
   */
  template <class FUNC>
  void forEach (const FUNC& func) const;


  /**
   * @brief Tell the cleaner that a new object was added to the container.
   */
  StatusCode inserted (const EventContext& ctx);


  /**
   * @brief Helper to report an error due to using a base class for insertion.
   * @param usedCLID CLID of the class used for insertion.
   */
  void insertError (CLID usedCLID) const;


  /**
   * @brief Return the deletion function for this container.
   */
  delete_function* delfcn() const;


  /**
   * @brief Description of this container to use for MsgStream.
   */
  std::string title() const;


private:
  /// Key type of this container.
  std::atomic<KeyType> m_keyType;

  /// CLID of the most-derived @c CondCont
  CLID m_clid;

  /// CLID+key for this container.
  DataObjID m_id;

  /// Associated @c DataProxy.
  SG::DataProxy* m_proxy;

  /// Set of mapped objects.
  CondContSet m_condSet;

  /// Handle to the cleaner service.
  ServiceHandle<Athena::IConditionsCleanerSvc> m_cleanerSvc;

  /// Other conditions dependencies that depend on this one, as inferred
  /// by addDependency calls.  There should only be a few of them.
  using DepSet = CxxUtils::ConcurrentPtrSet<CondContBase, CxxUtils::SimpleUpdater>;
  DepSet m_deps;

  /// Name of the global conditions cleaner service.
  static std::string s_cleanerSvcName ATLAS_THREAD_SAFE;
};


CLASS_DEF( CondContBase , 34480459 , 1 )


///////////////////////////////////////////////////////////////////////////


/**
 * @brief Base class for conditions containers that are either
 *        Run+LBN or timestamp.
 */
class CondContSingleBase
  : public CondContBase
{
public:
  /**
   * @brief Dump the container contents for debugging.
   * @param ost Stream to which to write the dump.
   */
  virtual
  void list (std::ostream& ost) const override final;


  /**
   * @brief Return all IOV validity ranges defined in this container.
   */
  virtual
  std::vector<EventIDRange> ranges() const override final;


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param obj Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * @c obj must point to an object of type @c T,
   * except in the case of inheritance, where the type of @c obj must
   * correspond to the most-derived @c CondCont type.
   * The container will take ownership of this object.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * EXTENDED if the last existing range in the container was extended
   * to match the new range;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  virtual
  StatusCode typelessInsert (const EventIDRange& r,
                             void* obj,
                             const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


  /**
   * @brief Test to see if a given IOV time is mapped in the container.
   * @param t IOV time to check.
   */
  virtual
  bool valid( const EventIDBase& t) const override final;


  /**
   * @brief Return the mapped validity range for an IOV time.
   * @param t IOV time to check.
   * @param r[out] The range containing @c t.
   *
   * Returns true if @c t is mapped; false otherwise.
   */
  virtual
  bool range (const EventIDBase& t, EventIDRange& r) const override final;


  /**
   * @brief Erase the first element not less than @c t.
   * @param IOV time of element to erase.
   * @param ctx Event context for the current thread.
   */
  virtual
  StatusCode erase (const EventIDBase& t,
                    const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


  /**
   * @brief Extend the range of the last IOV.
   * @param newRange New validity range.
   * @param ctx Event context.
   *
   * Returns failure if the start time of @c newRange does not match the start time
   * of the last IOV in the container.  Otherwise, the end time for the last
   * IOV is changed to the end time for @c newRange.  (If the end time for @c newRange
   * is before the end of the last IOV, then nothing is changed.)
   */
  virtual
  StatusCode extendLastRange (const EventIDRange& newRange,
                              const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


protected:
  /**
   * @brief Internal constructor.
   * @param rcusvc RCU service instance.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param payloadDeleter Object for deleting payload objects.
   * @param capacity Initial capacity of the container.
   */
  CondContSingleBase (Athena::IRCUSvc& rcusvc,
                      CLID clid,
                      const DataObjID& id,
                      SG::DataProxy* proxy,
                      std::shared_ptr<CondContSet::IPayloadDeleter> payloadDeleter,
                      size_t capacity);
};


///////////////////////////////////////////////////////////////////////////


template <class T> class CondCont;


/**
 * @brief Traits class to find the base for @c CondCont.
 *
 * @c CondCont<T> normally derives from @c CondContSingleBase.
 * However, if @c D derives from @c B, then using @c CONDCONT_BASE(D,B)
 * will cause @c ContCont<D> to derive from @c CondCont<B>.
 */
template <typename T>
class CondContBaseInfo
{
public:
  typedef CondContSingleBase Base;
};


namespace SG {
template <typename T>
struct Bases<CondCont<T> >
{
  using bases = BaseList<CondContBase>;
};
} // namespace SG




/**
 * @brief Declare that conditions object @c D derives from @c B.
 *
 * This allows using @c ReadCondHandle to retrieve a conditions object
 * of type @c D as @c B.
 */
#define CONDCONT_BASE(D, B)       \
template <>                       \
class CondContBaseInfo<D>         \
{                                 \
public:                           \
  typedef CondCont<B> Base;       \
};                                 \
SG_BASES(CondCont<D>, CondCont<B>);\
SG_BASES(D, B)
  



/**
 * @brief Hold mapping of ranges to condition objects.
 *
 * This object holds mappings from a set of IOV ranges (represented
 * as EventIDRange) to conditions objects (which are owned by this object).
 * It has methods to insert a new mapping and to retrieve objects
 * by IOV time.
 *
 * This object is recorded in the conditions store, so it must have a CLID
 * (the @c CondCont object, not @c T).  This CLID should be declared
 * using the  @c CONDCONT_DEF macro:
 *
 *@code
 *  CONDCONT_DEF(TYPE, CLID);
 @endcode
 *
 * It is possible for one conditions object to derive from another.
 * If @c TYPE derives from @c BASE, then declare this with a third
 * argument to @c CONDCONT_DEF:
 *
 *@code
 *  CONDCONT_DEF(TYPE, CLID, BASE);
 @endcode
 *
 * before any references to @c CondCont<TYPE>.
 * This is implemented by having @c CondCont<TYPE> derive from @c CondCont<BASE>.
 * In that case, the mappings will be stored only in the most-derived class.
 */
template <typename T>
class CondCont : public CondContBaseInfo<T>::Base
{
public:
  /// Base class.
  typedef typename CondContBaseInfo<T>::Base Base;

  typedef typename Base::CondContSet CondContSet;

  /// Payload type held by this class.
  typedef T Payload;

  typedef CondContBase::key_type key_type;


  /** 
   * @brief Constructor.
   * @param rcusvc RCU service instance.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param capacity Initial capacity of the container.
   */
  CondCont (Athena::IRCUSvc& rcusvc,
            const DataObjID& id,
            SG::DataProxy* proxy = nullptr,
            size_t capacity = 16);


  /// Destructor.
  virtual ~CondCont();

  /// No copying.
  CondCont (const CondCont&) = delete;
  CondCont& operator= (const CondCont&) = delete;


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param obj Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * @c obj must point to an object of type @c T.
   * This will give an error if this is not called
   * on the most-derived @c CondCont.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * EXTENDED if the last existing range in the container was extended
   * to match the new range;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  StatusCode insert (const EventIDRange& r,
                     std::unique_ptr<T> obj,
                     const EventContext& ctx = Gaudi::Hive::currentContext());


  /** 
   * @brief Look up a conditions object for a given time.
   * @param t IOV time to find.
   * @param obj[out] Object found.
   * @param r If non-null, copy validity range of the object here.
   *
   * Returns true if the object was found; false otherwide.
   */
  bool find (const EventIDBase& t,
             T const*& obj,
             EventIDRange const** r = nullptr) const;


  /** 
   * @brief Look up a conditions object for a given time.
   * @param t IOV time to find.
   *
   * Returns the found object, or nullptr.
   *
   * This variant may be more convenient to call from python.
   */
  const T* find (const EventIDBase& t) const;


protected:
  /**
   * @brief Internal constructor.
   * @param rcusvc RCU service instance.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param payloadDeleter Object for deleting payload objects.
   * @param capacity Initial capacity of the container.
   */
  CondCont (Athena::IRCUSvc& rcusvc,
            CLID clid,
            const DataObjID& id,
            SG::DataProxy* proxy,
            std::shared_ptr<typename CondContSet::IPayloadDeleter> payloadDeleter,
            size_t capacity);


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   */
  const void* cast (CLID clid, const void* ptr) const;


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   *
   * This is a virtual function that calls @c cast from the most-derived class
   * of the hierarchy.
   */
  virtual const void* doCast (CLID clid, const void* ptr) const override;


public:
  /// Helper to ensure that the inheritance information for this class
  /// gets initialized.
  static void registerBaseInit();
};


///////////////////////////////////////////////////////////////////////////


/**
 * @brief Base class for conditions containers for which keys are ranges
 *        in both Run+LBN and timestamp.
 */
class CondContMixedBase
  : public CondContBase
{
public:
  /**
   * @brief Dump the container contents for debugging.
   * @param ost Stream to which to write the dump.
   */
  virtual
  void list (std::ostream& ost) const override final;


  /**
   * @brief Return the number of conditions objects in the container.
   */
  virtual size_t entries() const override final;


  /**
   * @brief Return all IOV validity ranges defined in this container.
   */
  virtual
  std::vector<EventIDRange> ranges() const override final;


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param obj Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * @c obj must point to an object of type @c T,
   * except in the case of inheritance, where the type of @c obj must
   * correspond to the most-derived @c CondCont type.
   * The container will take ownership of this object.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  virtual
  StatusCode typelessInsert (const EventIDRange& r,
                             void* obj,
                             const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


  /**
   * @brief Test to see if a given IOV time is mapped in the container.
   * @param t IOV time to check.
   */
  virtual
  bool valid( const EventIDBase& t) const override final;


  /**
   * @brief Return the mapped validity range for an IOV time.
   * @param t IOV time to check.
   * @param r[out] The range containing @c t.
   *
   * Returns true if @c t is mapped; false otherwise.
   */
  virtual
  bool range (const EventIDBase& t, EventIDRange& r) const override final;


  /**
   * @brief Erase the first element not less than @c t.
   * @param IOV time of element to erase.
   * @param ctx Event context for the current thread.
   *
   * This is not implemented for mixed containers.
   */
  virtual
  StatusCode erase (const EventIDBase& t,
                    const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


  /**
   * @brief Extend the range of the last IOV.
   * @param newRange New validity range.
   * @param ctx Event context.
   *
   * Returns failure if the start time of @c newRange does not match the start time
   * of the last IOV in the container.  Otherwise, the end time for the last
   * IOV is changed to the end time for @c newRange.  (If the end time for @c newRange
   * is before the end of the last IOV, then nothing is changed.)
   *
   * This is not implemented for mixed containers.
   */
  virtual
  StatusCode extendLastRange (const EventIDRange& newRange,
                              const EventContext& ctx = Gaudi::Hive::currentContext()) override final;


protected:
  /**
   * @brief Internal constructor.
   * @param rcusvc RCU service instance.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param payloadDeleter Object for deleting actual payload objects.
   * @param capacity Initial capacity of the container.
   */
  CondContMixedBase (Athena::IRCUSvc& rcusvc,
                     CLID clid,
                     const DataObjID& id,
                     SG::DataProxy* proxy,
                     std::shared_ptr<CondContSet::IPayloadDeleter> payloadDeleter,
                     size_t capacity);


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param t Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  StatusCode insertMixed (const EventIDRange& r,
                          CondContBase::CondContSet::payload_unique_ptr t,
                          const EventContext& ctx = Gaudi::Hive::currentContext());


  /** 
   * @brief Internal lookup function.
   * @param t IOV time to find.
   * @param r If non-null, copy validity range of the object here.
   *
   * Looks up the conditions object corresponding to the IOV time @c t.
   * If found, return the pointer (as a pointer to the payload type
   * of the most-derived CondCont).  Otherwise, return nullptr.
   */
  const void* findMixed (const EventIDBase& t,
                         EventIDRange const** r) const;


  /**
   * @brief Return the payload deletion function for this container.
   */
  delete_function* payloadDelfcn() const;


private:
  /// Need to remember the RCU svc here in order to pass it to the
  /// TS maps.
  Athena::IRCUSvc& m_rcusvc;

  /// Deletion object for actual payload objects.
  std::shared_ptr<CondContSet::IPayloadDeleter> m_payloadDeleter;

  /// Mutex for insertions.
  std::mutex m_mutex;
};


///////////////////////////////////////////////////////////////////////////


template <class T> class CondContMixed;


/**
 * @brief Traits class to find the base for @c CondContMixed.
 *
 * @c CondContMixed<T> normally derives from @c CondContMixedBase.
 * However, if @c D derives from @c B, then using @c CONDCONTMIXED_BASE(D,B)
 * will cause @c ContContMixed<D> to derive from @c CondContMixed<B>.
 */
template <typename T>
class CondContMixedBaseInfo
{
public:
  typedef CondContMixedBase Base;
};


namespace SG {
template <typename T>
struct Bases<CondContMixed<T> >
{
  using bases = BaseList<CondContBase>;
};
} // namespace SG


/**
 * @brief Declare that conditions object @c D derives from @c B.
 *
 * This allows using @c ReadCondHandle to retrieve a conditions object
 * of type @c D as @c B.
 */
#define CONDCONT_MIXED_BASE(D, B)                                          \
template <>                                                                \
class CondContMixedBaseInfo<D>                                             \
{                                                                          \
public:                                                                    \
  static_assert (std::is_base_of_v<CondContMixed<B>, CondCont<B> >,        \
                 "CondCont<" #B "> is not a mixed conditions container."); \
  typedef CondCont<B> Base;                                                \
};                                                                         \
SG_BASES(CondContMixed<D>, CondCont<B>);                                   \
SG_BASES(D, B)
  



/**
 * @brief Conditions container for which keys are ranges  in both Run+LBN
 *        and timestamp.
 *
 * Don't use this directly; instead declare your container with
 * @c CONDCONT_MIXED_DEF and then use @c CondCont<T>.
 */
template <typename T>
class CondContMixed : public CondContMixedBaseInfo<T>::Base
{
public:
  /// Base class.
  typedef typename CondContMixedBaseInfo<T>::Base Base;

  typedef typename Base::CondContSet CondContSet;

  /// Payload type held by this class.
  typedef T Payload;

  typedef CondContBase::key_type key_type;


  /// Destructor.
  virtual ~CondContMixed();

  /// No copying.
  CondContMixed (const CondContMixed&) = delete;
  CondContMixed& operator= (const CondContMixed&) = delete;


  /** 
   * @brief Insert a new conditions object.
   * @param r Range of validity of this object.
   * @param obj Pointer to the object being inserted.
   * @param ctx Event context for the current thread.
   *
   * @c obj must point to an object of type @c T.
   * This will give an error if this is not called
   * on the most-derived @c CondCont.
   *
   * Returns SUCCESS if the object was successfully inserted;
   * OVERLAP if the object was inserted but the range partially overlaps
   * with an existing one;
   * DUPLICATE if the object wasn't inserted because the range
   * duplicates an existing one, and FAILURE otherwise
   * (ownership of the object will be taken in any case).
   */
  StatusCode insert (const EventIDRange& r,
                     std::unique_ptr<T> obj,
                     const EventContext& ctx = Gaudi::Hive::currentContext());


  /** 
   * @brief Look up a conditions object for a given time.
   * @param t IOV time to find.
   * @param obj[out] Object found.
   * @param r If non-null, copy validity range of the object here.
   *
   * Returns true if the object was found; false otherwide.
   */
  bool find (const EventIDBase& t,
             T const*& obj,
             EventIDRange const** r = nullptr) const;


  /**
   * @brief Look up a conditions object for a given time.
   * @param t IOV time to find.
   *
   * Returns the found object, or nullptr.
   *
   * This variant may be more convenient to call from python.
   */
  const T* find (const EventIDBase& t) const;


protected:
  /** 
   * @brief Internal Constructor.
   * @param rcusvc RCU service instance.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param payloadDeleter Object for deleting actual payload objects.
   * @param capacity Initial capacity of the container.
   */
  CondContMixed (Athena::IRCUSvc& rcusvc,
                 CLID clid,
                 const DataObjID& id,
                 SG::DataProxy* proxy,
                 std::shared_ptr<typename CondContSet::IPayloadDeleter> payloadDeleter,
                 size_t capacity);


  /** 
   * @brief Internal Constructor.
   * @param rcusvc RCU service instance.
   * @param CLID of the most-derived @c CondCont.
   * @param id CLID+key for this object.
   * @param proxy @c DataProxy for this object.
   * @param capacity Initial capacity of the container.
   */
  CondContMixed (Athena::IRCUSvc& rcusvc,
                 CLID clid,
                 const DataObjID& id,
                 SG::DataProxy* proxy,
                 size_t capacity);


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   */
  const void* cast (CLID clid, const void* ptr) const;


  /**
   * @brief Do pointer conversion for the payload type.
   * @param clid CLID for the desired pointer type.
   * @param ptr Pointer of type @c T*.
   *
   * Converts @c ptr from @c T* to a pointer to the type
   * given by @c clid.  Returns nullptr if the conversion
   * is not possible.
   *
   * This is a virtual function that calls @c cast from the most-derived class
   * of the hierarchy.
   */
  virtual const void* doCast (CLID clid, const void* ptr) const override;


public:
  /// Helper to ensure that the inheritance information for this class
  /// gets initialized.
  static void registerBaseInit();
};


///////////////////////////////////////////////////////////////////////////


#include "AthenaKernel/CondCont.icc"
#include "AthenaKernel/CondContMaker.h"

#define CONCATUNF_(x,y) x##y
#define CONCATUNF(x,y) CONCATUNF_(x,y)
#define UNIQUEVARNAME CONCATUNF(CONCATUNF(REGCCM_,__COUNTER__),__LINE__)


/// Declare a conditions container along with its CLID.
// For a conditions container not deriving from another, do
//    CONDCONT_DEF(TYPE, CLID);
//
// For a conditions container with a payload deriving from BASE, do
//    CONDCONT_DEF(TYPE, CLID, BASE);
//
#define CONDCONT_DEF_2(T, CLID_)    \
  CLASS_DEF( CondCont<T>, CLID_, 1) \
  static CondContainer::CondContMaker<T> maker_ ## CLID_ {}
#define CONDCONT_DEF_3(T, CLID_, BASE)           \
  CONDCONT_BASE(T, BASE);                        \
  CONDCONT_DEF_2(T, CLID_)
#define CONDCONT_DEF(...)  \
  BOOST_PP_OVERLOAD(CONDCONT_DEF_, __VA_ARGS__)(__VA_ARGS__)


/// Declare a conditions container along with its CLID.
// For a conditions container not deriving from another, do
//    CONDCONT_MIXED_DEF(TYPE, CLID);
//
// For a conditions container with a payload deriving from BASE, do
//    CONDCONT_MIXED_DEF(TYPE, CLID, BASE);
//
// The BASE class must have earlier been named in another CONDCONT_MIXED_DEF.
//
#define CONDCONT_MIXED_DEF_2(T, CLID_)                                   \
  template<> class CondCont<T> : public CondContMixed<T> {               \
  public:                                                                \
    CondCont (Athena::IRCUSvc& rcusvc, const DataObjID& id,              \
              SG::DataProxy* proxy =nullptr, size_t capacity = 16)       \
      : CondContMixed<T> (rcusvc, CLID_, id, proxy, capacity) {}         \
  protected:                                                             \
    CondCont (Athena::IRCUSvc& rcusvc, CLID clid, const DataObjID& id,   \
              SG::DataProxy* proxy,                                      \
              std::shared_ptr<typename CondContSet::IPayloadDeleter> payloadDeleter, \
              size_t capacity = 16)                                      \
      : CondContMixed<T> (rcusvc, clid, id, proxy,                       \
                          payloadDeleter, capacity) {}                   \
  };                                                                     \
  CLASS_DEF( CondCont<T>, CLID_, 1)                                      \
  SG_BASES(CondCont<T>, CondContMixed<T>);                               \
  static CondContainer::CondContMaker<T> maker_ ## CLID_ {}
#define CONDCONT_MIXED_DEF_3(T, CLID_, BASE)                             \
  CONDCONT_MIXED_BASE(T, BASE);                                          \
  CONDCONT_MIXED_DEF_2(T, CLID_)
#define CONDCONT_MIXED_DEF(...)                                          \
  BOOST_PP_OVERLOAD(CONDCONT_MIXED_DEF_, __VA_ARGS__)(__VA_ARGS__)

  
#endif // not ATHENAKERNEL_CONDCONT_H

