/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

namespace Trk{
  
  /*
   consistentSurface function takes a variable number of pointer arguments and calls associatedSurface
   on each one, and reports whether the result is the same (object equality) for all arguments.
   caveats: 
   1) If an argument is a nullptr, it is ignored in the comparison
   2) If only one argument is passed, the result is true
  */

  template <typename U>
  bool
  consistentSurfaces(U ){
    return true;
  }

  template <typename U, typename ...T>
  bool
  consistentSurfaces( U a, T...b){
    if (a==nullptr) return (consistentSurfaces(b...));
    return (((b!=nullptr)?(a->associatedSurface() == b->associatedSurface()):true) and ...);
  }

}