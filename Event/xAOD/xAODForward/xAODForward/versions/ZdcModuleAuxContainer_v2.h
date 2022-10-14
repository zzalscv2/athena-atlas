// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ALFADataAuxContainer_v2.h 693858 2015-09-09 10:30:15Z krasznaa $
#ifndef XAODFORWARD_VERSIONS_ZDCMODULEAUXCONTAINER_V2_H
#define XAODFORWARD_VERSIONS_ZDCMODULEAUXCONTAINER_V2_H

// System include(s):
#include <vector>
#include <stdint.h>

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

namespace xAOD {

   /// Auxiliary store for xAOD::ZdcModuleContainer_v2
   ///
   /// This is a custom auxiliary container for xAOD::ZdcModuleContainer_v2,
   /// to be used in the main reconstruction code.
   ///
   /// @author Peter Steinberg <peter.steinberg@bnl.gov>
   ///
   /// $Revision: 693858 $
   /// $Date: 2015-09-09 12:30:15 +0200 (Wed, 09 Sep 2015) $
   ///
   class ZdcModuleAuxContainer_v2 : public AuxContainerBase {

   public:
      /// Default constructor
      ZdcModuleAuxContainer_v2();

   private:

     std::vector<uint32_t> zdcId;
     std::vector<int> zdcSide;
     std::vector<int> zdcModule;
     std::vector<int> zdcType;
     std::vector<int> zdcChannel;

   }; // class ZdcModuleAuxContainer_v2

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::ZdcModuleAuxContainer_v2, xAOD::AuxContainerBase );

#endif //XAODFORWARD_VERSIONS_ZDCMODULEAUXCONTAINER_V2_H
