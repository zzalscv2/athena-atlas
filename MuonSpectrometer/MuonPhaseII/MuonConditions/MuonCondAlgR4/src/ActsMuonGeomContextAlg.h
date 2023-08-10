/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDALGR4_ActsMuonGeomContextAlg_H
#define MUONCONDALGR4_ActsMuonGeomContextAlg_H


/** 
 *    This algorithm loads the muon alignment constants from the conditions store and
 *    creates an ActsGeometryContext out of it. This algorithm is meant to persists
 *    temporarily until the intergration developments of the new geometry are finalized without
 *    clobbering with the official acts alignment cond alg
*/  

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/CondHandleKeyArray.h>
#include <ActsGeometryInterfaces/RawGeomAlignStore.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>


class ActsMuonGeomContextAlg: public AthReentrantAlgorithm {
public:
      ActsMuonGeomContextAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~ActsMuonGeomContextAlg() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;
      virtual bool isReEntrant() const override { return false; }

private:
    SG::ReadCondHandleKeyArray<ActsTrk::RawGeomAlignStore> m_readKeys{this, "AlignKeys", {}, 
                                                                "List of all muon alignment stores"};
    SG::WriteCondHandleKey<ActsGeometryContext> m_writeKey{this, "WriteKey", "ActsAlignment", "cond handle key"};
};
#endif