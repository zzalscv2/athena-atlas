/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   GsfMeasurementUpdator.h
 * @date   Friday 25th February 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * @brief  Cde for performing updates on multi-component states for the
 * gaussian-sum filter.
 */

#ifndef TrkGsfMeasurementUpdator_H
#define TrkGsfMeasurementUpdator_H

#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
//
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/ComponentParameters.h"

namespace Trk {

namespace GsfMeasurementUpdator {

/** @brief Method for updating the multi-state with a new measurement and
 * calculate the fit qaulity at the same time
 * updatingSign = 1 (default) means add, -1 means remove
 * */
MultiComponentState
update(Trk::MultiComponentState&&,
       const Trk::MeasurementBase&,
       FitQualityOnSurface& fitQoS,
       const int updatingSign  = 1);

/** @brief Method for determining the chi2 of the multi-component state and the
 * number of degrees of freedom */
FitQualityOnSurface
fitQuality(const MultiComponentState&, const MeasurementBase&);

} // end of GsfMeasurementUpdator namespace
} // end of namespace Trk

#endif
