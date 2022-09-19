/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARDIGITCONTAINERCNV_P3_H
#define LARTPCNV_LARDIGITCONTAINERCNV_P3_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "LArTPCnv/LArDigitContainer_p3.h"

class MsgStream;
class LArDigitContainer;
class LArOnlineID_Base;
class StoreGateSvc;

class LArDigitContainerCnv_p3  : public T_AthenaPoolTPCnvConstBase<LArDigitContainer, LArDigitContainer_p3>
{
public:

  LArDigitContainerCnv_p3(const LArOnlineID_Base* idHelper,
			  const LArOnlineID_Base* idSCHelper,
			  const StoreGateSvc* m_storeGateSvc);

  using base_class::persToTrans;
  using base_class::transToPers;

  virtual void          persToTrans(const LArDigitContainer_p3* pers, LArDigitContainer* trans, 
				    MsgStream &log) const override;
  virtual void          transToPers(const LArDigitContainer* trans, LArDigitContainer_p3* pers, 
				    MsgStream &log) const override;

 private:
  const LArOnlineID_Base* m_idHelper = nullptr;
  const LArOnlineID_Base* m_idSCHelper = nullptr;
  const StoreGateSvc* m_storeGateSvc = nullptr;
};

#endif
