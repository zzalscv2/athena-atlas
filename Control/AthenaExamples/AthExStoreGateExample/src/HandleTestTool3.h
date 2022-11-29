/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthExStoreGateExample/src/HandleTestTool3.h
 * @author Frank Winklmeier
 * @date July, 2022
 * @brief Test for DecorHandleKey depending on a regular handle key
 */


#ifndef ATHEXSTOREGATEEXAMPLE_HANDLETESTTOOL3_H
#define ATHEXSTOREGATEEXAMPLE_HANDLETESTTOOL3_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "AthExStoreGateExample/MyDataObj.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "IHandleTestTool.h"


namespace AthEx {


class HandleTestTool3 : public extends<AthAlgTool, IHandleTestTool>
{
public:
  using base_class::base_class;

  virtual StatusCode initialize() override;


private:
  SG::ReadHandleKey<MyDataObj> m_rhKey
  { this, "RHKey", "rcont", "ReadHandle key" };

  SG::ReadDecorHandleKey<MyDataObj> m_rdhKey
  { this, "RDecorKey", m_rhKey, "rdecor", "ReadDecorHandleKey depending on RHKey" };

  SG::WriteHandleKey<MyDataObj> m_whKey
  { this, "WHKey", "wcont", "WriteHandle key" };

  SG::WriteDecorHandleKey<MyDataObj> m_wdhKey
  { this, "WDecorKey", m_whKey, "wdecor", "WriteDecorHandleKey depending on WHKey" };
};


} // namespace AthEx


#endif // not ATHEXSTOREGATEEXAMPLE_HANDLETESTTOOL3_H
