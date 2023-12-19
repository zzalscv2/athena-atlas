/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEL2_BYTESTREAMCNV_H
#define TILEBYTESTREAM_TILEL2_BYTESTREAMCNV_H


#include "AthenaBaseComps/AthConstConverter.h"
#include "TileEvent/TileContainer.h"
#include "AthenaKernel/RecyclableDataObject.h"
#include "AthenaKernel/BaseInfo.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "CxxUtils/checker_macros.h"


class DataObject;
class StatusCode;
class IAddressCreator;
class IByteStreamEventAccess;
class StoreGateSvc; 
class IROBDataProviderSvc; 
class TileL2ContByteStreamTool ; 
class ByteStreamCnvSvc;
class TileHid2RESrcID;
class TileROD_Decoder;

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;


class TileRecyclableL2Container
  : public TileL2Container, public DataObject
{
public:
  TileRecyclableL2Container (const TileROD_Decoder& decoder);


protected:
  /**
   * @brief Recycle this object for use in another event.
   *
   * This is called from AthenaKernel/RecyclableDataObject when this object
   * is released by StoreGate.  Unlock the object so that non-const access
   * is again possible, and clear out the contents if the collections.
   */
  void recycle();
};


SG_BASE (TileRecyclableL2Container, TileL2Container);


/**
 * @class TileL2ContByteStreamCnv
 * @brief This AthConstConverter class provides conversion between ByteStream and TileL2Container
 * @author Aranzazu Ruiz
 *
 * This class provides methods to convert the bytestream data into
 * TileL2 objects (muon and transverse energy info) and vice versa. The
 * TileL2Container contains a TileL2 object per superdrawer.
 */

class TileL2ContByteStreamCnv
  : public AthConstConverter
{
  public:
    TileL2ContByteStreamCnv(ISvcLocator* svcloc);

    typedef TileL2ContByteStreamTool  BYTESTREAMTOOL ; 

    virtual StatusCode initialize() override;
    virtual StatusCode createObjConst(IOpaqueAddress* pAddr, DataObject*& pObj) const override;
    virtual StatusCode createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const override;
    virtual StatusCode finalize() override;
    
    /// Storage type and class ID
    virtual long repSvcType() const override { return i_repSvcType(); }
    static long storageType();
    static const CLID& classID();
    
  private: 
    //    BYTESTREAMTOOL* m_tool ;
    ToolHandle<BYTESTREAMTOOL> m_tool;
    
    ServiceHandle<IByteStreamEventAccess> m_byteStreamEventAccess;
    ByteStreamCnvSvc* m_byteStreamCnvSvc;
    
    /** Pointer to StoreGateSvc */
    ServiceHandle<StoreGateSvc> m_storeGate; 
    
    /** Pointer to IROBDataProviderSvc */
    ServiceHandle<IROBDataProviderSvc> m_robSvc;
    
    /** Pointer to TileROD_Decoder */
    ToolHandle<TileROD_Decoder> m_decoder;

    /** Pointer to TileHid2RESrcID */
    const TileHid2RESrcID* m_hid2re;

    /** Queue of data objects to recycle. */
    mutable Athena::RecyclableDataQueue<TileRecyclableL2Container> m_queue ATLAS_THREAD_SAFE;
};
#endif

