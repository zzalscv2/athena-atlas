// this is a -*- C++ -*- file
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1JetCopyAlgorithm_H
#define L1JetCopyAlgorithm_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "../src/JTMContainers.h"

template<typename T>
class L1JetCopyAlgorithm : public AthReentrantAlgorithm {

 public:

  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  typedef typename T::JetContainer JetContainer; // see JTMContainers.h

  L1JetCopyAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

 private:
  SG::ReadHandleKey<JetContainer> m_jetInContainerKey;
  SG::WriteHandleKey<JetContainer> m_jetOutContainerKey;
  
};

#endif  
