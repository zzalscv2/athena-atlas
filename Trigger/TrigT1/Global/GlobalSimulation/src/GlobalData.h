//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_BOOLDATA_H
#define GLOBSIM_BOOLDATA_H

/*
 * Global Data stores an data instance T,
 * along with its graph node serial number.
*/

#include <string>

namespace GlobalSim {

  template<typename T>
  class GlobalData {
  public:
    GlobalData (std::size_t sn, T data);
    
    ~GlobalData(){};
    
    const T& data() const;
    
    virtual size_t sn () const;
    
    virtual std::string to_string() const;
  private:
    std::size_t m_sn;
    T m_data;

  };

}

#endif
