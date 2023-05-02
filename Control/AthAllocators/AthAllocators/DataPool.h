/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHALLOCATORS_DATAPOOL_H
#define ATHALLOCATORS_DATAPOOL_H
/** @class DataPool
 * @brief  a typed memory pool that saves time spent
 *         allocation small object. This is typically used
 *         by container such as DataVector and DataList
 *
 * A DataPool instance, acts as handle to the pool for the declared type.
 * In the typical DataVector usage each store has a collection of pools
 * distinguished by type. So for the same active store the
 * same pool is used.
 *
 * Declaring @c DataPool instances as static will
 * cause thread-safety problems, and thus should no longer be done.
 *
 * Also be aware that a creating a DataPool object acquires a lock on the
 * underlying thread-specific allocator.  If you're just using one at a time,
 * there shouldn't be a problem. There is a  potential for deadlock if you have
 * a block of code that creates two DataPool objects, and another code block
 * that creates DataPool objects for the same types but in the opposite order.
 *
 * You can optionally provide (as a template argument) a function to be called
 * on an object when it is returned to the pool.  This can be used to reset
 * the state, release memory, etc.  This must be a static function, not
 * a lambda, and since it's used as a template argument, it should not be
 * in an anonymous namespace (should have public linkage).  Also be aware
 * that @c clear argument will have an effect only for the first @c DataPool
 * object to be created for a given @c VALUE.
 *
 * @author Srini Rajagopalan, scott snyder
 */

#include "AthAllocators/ArenaCachingHandle.h"
#include "AthAllocators/ArenaPoolAllocator.h"
#include <string>
#include "boost/iterator/iterator_adaptor.hpp"

template <typename VALUE>
using DataPoolClearFuncPtr_t = void (*)(VALUE*);

template <typename VALUE, DataPoolClearFuncPtr_t<VALUE> clear = nullptr>
class DataPool
{
private:
  typedef SG::ArenaPoolAllocator alloc_t;
  typedef SG::ArenaCachingHandle<VALUE, alloc_t> handle_t;

public:
  typedef typename handle_t::pointer pointer;
  typedef size_t size_type;

  class const_iterator;

  class iterator
    : public boost::iterator_adaptor<
    iterator,
    typename handle_t::iterator,
    VALUE *,
    boost::forward_traversal_tag,
    VALUE *>
  {
  public:
    iterator (const typename handle_t::iterator& it)
      : iterator::iterator_adaptor_ (it)
    {
    }

    friend class const_iterator;

  private:
    friend class boost::iterator_core_access;

    typename iterator::reference dereference() const
    { return &*this->base_reference(); }
  };

  class const_iterator
    : public boost::iterator_adaptor<
    const_iterator,
    typename handle_t::const_iterator,
    VALUE const *,
    boost::forward_traversal_tag,
    VALUE const *>
  {
  public:
    const_iterator (const typename handle_t::const_iterator& it)
      : const_iterator::iterator_adaptor_ (it)
    {
    }

    const_iterator (const iterator& it)
      : const_iterator::iterator_adaptor_ (it.base_reference())
    {
    }

  private:
    friend class boost::iterator_core_access;

    typename const_iterator::reference dereference() const
    { return &*this->base_reference(); }
  };


  //////////////////////////////////////////////////////////////////////
  /// Constructors:
  //////////////////////////////////////////////////////////////////////

  DataPool(size_type n = 0);

  DataPool(const EventContext& ctx,
           size_type n = 0);

  DataPool(SG::Arena* arena,
           size_type n = 0);

  ///////////////////////////////////////////////////////

  /// release all elements in the pool.
  void reset();

  /// free all memory in the pool.
  void erase();

  /**
   * @brief Set the desired capacity
   * @param size The desired capacity
   *
   * If @c size is greater than the total number of elements currently
   * cached (allocated), then space for more will be allocated.
   * This will be done in blocks. Therefore the allocator may allocate
   * more elements than is requested.
   *
   * If @c size is smaller than the total number of elements currently
   * cached, as many blocks as possible will be released back to the system.
   */
  void reserve(unsigned int size);

  /**
   * @brief Prepare to add cached elements
   * @param size Additional elements to add
   *
   *  If (current capacity - allocated elements)
   *  is less than the requested elements
   *  additional space will be allocated by
   *  calling reserve (allocated + size).
   *  Otherwise nothing will be done
   */
  void prepareToAdd(unsigned int size);

  /// return capacity of pool OK
  unsigned int capacity();

  /// return size already allocated OK
  unsigned int allocated();

  /// begin iterators over pool
  iterator begin();
  const_iterator begin() const;

  /// the end() method will allow looping over only valid elements
  /// and not over ALL elements of the pool
  iterator end();
  const_iterator end() const;

  /// obtain the next available element in pool by pointer
  /// pool is resized if its limit has been reached
  /// One must be sure to completely reset each object
  /// since it has values set in the previous event.
  pointer nextElementPtr();

  /// typename of pool
  static const std::string& typeName();

//-----------------------------------------------------------//

 private:

  handle_t m_handle;

  const static typename alloc_t::Params s_params;

  /// minimum number of elements in pool
  static constexpr size_t s_minRefCount = 1024;

  static typename alloc_t::Params initParams();
  static void callClear (SG::ArenaAllocatorBase::pointer p);
};


#include "AthAllocators/DataPool.icc"

#endif


