// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/ViewVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Identify view containers to be made persistent.
 */


#ifndef ATHCONTAINERS_VIEWVECTOR_H
#define ATHCONTAINERS_VIEWVECTOR_H


#include "AthContainers/OwnershipPolicy.h"
#include "AthContainers/IndexTrackingPolicy.h"
#include "AthContainers/exceptions.h"
#include "AthContainers/ViewVectorBase.h"
#include "AthLinks/ElementLink.h"
#include <atomic>


/**
 * @brief Identify view containers to be made persistent.
 *
 * This is a variant of @c DataVector that can only be constructed as a
 * view vector.  This class also holds the persistent representation
 * of the view vector, which is logically equivalent to a 
 * @c std::vector<ElementLink<DV> > (but stored as vector of pairs
 * of integers).
 *
 * The template parameter @c DV is the @c DataVector on which the view
 * is based. @c ViewVector<DV> acts just like @c DV except that it will
 * throw an exception if you try to construct it as an owning container.
 * (The ownership mode arguments cannot really be removed from the methods
 * completely, otherwise ConstDataVector<ViewVector<DV> > wouldn't compile.)
 *
 * To make a @c ViewVector<DV> object persistent:
 *  - Instead of using @c CLASS_DEF, to associate a CLID with the object,
 *    use @c VIEWVECTOR_CLASS_DEF.  You can then record a @c ViewVector
 *    to StoreGate without the @c VIEWVECTOR_CLASS_DEF being present
 *    in the compilation unit doing the record.  You can also record
 *    a @c ViewVector object using a pointer to the base @c DataVector.
 *    These behaviors can be useful when the @c ViewVector type
 *    is generated by templated code.
 *    (Currently, this will only work directly if the argument to the
 *    @c ViewVector class is a @c DataVector directly, rather than
 *    an intermediate class.)
 *  - In the AthenaPool package, declare the class in the requirements file,
 *    but do not implement it.  The default generated converter will get
 *    specialized appropriately for the ViewVector.
 *  - Previous versions of @c ViewVector were saved directly
 *    as @c std::vector<ElementLink<DV> >.  If you need to  continue to read
 *    such data, then generate a dictionary for
 *    @c std::vector<ElementLink<DV> > along with a guid.
 *    (You can use the @c Pers_t typedef in @c ViewVector
 *    to help with this.)
 *  - In a standalone root application, you must explicitly call
 *    @c toPersistent() on the @c ViewVector object prior to calling
 *    @c Fill() on the tree.
 */
template <class DV>
class ViewVector
  : public DV, public SG::ViewVectorBase
{
public:
  /// Basic types, forwarded from the base.
  typedef typename DV::size_type                size_type;
  typedef typename DV::difference_type          difference_type;
  typedef typename DV::allocator_type           allocator_type;
  typedef typename DV::base_value_type          base_value_type;
  typedef typename DV::BaseContainer            BaseContainer;
  typedef typename DV::DVL_BASE                 DVL_BASE;
  typedef typename DV::const_iterator           const_iterator;
  typedef typename DV::const_reverse_iterator   const_reverse_iterator;
  typedef typename DV::value_type               value_type;
  typedef typename DV::const_value_type         const_value_type;
  typedef typename DV::reference                reference;
  typedef typename DV::const_reference          const_reference;
  typedef typename DV::pointer                  pointer;
  typedef typename DV::const_pointer            const_pointer;
  typedef typename DV::unique_type              unique_type;

  /// If true, then this type must own its contents.
  // cppcheck-suppress duplInheritedMember
  static constexpr bool must_own = DV::must_own;

  /// The old persistent form of this class.
  typedef std::vector<ElementLink<DV> > Pers_t;


  //========================================================================
  /** @name Constructors, destructors, assignment. */
  //@{


  /**
   * @brief Default constructor.
   * @param ownPolicy The ownership mode for the container.
   *                  Must be @c SG::VIEW_ELEMENTS.
   *                  (Argument present only for interface compatibility.)
   */
  explicit ViewVector(SG::OwnershipPolicy ownPolicy = SG::VIEW_ELEMENTS);


  /**
   * @brief Sized constructor.
   * @param n The size of the container.
   * @param ownPolicy The ownership mode for the container.
   *                  Must be @c SG::VIEW_ELEMENTS.
   *                  (Argument present only for interface compatibility.)
   *
   * Note that unlike the standard vector constructor, you can't specify
   * an initial value here.  The container will be initialized with 0's.
   */
  explicit ViewVector(size_type n,
                      SG::OwnershipPolicy ownPolicy = SG::VIEW_ELEMENTS);


  /**
   * @brief Constructor from iterators.
   * @param first The start of the range to put in the new container.
   * @param last The end of the range to put in the new container.
   */
  template <class InputIterator>
  ViewVector(InputIterator first, InputIterator last);


  /// Use the compiler-generated copy constructor.
  ViewVector (const ViewVector&) = default;


  /**
   * @brief Move constructor.
   * @param rhs The container from which to move.
   *
   * Any auxiliary data will be moved along with the container contents.
   */
  ViewVector (ViewVector&& rhs);


  /**
   * @brief Constructor from base vector.
   * @param rhs The container from which to copy.
   */
  ViewVector (const DV& rhs);


  /**
   * @brief Move constructor from base vector.
   * @param rhs The container from which to copy.  Must be a view container.
   *
   * Any auxiliary data will be moved along with the container contents.
   */
  ViewVector (DV&& rhs);


  /**
   * @brief Constructor from an initializer list.
   * @param l An initializer list.
   */
  ViewVector(std::initializer_list<value_type> l);


  /// Can use compiler-generated assignment op.
  ViewVector& operator= (const ViewVector& rhs) = default;


  /**
   * @brief Assignment operator.
   * @param rhs The DataVector from which to assign.
   * @return This object.
   *
   * This is a `shallow' copy; after the completion of this, the DataVector
   * will not own its elements.  Any elements it owned prior to this call
   * will be released.
   *
   * Note: this method may only be called using the most derived
   * @c DataVector in the hierarchy.
   */
  ViewVector& operator= (const DV& rhs);


  /**
   * @brief Move assignment.
   * @param rhs The container from which to move.
   *
   * Any auxiliary data will be moved along with the container contents.
   */
  ViewVector& operator= (ViewVector&& rhs);


  /**
   * @brief Move assignment from base vector.
   * @param rhs The container from which to move.
   *            Must be a view vector.
   *
   * Any auxiliary data will be moved along with the container contents.
   */
  ViewVector& operator= (DV&& rhs);


  /**
   * @brief Assignment operator, from an initializer list.
   * @param l An initializer list.
   * @return This object.
   *
   * This is equivalent to @c assign.
   */
  ViewVector& operator= (std::initializer_list<value_type> l);


  /// Get the standard clear() from the base class.
  using DV::clear;
  
  
  /**
   * @fn void clear
   * @brief Erase all the elements in the collection.
   * @param ownPolicy The new ownership policy of the container.
   *                  Must be SG::VIEW_ELEMENTS.
   *                  (Argument present only for interface compatibility.)
   */
  void clear (SG::OwnershipPolicy ownPolicy);


  /**
   * @fn void clear
   * @brief Erase all the elements in the collection.
   * @param ownPolicy The new ownership policy of the container.
   *                  Must be SG::VIEW_ELEMENTS.
   *                  (Argument present only for interface compatibility.)
   * @param trackIndices The index tracking policy.
   */
  void clear (SG::OwnershipPolicy ownPolicy,
              SG::IndexTrackingPolicy trackIndices);


  /**
   * @brief Convert the vector to persistent form.
   */
  virtual void toPersistent() override;


  /**
   * @brief Convert the vector to transient form.
   */
  virtual void toTransient() override;


private:
  /// Helper to ensure that the inheritance information for this class
  /// gets initialized.
  void registerBaseInit();
};


