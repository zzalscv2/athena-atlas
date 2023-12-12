//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_IGLOBALALG_H
#define GLOBSIM_IGLOBALALG_H

#include <ostream>
/*
 * This file provides a PABC IGS_Alg for GlobalSim algs.
 * These algs are contaners for TCS::Algorithm objects
 * which handle interactions with the DataRepository
 *
 */


namespace TCS {
  class TopoInputEvent;
}

namespace GlobalSim {

  class DataRepository;
  
  class IGlobalAlg{
  public:
    virtual ~IGlobalAlg() {};

    // TCS::TopoInputEvent is needed by input Algorithms only.
    // This member function is not const because
    // the process() member function 
    // of the contained TCS::ConfiguredAlg is not const.
    virtual void run(DataRepository&,
		     const TCS::TopoInputEvent& event) = 0;
    virtual std::ostream& print(std::ostream&) const = 0;
  };
}

#endif
