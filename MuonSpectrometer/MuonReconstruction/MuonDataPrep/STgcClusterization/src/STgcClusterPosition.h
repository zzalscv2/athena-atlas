/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STgcClusterPosition_h
#define STgcClusterPosition_h

#include "Identifier/Identifier.h"

namespace Muon
{
  class STgcClusterPosition {
    public:
      STgcClusterPosition() = default;
      STgcClusterPosition(const Identifier& id, double pos, double err);

      void setClusterId(const Identifier& id) {m_clusterId = id;}
      void setMeanPosition(double pos) {m_meanPosition = pos;}
      void setErrorSquared(double err) {m_errorSquared = err;}
      Identifier getClusterId() const {return m_clusterId;}
      double getMeanPosition() const {return m_meanPosition;}
      double getErrorSquared() const {return m_errorSquared;}
  
    private:
      // Id of the channel with maximum charge
      Identifier m_clusterId{};
      // Mean local position of the cluster
      double m_meanPosition{0.};
      // Error squared of the mean position, negative value indicates nonexistent/invalid cluster
      double m_errorSquared{-1.};
  };
}

inline Muon::STgcClusterPosition::STgcClusterPosition(const Identifier& id, double pos, double err)
        : m_clusterId(id),
          m_meanPosition(pos), 
          m_errorSquared(err)
{
}

#endif
