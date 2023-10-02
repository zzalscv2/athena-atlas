// this file is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_VERTEXINDEXEDCONSTITUENTUSERINFO_H
#define JET_VERTEXINDEXEDCONSTITUENTUSERINFO_H

#include "xAODTracking/TrackParticle.h" // Necessary ! other wise next line do not compile !
#include "xAODTracking/Vertex.h"
#include "fastjet/PseudoJet.hh"

namespace jet {
  
  class VertexIndexedConstituentUserInfo : public fastjet::PseudoJet::UserInfoBase  {
    
  public:
    // Default ctor.
    // Sets the particle null.
    VertexIndexedConstituentUserInfo();
    
    /// Constructor from vertex
    VertexIndexedConstituentUserInfo(const xAOD::Vertex* vtx);
    
  
    /// Returns the associated vertex if this constit is a track. Else returns null. */
    const xAOD::Vertex* vertex() const {return m_vertex;} 
  protected:
    const xAOD::Vertex* m_vertex;
    
  };
}

#endif
