/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// GXFTrackState.h
//   Class grouping all fitter relevant info on a surface-by-surface
//   basis
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_GXFTRACKSTATE_H
#define TRK_GXFTRACKSTATE_H

#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include <memory>
#include <bitset>


namespace Trk {

  class MeasurementBase;
  class GXFMaterialEffects;
  class TransportJacobian;

  class GXFTrackState {
  public:
    GXFTrackState() = delete;
    GXFTrackState(const GXFTrackState &);
    GXFTrackState(GXFTrackState &&) = default;
    GXFTrackState & operator=(const GXFTrackState &) = delete;
    GXFTrackState & operator=(GXFTrackState &&) = default;
    ~GXFTrackState() = default;

    GXFTrackState(std::unique_ptr<const MeasurementBase>, std::unique_ptr<const TrackParameters>);
    GXFTrackState(std::unique_ptr<const TrackParameters>, TrackStateOnSurface::TrackStateOnSurfaceType);
    GXFTrackState(std::unique_ptr<GXFMaterialEffects>, std::unique_ptr<const TrackParameters>);
    GXFTrackState & operator=(GXFTrackState &) = delete;
    bool isSane() const;
    

    void setMeasurement(std::unique_ptr<const MeasurementBase>);
    const MeasurementBase *measurement(void);

    void setTrackParameters(std::unique_ptr<const TrackParameters>);
    const TrackParameters *trackParameters(void) const;
    
    GXFMaterialEffects *materialEffects();
    const Surface & associatedSurface() const;
    void setJacobian(TransportJacobian &);
    Eigen::Matrix<double, 5, 5> & jacobian();
    Amg::MatrixX & derivatives();
    void setDerivatives(Amg::MatrixX &);

    void setTrackCovariance(AmgSymMatrix(5) *);
    void resetTrackCovariance(); //!<reset covariance to nullptr
    AmgSymMatrix(5) & trackCovariance(void);
    bool hasTrackCovariance(void) const;
    void zeroTrackCovariance(void);

    void setFitQuality(FitQualityOnSurface);
    const FitQualityOnSurface fitQuality(void);

    TrackState::MeasurementType measurementType();
    void setMeasurementType(TrackState::MeasurementType);

    double sinStereo() const;
    void setSinStereo(double);

    double *measurementErrors();
    void setMeasurementErrors(const double *);

    int numberOfMeasuredParameters();

    bool isRecalibrated();
    void setRecalibrated(bool);

    Amg::Vector3D position();
    void setPosition(Amg::Vector3D &);

    bool measuresPhi() const;
    void setMeasuresPhi(bool);

    /**
     * @brief Set a specific type, wiping all others.
     *
     * When called, this method will set the bit for a specific type to true or
     * false. It will also set all other type bits to false.
     *
     * @param[in] type The track state type bit to set.
     * @param[in] value The boolean value for the given bit (default true).
     */
    void resetStateType(TrackStateOnSurface::TrackStateOnSurfaceType type, bool value=true);

    /**
     * @brief Set a specific type bit.
     *
     * This method sets a specific bit in the type bitfield to the specified
     * value, and does not touch any of the other bits.
     *
     * @param[in] type The track state type bit to set.
     * @param[in] value The boolean value for the given bit (default true).
     */
    void setStateType(TrackStateOnSurface::TrackStateOnSurfaceType type, bool value=true);

    /**
     * @brief Retrieve the value of a specific type bit.
     *
     * @param[in] type The track state type bit to set.
     * @return A boolean value indicating whether or not the type bit is set.
     */
    bool getStateType(TrackStateOnSurface::TrackStateOnSurfaceType type) const;

    std::optional<std::vector<std::unique_ptr<TrackParameters>>> & getHoles(void);
    void setHoles(std::vector<std::unique_ptr<TrackParameters>> &&);
    
    std::unique_ptr<const TrackStateOnSurface>
    trackStateOnSurface() const;

  private:
    std::unique_ptr<const MeasurementBase> m_measurement;       //!< The measurement defining the track state
    std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> m_tsType;      //!< type of track state, eg Fittable, Outlier, Scatterer, Brem, Hole
    std::unique_ptr<const TrackParameters> m_trackpar;  //!< track parameters
    std::unique_ptr<GXFMaterialEffects> m_materialEffects;      //!< Material effects on track (ie scatterer, brem)
    Eigen::Matrix<double, 5, 5> m_jacobian;    //!< Transport jacobian wrt previous state
    Amg::MatrixX m_derivs;  //!< Derivatives of local parameters wrt fit parameters

    AmgSymMatrix(5) m_covariancematrix;     //!< Covariance matrix of track parameters at this surface
    bool m_covariance_set;

    FitQualityOnSurface m_fitqual;
    double m_measerror[5]{};      //!< Measurement errors (corrected for stereo angle)
    double m_sinstereo;         //!< stereo angle
    TrackState::MeasurementType m_mType;      //!< Measurement type, eg pixel, SCT, ...
    bool m_recalib;             //!< Has this measurement already been recalibrated?
    bool m_measphi;
    Amg::Vector3D m_globpos;
    std::optional<std::vector<std::unique_ptr<TrackParameters>>> m_holes;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  inline Eigen::Matrix<double, 5, 5> & GXFTrackState::jacobian() {
    return m_jacobian;
  } 
  
  inline Amg::MatrixX & GXFTrackState::derivatives() {
    return m_derivs;
  }

  inline AmgSymMatrix(5) & GXFTrackState::trackCovariance(void) {
    return m_covariancematrix;
  }

  inline const TrackParameters *GXFTrackState::trackParameters(void) const {
    return m_trackpar.get();
  }

  inline GXFMaterialEffects *GXFTrackState::materialEffects() {
    return m_materialEffects.get();
  }

  inline TrackState::MeasurementType GXFTrackState::measurementType() {
    return m_mType;
  }

  inline void GXFTrackState::setMeasurementType(TrackState::MeasurementType mt) {
    m_mType = mt;
  }

  inline bool GXFTrackState::isRecalibrated() {
    return m_recalib;
  }
}
#endif
