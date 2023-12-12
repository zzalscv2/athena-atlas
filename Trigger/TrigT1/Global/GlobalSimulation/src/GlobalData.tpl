//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GlobalData.h"

#include <sstream>

namespace GlobalSim {

  template<typename T>
  GlobalData<T>::GlobalData(std::size_t sn, T data) :
    m_sn{sn}, m_data{data} {}
  
  template<typename T>
  std::size_t GlobalData<T>::sn() const {return m_sn;}

  template<typename T>
  const T& GlobalData<T>::data() const {return m_data;}


  template<typename T>
  std::string GlobalData<T>::to_string() const {
    std::stringstream ss;
    ss << "GlobalData sn: " << m_sn;
    return ss.str();
  }
}
