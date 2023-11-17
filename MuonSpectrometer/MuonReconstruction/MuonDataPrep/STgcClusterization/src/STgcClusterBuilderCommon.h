/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STgcClusterBuilderCommon_h
#define STgcClusterBuilderCommon_h

#include "AthenaBaseComps/AthMessaging.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "STgcClusterPosition.h"

#include <vector>
#include <array>
#include <optional>


namespace MuonGM
{
  class MuonDetectorManager;
}

namespace Muon
{

  // Forward declaration
  class sTgcPrepData;

  class STgcClusterBuilderCommon: public AthMessaging {
    public:
      /// Constructor
      STgcClusterBuilderCommon(const sTgcIdHelper& idHelper);

      /// Separate the sTGC PRDs by layer, from 0 to 7, and sort the PRDs per layer in ascending order of strip number
      std::array<std::vector<sTgcPrepData>, 8> sortSTGCPrdPerLayer(std::vector<sTgcPrepData>&& stripPrds) const;

      /// Find strip clusters, assuming the input vector of PRDs are sorted in ascending order of strip number
      std::vector<std::vector<sTgcPrepData>> findStripCluster(std::vector<sTgcPrepData>&& strips,
                                                              const int maxMissingStrip) const;

      /// Compute the cluster position using the weighted average method
      std::optional<STgcClusterPosition> weightedAverage(const std::vector<sTgcPrepData>& cluster,
                                                         const double resolution,
                                                         bool isStrip) const;

      /// Method to fit analytically a cluster to a Gaussian function to obtain the position of the cluster
      /// The method supports only strip clusters having at least 3 strips.
      ///   A Gaussian function y(x) = A exp(-(x - \mu)^2 / (2 \sigma^2)) can be written as a quadratic function:
      ///       ln(y(x)) = ln(A) - (x - \mu)^2 / (2 \sigma^2) 
      ///                = ln(A) - \mu^2 / (2 \sigma^2) - \mu x / \sigma^2 - x^2 / (2 \sigma^2)
      ///                = a + b x + c x^2
      ///   where a = ln(A) - \mu^2 / (2 \sigma^2), b = -\mu / \sigma^2 and c = -1 / / (2 \sigma^2).
      ///   Solving the equation, the mean position is \mu = -b / (2 c) and standard dev is \sigma = sqrt(-1 / (2 c)).
      ///
      /// Parameters positionResolution and angularResolution enter in the estimate of the error on the mean position.
      std::optional<STgcClusterPosition> caruanaGaussianFitting(const std::vector<sTgcPrepData>& cluster,
                                                 const double positionResolution,
                                                 const double angularResolution,
                                                 const MuonGM::MuonDetectorManager* detManager) const;

    private:
      const sTgcIdHelper& m_stgcIdHelper;
  };
}

#endif
