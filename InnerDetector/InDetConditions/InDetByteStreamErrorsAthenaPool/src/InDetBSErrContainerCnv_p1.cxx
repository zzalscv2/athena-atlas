/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetBSErrContainerCnv_p1.h"

void InDetBSErrContainerCnv_p1::transToPers
(const InDetBSErrContainer* transCont, InDetBSErrContainer_p1* persCont, MsgStream& /*log*/)
{
  InDetBSErrContainer::const_iterator it = transCont->begin();
  InDetBSErrContainer::const_iterator itEnd = transCont->end();
  (persCont->m_bsErrs).reserve(transCont->size());

  for (; it != itEnd; ++it) {
    // FIXME: Should change type of m_bsErrs, but don't want to cause possible
    // back-compatibility problems.
    std::pair<IdentifierHash, int>* ptr ATLAS_THREAD_SAFE = const_cast<std::pair<IdentifierHash, int>*> (*it);
    (persCont->m_bsErrs).push_back(ptr);
  }
  }

void  InDetBSErrContainerCnv_p1::persToTrans(const InDetBSErrContainer_p1* persCont, InDetBSErrContainer* transCont, MsgStream& /*log*/)
{
  std::vector<std::pair<IdentifierHash, int>* >::const_iterator it = (persCont->m_bsErrs).begin();
  std::vector<std::pair<IdentifierHash, int>* >::const_iterator itEnd = (persCont->m_bsErrs).end();
  transCont->reserve((persCont->m_bsErrs).size());

  for (; it != itEnd; ++it) {
    transCont->push_back(*it);
  }
  
}

//================================================================
InDetBSErrContainer* InDetBSErrContainerCnv_p1::createTransient(const InDetBSErrContainer_p1* persObj, MsgStream& log) {
  std::unique_ptr<InDetBSErrContainer> trans(std::make_unique<InDetBSErrContainer>());
  persToTrans(persObj, trans.get(), log);
  return(trans.release());
}
