/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*- 
#ifndef LARATHENAPOOL_LARDIGITCONTAINERCNV_H
#define LARATHENAPOOL_LARDIGITCONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "LArRawEvent/LArDigitContainer.h"

#include "LArTPCnv/LArDigitContainer_p2.h"
// to be replaced in follow-up MR
//#include "LArTPCnv/LArDigitContainer_p3.h"

class LArOnlineID_Base;
class StoreGateSvc;

typedef LArDigitContainer_p2 LArDigitContainerPERS;
// to be replaced in follow-up MR
//typedef LArDigitContainer_p3 LArDigitContainerPERS;

typedef T_AthenaPoolCustomCnv<LArDigitContainer,LArDigitContainerPERS> LArDigitContainerCnvBase;

class LArDigitContainerCnv : public LArDigitContainerCnvBase 
{
public:
  LArDigitContainerCnv(ISvcLocator*);
  StatusCode initialize();
protected:
  virtual LArDigitContainer* createTransient();
  virtual LArDigitContainerPERS* createPersistent(LArDigitContainer*);
 private:
  pool::Guid   m_p0_guid;
  pool::Guid   m_p1_guid;
  pool::Guid   m_p2_guid;
  pool::Guid   m_p3_guid;
  const LArOnlineID_Base* m_idHelper = nullptr;
  const LArOnlineID_Base* m_idSCHelper = nullptr;
  const StoreGateSvc* m_storeGateSvc = nullptr;
};

#endif
