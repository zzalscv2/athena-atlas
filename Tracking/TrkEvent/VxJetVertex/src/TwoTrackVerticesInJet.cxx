/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           TwoTrackVerticesInJet.cxx  -  Description
                             -------------------
    begin   : Jan 2008
    authors : Giacinto Piacquadio (University of Freiburg) 
              Christian Weiser (University of Freiburg)
    e-mails:  giacinto.piacquadio@physik.uni-freiburg.de)
              christian.weiser@cern.ch
    changes: new!

    (C) Atlas Collaboration 2008

   More information contained in the header file 

 ***************************************************************************/

#include "VxJetVertex/TwoTrackVerticesInJet.h"

#include "VxVertex/VxCandidate.h"

#include "TrkParticleBase/TrackParticleBase.h"

namespace Trk 
{
  
  TwoTrackVerticesInJet::TwoTrackVerticesInJet() = default;

  TwoTrackVerticesInJet::TwoTrackVerticesInJet(std::vector<const xAOD::Vertex*> twoTrackVertices,
                                               std::vector<const TrackParticleBase*> neutralTrackOfVertex)
          :
      m_twoTrackVertices(std::move(twoTrackVertices)),
      m_neutralTrackOfVertex(std::move(neutralTrackOfVertex))
  {}
  

  
  TwoTrackVerticesInJet::~TwoTrackVerticesInJet() 
  {
    deleteAll(m_twoTrackVertices);
    deleteAll(m_neutralTrackOfVertex);
  }
  
  TwoTrackVerticesInJet::TwoTrackVerticesInJet(const TwoTrackVerticesInJet& rhs)         
  {

    std::vector<const xAOD::Vertex*>::const_iterator vxBegin=rhs.m_twoTrackVertices.begin();
    std::vector<const xAOD::Vertex*>::const_iterator vxEnd=rhs.m_twoTrackVertices.end();

    std::vector<const TrackParticleBase*>::const_iterator neuBegin=rhs.m_neutralTrackOfVertex.begin();
    std::vector<const TrackParticleBase*>::const_iterator neuEnd=rhs.m_neutralTrackOfVertex.end();


    for (std::vector<const xAOD::Vertex*>::const_iterator vxIter=vxBegin;
         vxIter!=vxEnd;++vxIter) 
    {
      if (*vxIter!=nullptr)
      {
        const xAOD::Vertex* thisPtr=*vxIter;
        //const xAOD::Vertex* newVertex=thisPtr->clone();
        const xAOD::Vertex* newVertex = new xAOD::Vertex(*thisPtr);
        m_twoTrackVertices.push_back(newVertex);
      }
    }

    for (std::vector<const TrackParticleBase*>::const_iterator neuIter=neuBegin;
         neuIter!=neuEnd;++neuIter)
    {
      if (*neuIter!=nullptr)
      {
        m_neutralTrackOfVertex.push_back(new TrackParticleBase(**neuIter));
      }
    }
  }
  
  TwoTrackVerticesInJet & TwoTrackVerticesInJet::operator= (const TwoTrackVerticesInJet & rhs) 
  {
    if (this!=&rhs)
    {
      deleteAll(m_twoTrackVertices);
      deleteAll(m_neutralTrackOfVertex);
      
      m_twoTrackVertices.clear();
      m_neutralTrackOfVertex.clear();
      
      std::vector<const xAOD::Vertex*>::const_iterator vxBegin=rhs.m_twoTrackVertices.begin();
      std::vector<const xAOD::Vertex*>::const_iterator vxEnd=rhs.m_twoTrackVertices.end();
      
      std::vector<const TrackParticleBase*>::const_iterator neuBegin=rhs.m_neutralTrackOfVertex.begin();
      std::vector<const TrackParticleBase*>::const_iterator neuEnd=rhs.m_neutralTrackOfVertex.end();
      
      
      for (std::vector<const xAOD::Vertex*>::const_iterator vxIter=vxBegin;
           vxIter!=vxEnd;++vxIter) 
      {
        if (*vxIter!=nullptr)
        {
          const xAOD::Vertex* thisPtr=*vxIter;
          //const xAOD::Vertex* newVertex=thisPtr->clone();
          const xAOD::Vertex* newVertex = new xAOD::Vertex(*thisPtr);
          m_twoTrackVertices.push_back(newVertex);
        }
      }
      
      for (std::vector<const TrackParticleBase*>::const_iterator neuIter=neuBegin;
               neuIter!=neuEnd;++neuIter)
      {
        if (*neuIter!=nullptr)
        {
          m_neutralTrackOfVertex.push_back(new TrackParticleBase(**neuIter));
        }
      }

    }
    
    return *this;
  }
  
     
  void TwoTrackVerticesInJet::setTwoTrackVertices(std::vector<const xAOD::Vertex*> twoTrackVertices)
  {
    deleteAll(m_twoTrackVertices);
    m_twoTrackVertices=std::move(twoTrackVertices);
  }

  void TwoTrackVerticesInJet::setNeutralTrackOfVertices(std::vector<const TrackParticleBase*> neutralTrackOfVertex)
  {
    deleteAll(m_neutralTrackOfVertex);
    m_neutralTrackOfVertex=std::move(neutralTrackOfVertex);
  }

  const std::vector<const xAOD::Vertex*> & TwoTrackVerticesInJet::getTwoTrackVertice() const {
    return m_twoTrackVertices;
  }
  
  const std::vector<const TrackParticleBase*> & TwoTrackVerticesInJet::getNeutralTrackOfVertices() const 
  {
    return m_neutralTrackOfVertex;
  }
  

  void TwoTrackVerticesInJet::deleteAll(std::vector<const xAOD::Vertex*> & twoTrackVertices) noexcept
  {
    std::vector<const xAOD::Vertex*>::iterator vxBegin=twoTrackVertices.begin();
    std::vector<const xAOD::Vertex*>::iterator vxEnd=twoTrackVertices.end();

    for (std::vector<const xAOD::Vertex*>::iterator vxIter=vxBegin;
         vxIter!=vxEnd;++vxIter) 
    {
      if (*vxIter!=0)
      {
        delete *vxIter;
      }
    }
    twoTrackVertices.clear();
  }
  
  void TwoTrackVerticesInJet::deleteAll(std::vector<const TrackParticleBase*> & neutralTrackOfVertex) noexcept
  {
    for (const TrackParticleBase* p : neutralTrackOfVertex)
    {
      delete p;
    }
    neutralTrackOfVertex.clear();
  }


}//end namespace Trk

  

        
