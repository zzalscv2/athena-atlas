// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODJET_JETTRIGAUXCONTAINER_H
#define XAODJET_JETTRIGAUXCONTAINER_H

// Local include(s):
#include "xAODJet/versions/JetTrigAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current jet trigger auxiliary container
   ///
   /// All reconstruction code should attach the typedefed auxiliary
   /// container to the xAOD::JetContainer, so it will be easy to change
   /// the container type as we get new I/O technologies for these
   /// objects.
   ///
   typedef JetTrigAuxContainer_v1 JetTrigAuxContainer;
}

#endif // XAODJET_JETAUXCONTAINER_H
