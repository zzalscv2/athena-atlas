//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_DATAREPOSITORY_H
#define GLOBSIM_DATAREPOSITORY_H

#include "GlobalSimDefs.h"
#include "GlobalData.h"
#include "IDataRepositoryVisitor.h"

#include <vector>
#include <tuple>
#include <memory>
#include <ostream>
#include <algorithm>

/*
 * DataRepository is class for the type safe strorage of data
 * written out and read in by GlobalSim Algs.
 *
 * The repository holds its data in a std::tuple of vectors of
 * parameterised types GlobalData<T>, which contain the underlying data type
 * and which provide other underlying functionality.
 * 
 * By supplying an accept(IDataRepositoryVisitor& v), DataReposository allows
 * for flexible and type safe examination of the data for e.g. debugging
 * and monitoring purposes.
 */
 namespace GlobalSim {
  
   class DataRepository {
   public:
     
     ~DataRepository(){};
     
     friend std::ostream& operator << (std::ostream&, const DataRepository&);
     
     // allow lvals and r val type for data
     template<typename T>
     void write(const T&& data)
     {
       using Tnr =  typename std::remove_reference<T&>::type;

       std::get<std::vector<Tnr>>(m_vecs).push_back(std::move(data));

     }
     
     
     
     template<typename T>
     const GlobalData<T>& read(std::size_t sn)
     {
       const auto& vec = std::get<std::vector<GlobalData<T>>>(m_vecs);
       auto iter = std::find_if(vec.cbegin(), vec.cend(), [&sn] (const auto& gd){
	 return gd.sn() == sn;});
      if (iter == vec.cend()) {
	throw std::runtime_error ("DataRepository: item  with sn " +
				  std::to_string(sn) + " not found");
      }
      return *iter;
     }

         
     template<typename T>
     const std::vector<GlobalData<T>>& readVector()
     {
       return std::get<std::vector<GlobalData<T>>>(m_vecs);
     }

     void accept(IDataRepositoryVisitor& v) const;


     std::ostream&  print(std::ostream& os) const;
     
   private:
 
    std::tuple<
     std::vector<GSInputTOBArray>,
     std::vector<GSTOBArray>,
     std::vector<GSTOBArrayPtrVec>,
     std::vector<GSCount>,
     std::vector<GSDecision>
    > m_vecs;
    
  };

}


#endif
