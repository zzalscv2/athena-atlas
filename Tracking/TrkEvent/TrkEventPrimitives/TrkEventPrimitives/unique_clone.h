/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

namespace Trk{
 //Produces a unique_ptr clone from a bare pointer to object with a 'clone' method
 template<typename T>
  std::unique_ptr<T> unique_clone(const T * v) {
    if (v != nullptr) {
      return std::unique_ptr<T>(v->clone());
    } else {
      return nullptr;
    }
  }
  
  //Produces a unique_ptr clone from a unique_ptr to object with a 'clone' method
  template<typename T>
  std::unique_ptr<T> unique_clone(const std::unique_ptr<T> & v) {
    if (v != nullptr) {
      return std::unique_ptr<T>(v->clone());
    } else {
      return nullptr;
    }
  }
}