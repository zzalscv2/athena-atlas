/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetEDM/VertexIndexedConstituentUserInfo.h"

namespace jet {

  VertexIndexedConstituentUserInfo::VertexIndexedConstituentUserInfo()  : UserInfoBase(), m_vertex{nullptr} {
    //nop
  } 

  
  VertexIndexedConstituentUserInfo::VertexIndexedConstituentUserInfo(const xAOD::Vertex* vtx) : UserInfoBase(), m_vertex(vtx) {
    //nop
  }
}
