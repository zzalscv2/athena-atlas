///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonDefs.h 
// Header file for common definitions
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONKERNEL_PERFMONDEFS_H 
#define PERFMONKERNEL_PERFMONDEFS_H 1

/**
 * Common definitions for the @c PerfMon packages
 * @class PerfMon::State
 * factorizes the different states (and their name)
 * of the Gaudi finite state machine. At least those PerfMon is interested in.
 */

// STL includes
#include <string>
#include <array>

namespace PerfMon {

  struct State {
    enum Type {
      ini = 0,
      //run,
      evt,
      fin,
      cbk,
      io,
      dso,

      // keep last !
      Size
    };
  };
  typedef std::array<std::string, State::Size> Steps_t;
  static const Steps_t Steps = { {
    "ini", 
    //"run", 
    "evt", 
    "fin",
    "cbk",
    "io",
    "dso"
    }
  };

}


#endif // PERFMONKERNEL_PERFMONDEFS_H 
