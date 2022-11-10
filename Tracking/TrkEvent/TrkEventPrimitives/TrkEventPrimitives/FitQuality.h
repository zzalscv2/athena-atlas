/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// FitQuality.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEVENTPRIMITIVES_TRKFITQUALITY_H
#define TRKEVENTPRIMITIVES_TRKFITQUALITY_H

#include <cmath> //for std::floor in the included .icc file
#include <iosfwd>
#include <memory>
class MsgStream;

namespace Trk {
/**
    @class FitQualityImpl
    @brief simple/trivial Implemenation class 
    for fit the fit  quality. It holds the same
    payload as FitQuality_p1.

    It is used for the implementation of FitQualityOnSurface.
    And for the implementation of the extendable
    FitQuality that adds virtual methods to it

    @author Christos Anastopoulos.
*/

class FitQualityImpl
{
public:
  /** default ctor for POOL*/
  FitQualityImpl() = default;
  FitQualityImpl(const FitQualityImpl&) = default;
  FitQualityImpl(FitQualityImpl&&) = default;
  FitQualityImpl& operator=(const FitQualityImpl&) = default;
  FitQualityImpl& operator=(FitQualityImpl&&) = default;

  /** Constructor with @f$ \chi^2 @f$ and @f$ n_{dof} @f$ */
  FitQualityImpl(double chiSquared, int numberDoF)
    : m_chiSquared(chiSquared)
    , m_numberDoF(numberDoF)
  {
  }

  /**Constructor hadling double type of NDF*/
  FitQualityImpl(double chiSquared, double numberDoF)
    : m_chiSquared(chiSquared)
    , m_numberDoF(numberDoF)
  {
  }

  /** returns the @f$ \chi^2 @f$ of the overall track fit*/
  double chiSquared() const { return m_chiSquared; }

  /** returns the number of degrees of freedom of the overall track or
  vertex fit as integer */
  int numberDoF() const
  {

    return static_cast<int>(std::floor(m_numberDoF + 0.5));
  }

  /** returns the number of degrees of freedom of the overall track or
  vertex fit as double */
  double doubleNumberDoF() const { return m_numberDoF; }

  /** set the @f$ \chi^2 @f$*/
  void setChiSquared(double chiSquared) { m_chiSquared = chiSquared; }

  /** set the number of degrees of freedom*/
  void setNumberDoF(double numberDoF) { m_numberDoF = numberDoF; }

  explicit operator bool() const
  {
    //we need to have set something
    return (m_chiSquared != 0 || m_numberDoF != 0);
  }

protected:
  //Protected
  //We do not want to create/destroy
  //objects via this class
  ~FitQualityImpl() = default;
private:
  double m_chiSquared{};
  double m_numberDoF{};

}; // end of class definitions

/** @class FitQuality
    @brief Class to represent and store fit qualities from track
           reconstruction in terms of @f$\chi^2@f$ and number of
           degrees of freedom.

        Its main use is to describe the trajectory fit quality at
        a measurement and of the overall track. However, it can
        be extended as necessary.
        @author Edward.Moyse@cern.ch, Andreas.Salzburger@cern.ch
*/
class FitQuality : public FitQualityImpl
{
public:
  using FitQualityImpl::FitQualityImpl;
  using FitQualityImpl::operator=;
  using FitQualityImpl::chiSquared;
  using FitQualityImpl::doubleNumberDoF;
  using FitQualityImpl::numberDoF;
  using FitQualityImpl::setChiSquared;
  using FitQualityImpl::setNumberDoF;
 
  // Needed for T/P since we used to have only
  // FitQuality, This copies the  payload
  // we set to the persistent type
  // i.e FitQuality_p1.
  FitQuality(const FitQualityImpl& fq)
    : FitQualityImpl(fq.chiSquared(), fq.doubleNumberDoF())
  {
  }

  virtual ~FitQuality() = default;

  /**Virtual constructor */
  virtual FitQuality* clone() const;

  /**NVI uniqueClone */
  std::unique_ptr<FitQuality> uniqueClone() const;

}; // end of class definitions

/**Overload of << operator for MsgStream for debug output*/
MsgStream&
operator<<(MsgStream& sl, const FitQualityImpl& fq);
/**Overload of << operator for std::ostream for debug output*/
std::ostream&
operator<<(std::ostream& sl, const FitQualityImpl& fq);

} // end ns
#include "TrkEventPrimitives/FitQuality.icc"
#endif // TRKEVENTPRIMITIVES_FITQUALITY_H
