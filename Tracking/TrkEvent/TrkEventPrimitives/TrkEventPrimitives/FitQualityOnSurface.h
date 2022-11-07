/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// FitQualityOnSurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEVENTPRIMITIVES_FITQUALITYONSURFACE_H
#define TRKEVENTPRIMITIVES_FITQUALITYONSURFACE_H

#include <iostream>
class MsgStream;

#include "TrkEventPrimitives/FitQuality.h"

namespace Trk {
class FitQualityOnSurface final : public FitQualityImpl
{
public:
  /** default ctor for POOL*/
  using FitQualityImpl::FitQualityImpl;
  using FitQualityImpl::operator=;
  using FitQualityImpl::chiSquared;
  using FitQualityImpl::doubleNumberDoF;
  using FitQualityImpl::numberDoF;
  using FitQualityImpl::setChiSquared;
  using FitQualityImpl::setNumberDoF;
  using FitQualityImpl::operator bool;
  ~FitQualityOnSurface() = default;

  // Needed for T/P since we used to have only
  // FitQuality, this copies the  payload
  // we to the persistent type
  // i.e FitQuality_p1.
  FitQualityOnSurface(const FitQualityImpl& fq)
    : FitQualityImpl(fq.chiSquared(), fq.doubleNumberDoF())
  {
  }
};

}
#endif // TRKEVENTPRIMITIVES_FITQUALITYONSURFACE_H
