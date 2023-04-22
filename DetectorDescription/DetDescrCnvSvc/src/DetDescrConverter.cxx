/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Detector description conversion service package
 -----------------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: DetDescrConverter.cxx,v 1.6 2008-12-14 02:24:44 ssnyder Exp
//$ <version>	$Name: not supported by cvs2svn $

//<<<<<< INCLUDES                                                       >>>>>>

#include "DetDescrCnvSvc/DetDescrConverter.h"

#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "DetDescrCnvSvc/DetDescrCnvSvc.h"

StatusCode DetDescrConverter::fillObjRefs(IOpaqueAddress* /*pAddr*/,
                                          DataObject* /*pObj*/) {
    return StatusCode::SUCCESS;
}

StatusCode DetDescrConverter::createRep(DataObject* /*pObj*/,
                                        IOpaqueAddress*& /*pAddr*/) {
    return StatusCode::SUCCESS;
}

StatusCode DetDescrConverter::fillRepRefs(IOpaqueAddress* /*pAddr*/,
                                          DataObject* /*pObj*/) {
    return StatusCode::SUCCESS;
}

long DetDescrConverter::storageType() {
    return DetDescr_StorageType;
}

DetDescrConverter::DetDescrConverter(const CLID& myCLID, ISvcLocator* svcloc)
    : Converter(DetDescr_StorageType, myCLID, svcloc) {}
