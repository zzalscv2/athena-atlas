/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/** 
 * @file LArRampCompleteCnv.h
 * @brief AthenaPool converter LArRampComplete
 * @author RD Schaffer <R.D. Schaffer@cern.ch>
 */

#ifndef LArRampCompleteCnv_H
#define LArRampCompleteCnv_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCoolMultChanCnv.h"
#include "LArRawConditions/LArRampComplete.h"
#include "LArRawConditions/LArRampP.h"
#include "LArCondTPCnv/LArRampSubsetCnv_p1.h"

// the latest persistent representation type of LArRampComplete
typedef LArConditionsSubset<LArRampP1> LArRampTransType;
typedef LArRampSubset_p1               LArRampPersType;

typedef T_AthenaPoolCoolMultChanCnv<LArRampComplete, LArRampTransType, LArRampPersType > LArRampCompleteCnvBase;

class LArRampCompleteCnv : public LArRampCompleteCnvBase {
friend class CnvFactory<LArRampCompleteCnv >;
protected:
public:
    LArRampCompleteCnv (ISvcLocator* svcloc) : LArRampCompleteCnvBase(svcloc) {}
protected:
    virtual LArRampPersType*   createPersistent (LArRampTransType* transObj);
    virtual LArRampTransType*  createTransient ();
    LArRampTransType*          createTransient(LArConditionsSubset<LArRampP>* orig);
};

class LArRampCopy
{
public:
    // Copy LArRampP subset to LArRampP1
    void copyOldtoNew(const LArConditionsSubset<LArRampP>* oldRamp,
                      LArConditionsSubset<LArRampP1>* newRamp);
};

#endif