#ifndef XAOD_STANDALONE
#include "AthenaKernel/BaseInfo.h"
namespace SG {


/// Specialize to declare that ViewVector<DV> derives from DV.
template<class DV>
struct Bases<ViewVector<DV> > {
  using bases = BaseList<DV>;
};                               


}
#endif


#include "AthContainers/ViewVector.icc"


#ifndef XAOD_STANDALONE
/**
 * @brief ClassID_traits specialization for @c ViewVector.
 *
 * We want to allow SG operations on a @c ViewVector even when the
 * CLASS_DEF for the @c ViewVector isn't present in the compilation unit
 * doing the record.  For example, we may have some piece of code
 * templated on a @c DataVector class @c DV that creates a @c ViewVector<DV>
 * and records it.  When this is used, the header defining @c should
 * be included, but the one defining @c ViewVector<DV> may not be.
 * But we'd like the record to work anyway.
 *
 * So, for @c ViewVector, we use this specialization of @c ClassID_traits.
 * It stores the CLID and the name (which recall will usually be a typedef
 * name, so we can't get it from the @c DV template argument) as static
 * class members.  We define a macro @c VIEWVECTOR_CLASS_DEF which then
 * sets these class statics during global initialization.
 */
template <class DV>
struct ClassID_traits< ViewVector<DV> >
{
  static const bool s_isDataObject = false;
  typedef std::integral_constant<bool, s_isDataObject> is_DataObject_tag;
  typedef std::true_type has_classID_tag;

  static const CLID& ID() {
    if (s_clid == CLID_NULL)
      SG::throwExcMissingViewVectorCLID (typeid(ViewVector<DV>));
    static const CLID clid = s_clid;
    return clid;
  }
  static const char* typeNameString() {
    if (s_name == nullptr)
      SG::throwExcMissingViewVectorCLID (typeid(ViewVector<DV>));
    return s_name;
  }
  static const std::string& typeName() {
    static const std::string name = typeNameString();  return name;
  }
  static const std::type_info& typeInfo() {
    return typeid (ViewVector<DV>);
  }
  typedef std::false_type has_version_tag;
  static const bool s_isConst = false;

  static bool init (CLID clid, const char* name)
  {
    if (s_clid == CLID_NULL) {
      s_clid = clid;
      s_name = name;
    }
    return true;
  }
private:
  static std::atomic<CLID> s_clid;
  static std::atomic<const char*> s_name;
};

template <class DV>
std::atomic<CLID> ClassID_traits< ViewVector<DV> >::s_clid { CLID_NULL };
template <class DV>
std::atomic<const char*> ClassID_traits< ViewVector<DV> >::s_name { nullptr };
#endif // not XAOD_STANDALONE


#ifdef XAOD_STANDALONE

// FIXME: xAOD has its own, incompatible, CLASS_DEF, but we don't want to use it
//        due to dependency issues.  Probably need to move things around
//        a bit to resolve this.
#define VIEWVECTOR_CLASS_DEF(NAME, CID)

#else

/**
 * @brief Use this macro to associate a CLID with a @c ViewVector class.
 */
#define VIEWVECTOR_CLASS_DEF(NAME, CID) \
  [[maybe_unused]] \
  static const bool clidinit_##CID = ClassID_traits<NAME>::init (CID, #NAME); \
  CLIDREGISTRY_ADDENTRY(CID, NAME)
#endif


#endif // not ATHCONTAINERS_VIEWVECTOR_H

