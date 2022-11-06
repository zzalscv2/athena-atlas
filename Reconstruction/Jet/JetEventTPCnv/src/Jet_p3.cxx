/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////// -*- C++ -*- /////////////////////////////
// Jet_p3.cxx 
// Implementation file for class Jet_p3
// Author: P.A.Delsart
// new JetEDM
/////////////////////////////////////////////////////////////////// 


// JetEventTPCnv includes
#include "JetEventTPCnv/Jet_p3.h"

#include "JetEvent/JetTagInfoBase.h"
#include "JetEvent/JetAssociationBase.h"


// Delete the pointed-to objects, if we own them.
Jet_p3::~Jet_p3()
{
  if (m_ownPointers) {
    for (size_t i = 0; i < m_tagJetInfo.size(); i++)
      delete m_tagJetInfo[i];

    for (size_t i = 0; i < m_associations.size(); i++)
      delete m_associations[i];
  }
}

