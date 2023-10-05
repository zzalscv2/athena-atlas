/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CurvilinearParametersT.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKPARAMETERSBASE_CURVILINEARPARAMETERS_T_H
#define TRKPARAMETERSBASE_CURVILINEARPARAMETERS_T_H

#include "TrkParametersBase/ParametersBase.h"

#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/CurvilinearUVT.h"
#include "TrkEventPrimitives/SurfaceTypes.h"
#include <memory>
class MsgStream;

namespace Trk {
/**
 @class CurvilinearParametersT

 The specialised class for charged and neutral track parameters in the
 curvilinear frame.

 The method parameters() returns a vector of the track parameters, defined as:
 \f$\left(\begin{array}{c}
 locX\\locY\\\phi\\\theta\\q/p
 \end{array}\right)\f$

 The following is true per definition:
  - the local position is per definition (0.,0.)
  - the surface is per definition a plane surface normal to the track (UVT
 frame)

 @tparam DIM number of track parameters (usually 5)
 @tparam T   charge of track (either <tt>Trk::Charged</tt> or
 <tt>Trk::Neutral</tt>)

 @author andreas.salzburger@cern.ch
 @author Christos Anastopoulos (Athena MT modifications)
*/

template<int DIM, class T, class S>
class CurvilinearParametersT final : public ParametersBase<DIM, T>
{
public:
  static_assert(S::staticType == SurfaceType::Plane,
                "The surface type must be Plane");

  /** default constructor only for POOL */
  CurvilinearParametersT() = default;

  /** Create CurvilinearParametersT from DIM+2 parameters
      - these are: global position, momentum, charge, extension */
  CurvilinearParametersT(
    const AmgVector(DIM + 2) & parameters,
    std::optional<AmgSymMatrix(DIM)> covariance = std::nullopt,
    unsigned int cIdenfier = 0);

  /**Create CurvilinearParametersT from mixed parameters: pos, local
   * parameters*/
  CurvilinearParametersT(
    const Amg::Vector3D& pos,
    double phi,
    double theta,
    double qOverP,
    std::optional<AmgSymMatrix(DIM)> covariance = std::nullopt,
    unsigned int cIdenfier = 0);

  /** Create CurvilinearParametersT from global parameters.
     -- it will throw a GaudiException if the position is not on surface  */
  CurvilinearParametersT(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& mom,
    double charge,
    std::optional<AmgSymMatrix(DIM)> covariance = std::nullopt,
    unsigned int cIdenfier = 0);

  /** Copy Constructor */
  CurvilinearParametersT(const CurvilinearParametersT<DIM, T, S>&) = default;

  /** Move Constructor */
  CurvilinearParametersT(CurvilinearParametersT<DIM, T, S>&&) noexcept = default;

  /** Assignment operator*/
  CurvilinearParametersT<DIM, T, S>& operator=(
    const CurvilinearParametersT<DIM, T, S>&) = default;

  /** Move assignment operator*/
  CurvilinearParametersT<DIM, T, S>& operator=(
    CurvilinearParametersT<DIM, T, S>&&) noexcept = default;

  /** Destructor */
  virtual ~CurvilinearParametersT() = default;

  /** the curvilinear parameters identifier */
  unsigned int cIdentifier() const;

  void setcIdentifier(unsigned int cIdentifier);

  /** Access method for the position */
  virtual Amg::Vector3D position() const override final;

  /** Access method for the momentum */
  virtual Amg::Vector3D momentum() const override final;

  /** Test to see if there's a surface there. */
  virtual bool hasSurface() const override final;

  /** Access to the Surface method */
  virtual const S& associatedSurface() const override final;

  /** equality operator */
  virtual bool operator==(
    const ParametersBase<DIM, T>& rhs) const override final;

  /** Virtual clone */
  virtual CurvilinearParametersT<DIM, T, S>* clone() const override final;


  /** Return the ParametersType enum */
  virtual ParametersType type() const override final;

  /** Return the Surface Type enum*/
  virtual SurfaceType surfaceType() const override final;

  /** Return the measurementFrame of the parameters */
  virtual Amg::RotationMatrix3D measurementFrame() const override final;

  /**Dumps relevant information about the track parameters into the ostream.*/
  virtual MsgStream& dump(MsgStream& out) const override final;
  virtual std::ostream& dump(std::ostream& out) const override final;

private:
  /* Helper to factor in update of parameters*/
  virtual void updateParametersHelper(const AmgVector(DIM) &) override final;

  /** return the curvilinear frame */
  CurvilinearUVT curvilinearFrame() const;

protected:
  /*
   * Add dependent names into scope
   */
  using ParametersBase<DIM, T>::m_parameters;
  using ParametersBase<DIM, T>::m_covariance;
  using ParametersBase<DIM, T>::m_chargeDef;
  Amg::Vector3D m_position; //!< point on track
  Amg::Vector3D m_momentum; //!< momentum at this point on track
  S m_surface;              //!< surface template
  /** the curvilinear parameters identifier */
  unsigned int m_cIdentifier = 0;
  /*
   * friends needed for Persistency
   */
  template<typename pars>
  friend class ::TrackParametersCovarianceCnv;
  friend class ::TrackParametersCnv_p2;
  friend class ::MeasuredPerigeeCnv_p1;

  /** DESIGN TO BE REVISITED */
  friend class MaterialEffectsEngine;
};
} // end of namespace Trk

#include "TrkParametersBase/CurvilinearParametersT.icc"
#endif
