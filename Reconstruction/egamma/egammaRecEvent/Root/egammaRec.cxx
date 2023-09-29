/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaRecEvent/egammaRec.h"

const xAOD::CaloCluster*
egammaRec::caloCluster(size_t index) const
{
  if (index >= m_caloClusters.size() || !m_caloClusters[index].isValid()){
    return nullptr;
  }
  return (*(m_caloClusters[index]));
}

ElementLink<xAOD::CaloClusterContainer>
egammaRec::caloClusterElementLink(size_t index) const
{
  if (index >= m_caloClusters.size() || !m_caloClusters[index].isValid()){
    return {};
  }
  return m_caloClusters[index];
}

const std::vector<ElementLink<xAOD::CaloClusterContainer>> &
egammaRec::caloClusterElementLinks() const {
  return m_caloClusters;
}

const xAOD::TrackParticle*
egammaRec::trackParticle(size_t index) const
{
  if (index >= m_trackParticles.size() || !m_trackParticles[index].isValid()){
    return nullptr;
  }
  return (*(m_trackParticles[index]));
}

ElementLink<xAOD::TrackParticleContainer>
egammaRec::trackParticleElementLink(size_t index) const
{
  if (index >= m_trackParticles.size() || !m_trackParticles[index].isValid()){
    return {};
  }
  return m_trackParticles[index];
}

const std::vector<ElementLink<xAOD::TrackParticleContainer>> &
egammaRec::trackParticleElementLinks() const {
  return m_trackParticles;
}

const xAOD::Vertex*
egammaRec::vertex(size_t index) const
{
  if (index >= m_vertices.size() || !m_vertices[index].isValid()){
    return nullptr;
  }
  return (*(m_vertices[index]));
}

ElementLink<xAOD::VertexContainer>
egammaRec::vertexElementLink(size_t index) const{
  if (index >= m_vertices.size() || !m_vertices[index].isValid()){
    return {};
  }
  return m_vertices.at(index);
}

const std::vector<ElementLink<xAOD::VertexContainer>> &
egammaRec::vertexElementLinks() const {
  return m_vertices;
}

void
egammaRec::pushBackVertex(const ElementLink<xAOD::VertexContainer>& vertexElementLink)
{
  m_vertices.push_back(vertexElementLink);
}

void
egammaRec::pushFrontVertex(const ElementLink<xAOD::VertexContainer>& vertexElementLink)
{
  m_vertices.insert(m_vertices.begin(), vertexElementLink);
}

