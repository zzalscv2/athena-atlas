/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCM_ZEROSUPPRESSION_H
#define BCM_ZEROSUPPRESSION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "InDetBCM_RawData/BCM_RDO_Container.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

/** @class BCM_ZeroSuppression
    
  @author Bilge M. Demirkoz <demirkoz@cern.ch>

  This algorithm takes the BCM RDO and zero-suppresses it
  providing a smaller object to go into the AOD.

*/
class BCM_ZeroSuppression final: public AthReentrantAlgorithm {
 public:
  BCM_ZeroSuppression(const std::string& name, ISvcLocator* pSvcLocator);
  ~BCM_ZeroSuppression();
  
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;
  
 private:
  /** A data member to contain the name of the BCM RDO
      that contains the input BCM information.  This data member is
      used as a job option property.
  */
  SG::ReadHandleKey<BCM_RDO_Container>  m_bcmContainerName;
  SG::WriteHandleKey<BCM_RDO_Container> m_bcmOutputName;
};

#endif
