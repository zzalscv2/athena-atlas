//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "DataRepository.h"

#include <string>

namespace GlobalSim {

 
  template<typename T>
  std::size_t get_tuple_size(T) {
    return std::tuple_size<T>();
  };


  /*
   * Provide functions for unrolling the std::tuple class used
   * to hold vectors of GlobalData<T> objects.
   *
   * Because std::tuple is a heterogeneous containers (is able to contain
   * more than one type), ut cannot be simply interated over at run time.
   *
   * Instead, functions executed at compile time are used. These take
   * some kind if function (e.g. function object or lambda) to perform
   * actions on the tuple contents.
   */
  template <std::size_t Index, typename Tuple, typename Functor>
  auto tuple_at(const Tuple& tpl, const Functor& func) -> void {
    const auto& v = std::get<Index>(tpl);
    func(v);    
  }

  template<typename Tuple, typename Functor, std::size_t Index=0>
  auto tuple_for_each(const Tuple& tpl, const Functor& f) -> void {
    constexpr auto tuple_size = std::tuple_size_v<Tuple>;
    if constexpr(Index < tuple_size) {
      tuple_at<Index>(tpl, f);
      tuple_for_each<Tuple, Functor, Index+1> (tpl, f);
    }
  }

  // used for printing tuple contents of the tuple to an ostream object
  // this is a specific case of accept()
  std::ostream&  DataRepository::print(std::ostream& os) const{

    os << "DataRepository Dump\n";
    
    auto printVec = [&os](const auto& v) {
     os << "v size[" << v.size() << "]\n";

      for (const auto& e : v) {
	os << e.to_string() << ' ';
      }
      os << '\n';
    
    };

      
    tuple_for_each(m_vecs, printVec);
    return os;
  }

  // used to pass a function object to tuple in order to act on its contents.
  void  DataRepository::accept(IDataRepositoryVisitor& visitor) const{
    
    auto processVisitor = [&visitor](const auto& v) {
      visitor.process(v);
    };
    
    
    tuple_for_each(m_vecs, processVisitor);
  }

  std::ostream& operator << (std::ostream& os, const DataRepository& ds) {
    return ds.print(os);
  }
}
