///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p2.cxx 
// Implementation file for class Jet_p2
// Author: R.Seuster<seuster@cern.ch>
// new JetEDM
/////////////////////////////////////////////////////////////////// 


// JetEventTPCnv includes
#include "JetEventTPCnv/Jet_p2.h"

#include "JetEvent/JetTagInfoBase.h"
#include "JetEvent/JetAssociationBase.h"


// Delete the pointed-to objects, if we own them.
Jet_p2::~Jet_p2()
{
  if (m_ownPointers) {
    for (size_t i = 0; i < m_tagJetInfo.size(); i++)
      delete m_tagJetInfo[i];

    for (size_t i = 0; i < m_associations.size(); i++)
      delete m_associations[i];
  }
}

