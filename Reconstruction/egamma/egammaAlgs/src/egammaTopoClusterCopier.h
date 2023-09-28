/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_EGAMMATOPOCLUSTERCOPIER_H
#define EGAMMAALGS_EGAMMATOPOCLUSTERCOPIER_H

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthContainers/ConstDataVector.h"

#include "GaudiKernel/SystemOfUnits.h"

#include <Gaudi/Accumulators.h>

/**
   @class egammaTopoClusterCopier
   @brief Select topo-clusters to be used in egamma reconstruction

   This algorithm is the first step in egamma reconstruction. It selects
   topoclusters and decorate them with the EM fraction. The input and output
   containers are collections of xAOD::CaloCluster.

   - Input container: input clusters
   - Output container: view of decorated selected clusters
   - Output container: shallow copy of decorated clusters of the input container

   The algorithm computes a custom version of the EM fraction defined as the sum of the
   energy of the presampler, three accordion layers and the energy in the tile-gap divided
   by the total energy of the cluster. The computation is done only for cluster passing
   the preselection (|eta| and minimum EM energy). Otherwise a value of 0 is used.
   The value is used to decorate the clusters with "EMFraction" decoration.

   The selection is based on:
   - maximum |eta| of selected clusters
   - minimum EM energy of selected clusters
   - mimimum EM fraction of selected clusters
*/
class egammaTopoClusterCopier : public AthReentrantAlgorithm {

public:

  egammaTopoClusterCopier(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

private:

  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inputTopoCollection {
    this,
    "InputTopoCollection",
    "CaloTopoClusters",
    "input topocluster collection"
  };

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputTopoCollectionShallow {
    this,
    "TopoShallow_doNotConfig",
    "",
    "Shallow copy of input collection that allows properties to be modified"
  };

  SG::WriteHandleKey<ConstDataVector<xAOD::CaloClusterContainer>> m_outputTopoCollection {
    this,
    "OutputTopoCollection",
    "egammaTopoCluster",
    "View container of selected topoclusters"
  };

  SG::WriteHandleKey<ConstDataVector<xAOD::CaloClusterContainer>> m_outputFwdTopoCollection {
    this,
    "OutputFwdTopoCollection",
    "",
    "View container of selected fwd topoclusters"
  };

  Gaudi::Property<float> m_etaCut {
    this,
    "EtaCut",
    2.6,
    "maximum |eta| of selected clusters"
  };

  Gaudi::Property<double> m_fwdEtaCut {
    this,
    "fwdEtaCut",
    2.5,
    "minimum |eta| of selected fwd clusters"
  };

  Gaudi::Property<float> m_ECut {
    this,
    "ECut",
    700,
    "minimum EM energy of selected clusters"
  };

  Gaudi::Property<double> m_fwdETCut {
    this,
    "fwdETCut",
    5. * Gaudi::Units::GeV,
    "Fwd ET cut"
  };

  Gaudi::Property<float> m_EMFracCut {
    this,
    "EMFracCut",
    0.5,
    "mimimum EM fraction of selected clusters"
  };

  /** @brief Private member flag to do the track matching. */
  Gaudi::Property<bool> m_hasITk {
    this,
    "hasITk",
    false,
    "Boolean to do track matching"
  };

  /** @brief Private member flag to copy forward clusters. */
  bool m_doForwardClusters = false;

  mutable Gaudi::Accumulators::Counter<> m_AllClusters {};
  mutable Gaudi::Accumulators::Counter<> m_PassPreSelection {};
  mutable Gaudi::Accumulators::Counter<> m_PassSelection {};
  mutable Gaudi::Accumulators::Counter<> m_CentralPassPreSelection {};
  mutable Gaudi::Accumulators::Counter<> m_CentralPassSelection {};
  mutable Gaudi::Accumulators::Counter<> m_FwdPassPreSelection {};
  mutable Gaudi::Accumulators::Counter<> m_FwdPassSelection {};
  mutable Gaudi::Accumulators::Counter<> m_SharedPassPreSelection {};
  mutable Gaudi::Accumulators::Counter<> m_SharedPassSelection {};
};

#endif // EGAMMATOOLS_EMCLUSTERTOOL_H

