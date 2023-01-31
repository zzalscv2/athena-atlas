/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataRead/src/AllocTestWriteWithAlloc.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Test writing AllocTest with a non-default allocator.
 */


#include "AllocTestWriteWithAlloc.h"
#include "DataModelTestDataRead/AllocTestAuxContainer.h"


namespace DMTest {


/**
 * @brief Gaudi initialize method.
 */
StatusCode AllocTestWriteWithAlloc::initialize()
{
  ATH_CHECK( m_containerKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode AllocTestWriteWithAlloc::execute (const EventContext& ctx) const
{
  SG::WriteHandle<AllocTestContainer> cont (m_containerKey, ctx);
  ATH_CHECK( cont.record (std::make_unique<AllocTestContainer>(),
                          std::make_unique<AllocTestAuxContainer>()) );
  for (size_t i = 0; i < 10; i++) {
    cont->push_back (std::make_unique<AllocTest>());
    cont->back()->setAtInt1 (ctx.evt()*100 + i);
    cont->back()->setAtInt2 (ctx.evt()*100 + i + 10);
  }
  return StatusCode::SUCCESS;
}


} // namespace DMTest
