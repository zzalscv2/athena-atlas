/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _ERRORSCALING_CAST_H_
#define _ERRORSCALING_CAST_H_

#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace Trk {

template <typename T_res, typename T_src>
const T_res* ErrorScalingCast(const T_src* src) {

  if (T_res::s_type != src->type()) {
    std::stringstream message;
    message << "Invalid RIO_OnTrackErrorScaling type. Expected "
            << T_res::s_type << " But Received " << src->type();

    throw std::runtime_error(message.str());
  }
  return static_cast<const T_res*>(src);
}

}  // namespace Trk

#endif

