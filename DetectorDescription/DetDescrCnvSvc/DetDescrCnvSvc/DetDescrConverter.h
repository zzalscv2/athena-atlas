/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Detector description conversion service package
 -----------------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: DetDescrConverter.h,v 1.6 2008-12-14 02:24:44 ssnyder Exp $
//<version>	$Name: not supported by cvs2svn $

#ifndef DETDESCRCNVSVC_DETDESCRCONVERTER_H
#define DETDESCRCNVSVC_DETDESCRCONVERTER_H

//<<<<<< INCLUDES                                                       >>>>>>

#include "GaudiKernel/Converter.h"

//<<<<<< PUBLIC TYPES                                                   >>>>>>

class DetDescrCnvSvc;
class DetDescrAddress;
class DataObject;

// Abstract factory to create the converter
template <class TYPE>
class CnvFactory;

//<<<<<< CLASS DECLARATIONS                                             >>>>>>

class DetDescrConverter : public Converter {
   public:
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) = 0;
    virtual StatusCode fillObjRefs(IOpaqueAddress* pAddr, DataObject* pObj);
    virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& pAddr);
    virtual StatusCode fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj);

    /// Storage type for all DetDescrConverters
    static long storageType();

   protected:
    DetDescrConverter(const CLID& myCLID, ISvcLocator* svcloc);
};

#endif  // DETDESCRCNVSVC_DETDESCRCONVERTER_H
