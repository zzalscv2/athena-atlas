/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//author Renato Febbraro
//renato.febbraro@cern.ch
//date February 2008

#ifndef TILELASEROBJ_BYTESTREAMTOOL_H
#define TILELASEROBJ_BYTESTREAMTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "TileByteStream/TileHid2RESrcID.h"

class TileHWID;
class TileLaserObject;

#include <string>

class TileLaserObjByteStreamTool: public AthAlgTool {

  public:

    /** constructor
     */
    TileLaserObjByteStreamTool(const std::string& type, const std::string& name,
        const IInterface* parent);

    /** destructor
     */
    virtual ~TileLaserObjByteStreamTool();

    /** AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

    virtual StatusCode initialize();
    virtual StatusCode finalize();

    /** Provides conversion from TileLaserObject to BS
     */
    StatusCode convertLaser(TileLaserObject* cont, FullEventAssembler<TileHid2RESrcID> *fea);

  private:

    Gaudi::Property<bool> m_initializeForWriting{this, "InitializeForWriting", false, "Initialize for writing"};

    SG::ReadCondHandleKey<TileHid2RESrcID> m_hid2RESrcIDKey{this,
        "TileHid2RESrcID", "TileHid2RESrcIDHLT", "TileHid2RESrcID key"};

    const TileHWID* m_tileHWID;
    bool m_verbose;
};

#endif
