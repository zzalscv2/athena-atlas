/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// *****************************************************************************
// **
// ** This abstract interface is used by CaloDetDescr/CaloAlignTool.
// ** The main purpose of this interface is to register align() function
// ** as a callback on Cond DB objects at initialization time.
// ** GeoModelSvc instantiates CaloAlignTool after GeoModelSvc::align() has
// ** been registered, by this way it is guaranteed that
// ** CaloAlignTool::align() gets called after LArDetectorTool::align() and
// ** TileDetectorTool::align()
// **
// *****************************************************************************

#ifndef GEOMODELINTERFACES_IGEOALIGNTOOL_H
#define GEOMODELINTERFACES_IGEOALIGNTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "AthenaKernel/IOVSvcDefs.h"

class IGeoAlignTool : virtual public IAlgTool
{
public:
  DeclareInterfaceID(IGeoAlignTool, 1, 0);

  virtual StatusCode align(IOVSVC_CALLBACK_ARGS) = 0;
};


#endif
