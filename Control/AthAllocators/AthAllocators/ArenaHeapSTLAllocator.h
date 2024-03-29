// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  AthAllocators/ArenaHeapSTLAllocator.h
 * @author scott snyder
 * @date Nov 2011
 * @brief STL-style allocator wrapper for @c ArenaHeapAllocator.
 *
 * This class defines a STL-style allocator for types @c T with the
 * following special properties.
 *
 *  - We use an @c ArenaHeapAllocator for allocations.
 *  - The memory is owned directly by the allocator object.
 *  - Only one object at a time may be allocated.
 *
 * So, this allocator is suitable for an STL container which allocates
 * lots of fixed-size objects, such as @c std::list.
 *
 * Much of the complexity here is due to the fact that the allocator
 * type that gets passed to the container is an allocator for the
 * container's value_type, but that allocator is not actually
 * used to allocate memory.  Instead, an allocator for the internal
 * node type is used.  This makes it awkward if you want to have
 * allocators that store state.
 *
 * Further, to avoid creating a pool for the allocator for the container's
 * value_type (when the container doesn't actually use that for allocation),
 * this template has a second argument.  This defaults to the first argument,
 * but is passed through by rebind.  If the first and second argument
 * are the same, then we don't create a pool.
 *
 * The effect of all this is that you can give an allocator type like
 *   ArenaHeapSTLAllocator<Mytype>
 * to a STL container.  Allocators for Mytype won't use
 * a pool, but an allocator for node<Mytype> will use the pool.
 */


#ifndef ATLALLOCATORS_ARENAHEAPSTLALLOCATOR
#define ATLALLOCATORS_ARENAHEAPSTLALLOCATOR


#include "AthAllocators/ArenaHeapAllocator.h"
#include "CxxUtils/concepts.h"
#include "CxxUtils/checker_macros.h"
#include <string>
#include <cstddef>


namespace SG {


/**
 * @brief Initializer for pool allocator parameters.
 *
 *        We override the defaults to disable calling the payload ctor/dtor.
 */
template <class T>
class ArenaHeapSTLAllocator_initParams
  : public ArenaHeapAllocator::initParams<T, false, true, true>
{
public:
  /// We take defaults from this.
  typedef ArenaHeapAllocator::initParams<T, false, true, true> Base;

    /**
     * @brief Constructor.
     * @param nblock Value to set in the parameters structure for the
     *               number of elements to allocate per block.
     * @param name   Value to set in the parameters structure for the
     *               allocator name.
     */
    ArenaHeapSTLAllocator_initParams (size_t nblock = 1000,
                                      const std::string& name = "");

    /// Return an initialized parameters structure.
    ArenaAllocatorBase::Params params() const;

    /// Return an initialized parameters structure.
    // Note: gcc 3.2.3 doesn't allow defining this out-of-line.
    operator ArenaAllocatorBase::Params() const { return params(); }
};


/**
 * @brief STL-style allocator wrapper for @c ArenaHeapAllocator.
 *        This is the generic specialization, which uses the heap allocator.
 *
 * See the file-level comments for details.
 */
template <class T, class VETO=T>
class ArenaHeapSTLAllocator
{
public:
  /// Standard STL allocator typedefs.
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;

  /// When we assign to a container, the target should retain its allocator.
  typedef std::false_type propagate_on_container_copy_assignment;

  /// Move allocators on move/swap.
  typedef std::true_type propagate_on_container_move_assignment;
  typedef std::true_type propagate_on_container_swap;


  /// Standard STL allocator rebinder.
  template <class U> struct rebind {
    typedef ArenaHeapSTLAllocator<U, VETO> other;
  };


  /**
   * @brief Default constructor.
   * @param nblock Value to set in the parameters structure for the
   *               number of elements to allocate per block.
   * @param name   Value to set in the parameters structure for the
   *               allocator name.
   */
  ArenaHeapSTLAllocator (size_t nblock = 1000, const std::string& name = "");


  /**
   * @brief Copy constructor.
   *
   * The @c name and @c nblock parameters are copied, but the data are not.
   */
  ArenaHeapSTLAllocator (const ArenaHeapSTLAllocator& a);


  /**
   * @brief Constructor from another @c ArenaHeapSTLAllocator.
   *
   * The @c name and @c nblock parameters are copied, but the data are not.
   */
  template <class U, class V>
  ArenaHeapSTLAllocator (const ArenaHeapSTLAllocator<U, V>& a);


  /**
   * @brief Move constructor.
   *
   * Move the data.
   */
  ArenaHeapSTLAllocator (ArenaHeapSTLAllocator&& a);


  /**
   * @brief Move assignment.
   *
   * Move the data.
   */
  ArenaHeapSTLAllocator& operator= (ArenaHeapSTLAllocator&& a);


  /**
   * @brief Swap.
   */
  void swap (ArenaHeapSTLAllocator& a);


  /**
   * @brief Return allocator to use for a copy-constructed container.
   *
   * When we copy-construct a container, we want the new allocator
   * to copy parameters from the old one, but not the data.
   */
  ArenaHeapSTLAllocator select_on_container_copy_construction() const;


  /**
   * @brief Equality test.
   *
   * Two allocators should compare equal if objects allocated by one
   * can be deallocated by the other.  We should just check if they
   * are the same object.
   */
  bool operator== (const ArenaHeapSTLAllocator& other) const;


  /**
   * @brief Inequality test.
   *
   * Two allocators should compare equal if objects allocated by one
   * can be deallocated by the other.  We should just check if they
   * are the same object.
   */
  bool operator!= (const ArenaHeapSTLAllocator& other) const;


  // We don't bother to supply a more general constructor --- shouldn't
  // be needed.


  /// Convert a reference to an address.
  pointer address (reference x) const;
  ATH_MEMBER_REQUIRES(!(std::is_same_v<reference,const_reference>),
                      const_pointer)
  address (const_reference x) const { return &x; }


  /**
   * @brief Allocate new objects.
   * @param n Number of objects to allocate.  Must be 1.
   * @param hint Allocation hint.  Not used.
   */
  pointer allocate (size_type n, const void* hint = 0);


  /**
   * @brief Deallocate objects.
   * @param n Number of objects to deallocate.  Must be 1.
   */
  void deallocate (pointer, size_type n);
  ATH_MEMBER_REQUIRES(!(std::is_same_v<pointer,const_pointer>), void)
  deallocate (const_pointer p, size_type n) const
  {
    pointer p_nc ATLAS_THREAD_SAFE = const_cast<pointer>(p);
    deallocate (p_nc, n);
  }


  /**
   * @brief Return the maximum number of objects we can allocate at once.
   *
   * This always returns 1.
   */
  size_type max_size() const throw();


  /**
   * @brief Call the @c T constructor.
   * @param p Location of the memory.
   * @param args Arguments to pass to the constructor.
   */
  template <class... Args>
  void construct (pointer p, Args&&... args);


  /**
   * @brief Call the @c T destructor.
   * @param p Location of the memory.
   */
  void destroy (pointer p);


  /**
   * @brief Return the hinted number of objects allocated per block.
   */
  size_t nblock() const;


  /**
   * @brief Return the name of this allocator.
   */
  const std::string& name() const;


  /**
   * @brief Free all allocated elements.
   *
   * All elements allocated are returned to the free state.
   * @c clear should be called on them if it was provided.
   * The elements may continue to be cached internally, without
   * returning to the system.
   */
  void reset();


  /**
   * @brief Free all allocated elements and release memory back to the system.
   *
   * All elements allocated are freed, and all allocated blocks of memory
   * are released back to the system.
   * @c destructor should be called on them if it was provided
   * (preceded by @c clear if provided and @c mustClear was set).
   */
  void erase();


  /**
   * @brief Set the total number of elements cached by the allocator.
   * @param size The desired pool size.
   *
   * This allows changing the number of elements that are currently free
   * but cached.  Any allocated elements are not affected by this call.
   *
   * If @c size is greater than the total number of elements currently
   * cached, then more will be allocated.  This will preferably done
   * with a single block, but that is not guaranteed; in addition, the
   * allocator may allocate more elements than is requested.
   *
   * If @c size is smaller than the total number of elements currently
   * cached, as many blocks as possible will be released back to the system.
   * It may not be possible to release the number of elements requested;
   * this should be implemented on a best-effort basis.
   */
  void reserve (size_t size);


  /**
   * @brief Return the statistics block for this allocator.
   */
  ArenaAllocatorBase::Stats stats() const;


  /**
   * @brief Return a pointer to the underlying allocator (may be 0).
   */
  const ArenaBlockAllocatorBase* poolptr() const;


  /**
   * @brief Write-protect the memory managed by this allocator.
   *
   * Adjust protection on the memory managed by this allocator
   * to disallow writes.
   */
  void protect();


  /**
   * @brief Write-enable the memory managed by this allocator.
   *
   * Adjust protection on the memory managed by this allocator
   * to allow writes.
   */
  void unprotect();


private:
  /// The underlying allocator.
  ArenaHeapAllocator m_pool;
};



/// Forward declaration.
template <class T>
class ArenaNonConstHeapSTLAllocator;


/**
 * @brief STL-style allocator wrapper for @c ArenaHeapAllocator.
 *        This is the specialization for the case of the vetoed type.
 *
 *
 *        We want to allow calling the non-const allocator methods
 *        reset(), erase(), and reserve() only if the corresponding
 *        container is non-const.  However, we can't really do that,
 *        since the get_allocator() method of containers is only const,
 *        and returns an allocator by value.  So instead, we split up this
 *        class.  @c ArenaHeapSTLAllocator holds a const pointer to the
 *        underlying allocator, and supports only the const methods on it.
 *        Then @c ArenaNonConstHeapSTLAllocator derives from it and
 *        implements the non-const methods.  To get an instance of the
 *        latter class, call ArenaHeapSTLAllocator::get_allocator(c),
 *        where @c is the container --- and @c must be non-const.
 *
 * See the file-level comments for details.
 */
template <class T>
class ArenaHeapSTLAllocator<T, T>
  : public std::allocator<T>
{
public:
  typedef std::allocator<T> base;

  /// Standard STL allocator typedefs.
  typedef typename base::value_type      value_type;
  typedef typename base::size_type       size_type;
  typedef typename base::difference_type difference_type;


  /// Standard STL allocator rebinder.
  template <class U> struct rebind {
    typedef ArenaHeapSTLAllocator<U, T> other;
  };


  /**
   * @brief Default constructor.
   * @param nblock Value to set in the parameters structure for the
   *               number of elements to allocate per block.
   * @param name   Value to set in the parameters structure for the
   *               allocator name.
   */
  ArenaHeapSTLAllocator (size_t nblock = 1000, const std::string& name = "");


  /**
   * @brief Constructor from another @c ArenaHeapSTLAllocator.
   *
   * The @c name and @c nblock parameters are copied, but the data are not.
   */
  template <class U, class V>
  ArenaHeapSTLAllocator (const ArenaHeapSTLAllocator<U, V>& a);


  // We don't bother to supply a more general constructor --- shouldn't
  // be needed.


  /**
   * @brief Return the hinted number of objects allocated per block.
   */
  size_t nblock() const;


  /**
   * @brief Return the name of this allocator.
   */
  const std::string& name() const;


  /**
   * @brief Return the statistics block for this allocator.
   */
  ArenaAllocatorBase::Stats stats() const;


  /**
   * @brief Return a pointer to the underlying allocator (may be 0).
   */
  const ArenaBlockAllocatorBase* poolptr() const;


  /**
   * @brief Return an allocator supporting non-const methods from
   *        a non-const container reference.
   * @param c The (non-const) container.
   */
  template <class CONT>
  static
  ArenaNonConstHeapSTLAllocator<T> get_allocator (CONT& c);


private:
  /// Saved hinted number of objects per block.
  size_t m_nblock;

  /// Saved allocator name.
  std::string m_name;

  /// Point at an underlying allocator from a different specialization.
  const ArenaBlockAllocatorBase* m_poolptr;
};


/**
 * @brief STL-style allocator wrapper for @c ArenaHeapAllocator.
 *        Non-const variant for the case of the vetoed type.
 *
 *        See documentation above for details.
 */
template <class T>
class ArenaNonConstHeapSTLAllocator
  : public ArenaHeapSTLAllocator<T, T>
{
public:
  /**
   * @brief Constructor.
   * @param a Allocator to reference.
   * @param poolptr_nc Non-const pointer to the underlying allocator.
   */
  template <class U, class V>
  ArenaNonConstHeapSTLAllocator (const ArenaHeapSTLAllocator<U, V>& a,
                                 ArenaBlockAllocatorBase* poolptr_nc);


  /**
   * @brief Free all allocated elements.
   *
   * All elements allocated are returned to the free state.
   * @c clear should be called on them if it was provided.
   * The elements may continue to be cached internally, without
   * returning to the system.
   */
  void reset();


  /**
   * @brief Free all allocated elements and release memory back to the system.
   *
   * All elements allocated are freed, and all allocated blocks of memory
   * are released back to the system.
   * @c destructor should be called on them if it was provided
   * (preceded by @c clear if provided and @c mustClear was set).
   */
  void erase();


  /**
   * @brief Set the total number of elements cached by the allocator.
   * @param size The desired pool size.
   *
   * This allows changing the number of elements that are currently free
   * but cached.  Any allocated elements are not affected by this call.
   *
   * If @c size is greater than the total number of elements currently
   * cached, then more will be allocated.  This will preferably done
   * with a single block, but that is not guaranteed; in addition, the
   * allocator may allocate more elements than is requested.
   *
   * If @c size is smaller than the total number of elements currently
   * cached, as many blocks as possible will be released back to the system.
   * It may not be possible to release the number of elements requested;
   * this should be implemented on a best-effort basis.
   */
  void reserve (size_t size);


  /**
   * @brief Write-protect the memory managed by this allocator.
   *
   * Adjust protection on the memory managed by this allocator
   * to disallow writes.
   */
  void protect();


  /**
   * @brief Write-enable the memory managed by this allocator.
   *
   * Adjust protection on the memory managed by this allocator
   * to allow writes.
   */
  void unprotect();


private:
  /// Non-const pointer to the underlying allocator.
  ArenaBlockAllocatorBase* m_poolptr_nc;
};


/**
 * @brief Hook for unprotecting an arena.
 *
 * Sometimes we need to ensure that an arena is unprotected before we start
 * destroying an object that contains the arena.  To do that without
 * making assumptions about whether the arena supports an unprotect
 * operation, call this function.
 */
template <class T, class VETO>
void maybeUnprotect (ArenaHeapSTLAllocator<T, VETO>& a);


} // namespace SG


#include "AthAllocators/ArenaHeapSTLAllocator.icc"


#endif // ATLALLOCATORS_ARENAHEAPSTLALLOCATOR
