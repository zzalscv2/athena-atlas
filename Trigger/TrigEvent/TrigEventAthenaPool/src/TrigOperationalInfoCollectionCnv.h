/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigEventAthenaPool
 * @Class  : TrigOperationalInfoCollection
 *
 * @brief transient persistent converter for TrigOperationalInfoCollection
 *
 * @author Tomasz Bold (based on exampels by Andrew Hamilton and Francesca Bucci)
 *
 * File and Version Information:
 * $Id: TrigOperationalInfoCollectionCnv.h,v 1.2 2009-02-23 18:59:20 ssnyder Exp $
 **********************************************************************************/
#ifndef TRIGEVENTATHENAPOOL_TRIGOPERATIONALINFOCOLLECTIONCNV_H
#define TRIGEVENTATHENAPOOL_TRIGOPERATIONALINFOCOLLECTIONCNV_H


#include "TrigSteeringEvent/TrigOperationalInfoCollection.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
class TrigOperationalInfoCollection_tlp1;

// typedef to the latest persistent version
typedef TrigOperationalInfoCollection_tlp1  TrigOperationalInfoCollection_PERS;

class MsgStream;

class TrigOperationalInfoCollectionCnv_impl;
class TrigOperationalInfoCollectionCnv : public T_AthenaPoolCustomCnv<TrigOperationalInfoCollection, TrigOperationalInfoCollection_PERS > 
{

  friend class CnvFactory<TrigOperationalInfoCollectionCnv>;

 protected:
  
  TrigOperationalInfoCollectionCnv(ISvcLocator* svcloc);
  ~TrigOperationalInfoCollectionCnv();

  TrigOperationalInfoCollection_PERS*  createPersistent(TrigOperationalInfoCollection* transObj);

  TrigOperationalInfoCollection*       createTransient ();

 private:
  TrigOperationalInfoCollectionCnv_impl* m_impl;
};


#endif
