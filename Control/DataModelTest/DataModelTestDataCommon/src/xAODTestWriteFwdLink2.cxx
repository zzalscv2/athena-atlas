/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWriteFwdLink2.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2023
 * @brief Testing writing of forward-declared DataLink.
 */


#include "xAODTestWriteFwdLink2.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthLinks/DataLink.h"
#include "AthContainersInterfaces/IAuxStore.h"


namespace DMTest {


StatusCode xAODTestWriteFwdLink2::initialize()
{
  ATH_CHECK( m_cvecAuxKey.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode xAODTestWriteFwdLink2::execute (const EventContext& ctx) const
{
  unsigned int count = ctx.eventID().event_number() + 1;

  auto cvecaux = std::make_unique<DMTest::CAuxContainer>();
  DataVector<DMTest::C> cvec;
  cvec.setStore (cvecaux.get());
  for (size_t i = 0; i < 10; i++) {
    DMTest::C* c = cvec.push_back (std::make_unique<DMTest::C>());
    c->setAnInt (count*100 + 10+i);
    c->setAFloat (count*100 + 20.5+i);
  }
  SG::WriteHandle<DMTest::CAuxContainer> cvecaux_h (m_cvecAuxKey, ctx);
  ATH_CHECK( cvecaux_h.record (std::move (cvecaux)) );
  return StatusCode::SUCCESS;
}


} // namespace DMTest

