//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//


#ifndef CALORECGPU_BASICGPUCLUSTERINFOCALCULATOR_H
#define CALORECGPU_BASICGPUCLUSTERINFOCALCULATOR_H

#include "CxxUtils/checker_macros.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

#include "CLHEP/Units/SystemOfUnits.h"

/**
 * @class BasicGPUClusterInfoCalculator
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 11 August 2022
 * @brief Standard tool to calculate cluster info (energy, transverse energy, pseudo-rapidity and azimuthal angle)
 *        and apply E/ET cuts on clusters if desired.
 */


class BasicGPUClusterInfoCalculator:
  public AthAlgTool, virtual public CaloClusterGPUProcessor, public CaloGPUTimed
{
 public:

  BasicGPUClusterInfoCalculator(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicGPUClusterInfoCalculator();

  virtual size_t size_of_temporaries() const
  {
    return sizeof(ClusterInfoCalculatorTemporaries);
  };

 private:

  /**
  * @brief if set to @p true cluster cuts are on \f$|E|_\perp\f$, if @p false on \f$E_\perp\f$. Default is @p true.
  *
  */
  Gaudi::Property<bool> m_cutClustersInAbsE {this, "ClusterCutsInAbsEt", true, "Do cluster cuts in Abs Et instead of Et"};

  /**
   * @brief \f$E_\perp\f$ cut on the clusters.
   *
   * The clusters have to pass this cut (which is on \f$E_\perp\f$
   * or \f$|E|_\perp\f$ of the cluster depending on the above switch)
   * in order to be inserted into the CaloClusterContainer.  */

  Gaudi::Property<float> m_clusterETThreshold {this, "ClusterEtorAbsEtCut", 0.*CLHEP::MeV, "Cluster E_t or Abs E_t cut"};

};

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_H