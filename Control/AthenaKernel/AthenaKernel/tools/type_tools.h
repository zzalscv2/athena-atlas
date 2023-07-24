/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_TOOLS_TYPE_TOOLS_H
#define ATHENAKERNEL_TOOLS_TYPE_TOOLS_H
#include <type_traits>

namespace type_tools {
  ///assign a type to an integer value
  template <int v>
  struct Int2Type { 
    enum { value = v };
  };

  typedef Int2Type<1> true_tag; 
  typedef Int2Type<0> false_tag;


  /**
   * @class Parameter
   * @brief an algorithm to provide an efficient way to pass a parameter:
   * by value for scalars (arithmetic types and pointers),
   * by reference for compound objects.Derived from Loki library
   * @author Paolo Calafiura
   *
   */

  template <typename T>
  struct Parameter {
  private:
    typedef typename std::add_lvalue_reference<T>::type TRef;
    typedef const TRef const_TRef;
    static const bool  s_isScalar = std::is_scalar<T>::value;
  public:
    typedef typename std::conditional<s_isScalar,T,TRef>::type ref_type;
    typedef typename std::conditional<s_isScalar,T,const T&>::type const_type;
    typedef typename std::conditional<s_isScalar,T,T*>::type ptr_type;
    typedef const_type type;
  };


  /**
   * @class Copy
   * @brief an algorithm to define a suitable "copy type": 
   * by default use copy-by-value
   * @author Paolo Calafiura
   *
   */
  template <typename T>
  struct Copy {
    typedef T type;
    typedef const T& const_reference;
    typedef const T* const_pointer;
  };

  /// when T is pointer we "copy" into a const T* 
  template <typename T>
  struct Copy<T*> {
    typedef const T* type;
    typedef const T* const_reference;
    typedef const T* const* const_pointer;
  }; 
}
#endif

