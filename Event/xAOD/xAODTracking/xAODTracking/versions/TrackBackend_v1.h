/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKBACKEND_V1_H
#define XAODTRACKING_VERSIONS_TRACKBACKEND_V1_H
#include <cstdint>
#include <vector>
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"


namespace xAOD
{
  /**
   * @brief Track Backend for Acts MultiTrajectory
   **/

  class TrackBackend_v1 : public SG::AuxElement
  {
  private:
    static const SG::AuxElement::Accessor<std::vector<double> > s_paramsAcc, s_covParamsAcc;
  public:
    TrackBackend_v1() = default;
    /**
     * access track backend vector of const element
     **/

    template <std::size_t measdim = 6>
    Eigen::Map<const Eigen::Matrix<double, measdim, 1>> paramsEigen() const
    {
      return Eigen::Map<const Eigen::Matrix<double, measdim, 1>>{s_paramsAcc(*this).data()};
    }
    /**
     * access parameters of non const element
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<Eigen::Matrix<double, measdim, 1>> paramsEigen()
    {
      return Eigen::Map<Eigen::Matrix<double, measdim, 1>>{s_paramsAcc(*this).data()};
    }

    /**
     * access track parameters as plain vector
     **/
    const std::vector<double> &params() const;
    /**
     * access set parameters from plain vector
     **/
    void setParams(const std::vector<double> &m);

    /**
     * access track covariance matrix (flattened, rows layout) of const element
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<const Eigen::Matrix<double, measdim, measdim>> covParamsEigen() const
    {
      return Eigen::Map<const Eigen::Matrix<double, measdim, measdim>>{s_covParamsAcc(*this).data()};
    }

    /**
     * access track covariance matrix (flattened, rows layout)
     **/
    template <std::size_t measdim = 6>
    Eigen::Map<Eigen::Matrix<double, measdim, measdim>> covParamsEigen()
    {
      return Eigen::Map<Eigen::Matrix<double, measdim, measdim>>{s_covParamsAcc(*this).data()};
    }

    /**
     * access track covariance as plain vector
     **/
    const std::vector<double> &covParams() const;
    /**
     * access set covariance from plain vector
     **/
    void setCovParams(const std::vector<double> &m);

    /**
     * access nMeasurements
     **/
    const std::vector<unsigned int> &nMeasurements() const;
    /**
     * access set nMeasurements
     **/
    void setnMeasurements(const std::vector<unsigned int> &m);

    /**
     * access nHoles
     **/
    const std::vector<unsigned int> &nHoles() const;
    /**
     * access set nHoles
     **/
    void setnHoles(const std::vector<unsigned int> &m);

    /**
     * access chi2
     **/
    const std::vector<float> &chi2() const;
    /**
     * access set chi2
     **/
    void setChi2(const std::vector<float> &m);

    /**
     * access ndf
     **/
    const std::vector<unsigned int> &ndf() const;
    /**
     * access set ndf
     **/
    void setNdf(const std::vector<unsigned int> &m);

    /**
     * access nOutliers
     **/
    const std::vector<unsigned int> &nOutliers() const;
    /**
     * access set nOutliers
     **/
    void setnOutliers(const std::vector<unsigned int> &m);

    /**
     * access nSharedHits
     **/
    const std::vector<unsigned int> &nSharedHits() const;
    /**
     * access set nSharedHits
     **/
    void setnSharedHits(const std::vector<unsigned int> &m);

    void resize(size_t sz = 6);

    /**
     * @brief retrieve the size of the internal vectors for the data storage
     */
    size_t size() const;
  };
}
#endif
