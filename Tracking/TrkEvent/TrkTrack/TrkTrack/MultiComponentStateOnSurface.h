/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*******************************************************************************
            MultiComponentStateOnSurface.h  -  description
            -----------------------------------------------
begin                : Monday 20th December 2004
author               : atkinson, amorley,anastopoulos
description          : This class is a multi component adaptation of the
                      class TrackStateOnSurface (from Moyse).
                      In that class the track state was represented
                      by a single 5 component track paramter
                      vector (a0, z0, phi0, theta0, q/p) and the associated
                      covariance matrix.
                      In its multi-component form the track
                      state on surface is represented by many track parameters
                      each with a covariance matrix and additionally a
                      weighting is attached to each component which reflects
                      the importance of that particular component in the
                      overall mixture of components which is used to describe
                      the track state at that surface.
*******************************************************************************/

#ifndef TrkMultiComponentStateOnSurface_H
#define TrkMultiComponentStateOnSurface_H

#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkParameters/ComponentParameters.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include <iostream>

class MsgStream;
class TrackCollectionCnv;
class TrackStateOnSurfaceCnv_p3;
namespace Trk {
class MaterialEffectsBase;
class MeasurementBase;

class MultiComponentStateOnSurface final : public TrackStateOnSurface
{

  friend class ::TrackCollectionCnv;
  friend class ::TrackStateOnSurfaceCnv_p3;

public:
  /** Default constructor for POOL. This should not be used! */
  MultiComponentStateOnSurface();

  /** Create a MultiComponentStateOnSurface Object. This has the same form as
     the singular version (Trk::TrackStateOnSurface) with the exception that the
     pointer to a single track paramters vector is now repleaced with a pointer to a
     multi-component state */
  MultiComponentStateOnSurface(
    const FitQualityOnSurface&,
    std::unique_ptr<MeasurementBase>,
    MultiComponentState&&,
    std::unique_ptr<MaterialEffectsBase> materialEffectsOnTrack = nullptr);

  /** Create a MultiComponentStateOnSurface Object with an explicit declaration
   * of the track parameters to be passed to the Trk::TrackStateOnSurface base
   * class */
  MultiComponentStateOnSurface(
    const FitQualityOnSurface&,
    std::unique_ptr<MeasurementBase>,
    std::unique_ptr<TrackParameters>,
    MultiComponentState&&,
    std::unique_ptr<MaterialEffectsBase> materialEffectsOnTrack = nullptr);

  /** Create TrackStateOnSurface with TrackStateOnSurfaceType. */
  MultiComponentStateOnSurface(
    const FitQualityOnSurface&,
    std::unique_ptr<MeasurementBase>,
    MultiComponentState&&,
    std::unique_ptr<MaterialEffectsBase>,
    const std ::bitset<NumberOfTrackStateOnSurfaceTypes>&);

  /** Create a MultiComponentStateOnSurface Object with an explicit declaration
   * of the track parameters to be passed to the base and also a
   * TrackStateOnSurfaceType */
  MultiComponentStateOnSurface(
    const FitQualityOnSurface&,
    std::unique_ptr<MeasurementBase>,
    std::unique_ptr<TrackParameters>,
    MultiComponentState&&,
    std::unique_ptr<MaterialEffectsBase>,
    const std ::bitset<NumberOfTrackStateOnSurfaceTypes>& types);

  /** Constructor without a FitQualityOnSurface. */
  MultiComponentStateOnSurface(std::unique_ptr<MeasurementBase>,
                               MultiComponentState);

  /** Copy constructor and assignment*/
  MultiComponentStateOnSurface(const MultiComponentStateOnSurface& other);
  MultiComponentStateOnSurface& operator=(
    const MultiComponentStateOnSurface& other);

  /** Move constructor and assignment*/
  MultiComponentStateOnSurface(MultiComponentStateOnSurface&& other) noexcept =
    default;
  MultiComponentStateOnSurface& operator=(
    MultiComponentStateOnSurface&& other) noexcept = default;

  /** Virtual destructor */
  virtual ~MultiComponentStateOnSurface() = default;

  /** Clone method for deep copy of MultiComponentStateOnSurface */
  virtual MultiComponentStateOnSurface* clone() const override final;

  /** This is Multi, since we MultiComponent */
  virtual TrackStateOnSurface::Variety variety() const override final;

  /** Method to return a pointer to the multi-component state  const overload*/
  const MultiComponentState& components() const;

  /** Method to return a pointer to the multi-component state non const
   * overload*/
  MultiComponentState& components();

private:
  MultiComponentState m_multiComponentState{};
};

/** Overload of << operator for MsgStream for debug output */
MsgStream&
operator<<(MsgStream&, const MultiComponentStateOnSurface&);

/** Overload of << operator for std::ostream for debug output */
std::ostream&
operator<<(std::ostream&, const MultiComponentStateOnSurface&);

} // end of Trk namespace

/// Trk::Track is constucted from  DataVector<const Trk::TrackStateOnSurface>.
/// Let the type system know this class inherits so we can have
/// DataVector<const Trk::MultiComponentStateOnSurface>
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(const Trk::MultiComponentStateOnSurface,
                const Trk::TrackStateOnSurface);
typedef DataVector<const Trk::MultiComponentStateOnSurface>
  MultiComponentStateOnSurfaceDV;

#include "TrkTrack/MultiComponentStateOnSurface.icc"
#endif
