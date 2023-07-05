/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  PixelRawContByteStreamCnv.h
//   Header file for class PixelRawContByteStreamCnv
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
// classID() - returning PixelRDO_Container ID
// createObj() - creates PixelRDO_Container
// createRep() - convert Pixel_RDO in the container into ByteStream
///////////////////////////////////////////////////////////////////
//  Version 00-00-39 05/03/2007 Daniel Dobos
///////////////////////////////////////////////////////////////////

#ifndef PIXELBYTESTREAM_PIXELRAWCONTRAWEVENTCNV_H
#define PIXELBYTESTREAM_PIXELRAWCONTRAWEVENTCNV_H



#include "GaudiKernel/ServiceHandle.h" //member
#include "AthenaBaseComps/AthConstConverter.h" //inheritance
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h" //SvcHandle template arg
#include "InDetRawData/Pixel1RawData.h"
#include "InDetRawData/InDetRawDataCollection.h"

class PixelRawContByteStreamTool;
class IOpaqueAddress;
class DataObject;


class PixelRawContByteStreamCnv: public AthConstConverter {
 public:

  typedef InDetRawDataCollection<Pixel1RawData> COLLECTION; // define collection format here

  PixelRawContByteStreamCnv(ISvcLocator* svcloc);

  virtual StatusCode initialize() override;

  //! this creates the RawEvent fragments for Pixel
  virtual StatusCode createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const override;

  /// Storage type and class ID
  virtual long repSvcType() const override { return i_repSvcType(); }
  static long storageType();
  static const CLID& classID();

private: 
  const PixelRawContByteStreamTool* m_PixelRawContBSTool;
  ServiceHandle<IByteStreamEventAccess> m_ByteStreamEventAccess; 
};
#endif // PIXELBYTESTREAM_PXIELRAWCONTRAWEVENTCNV_H



