/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEDIGITS_BYTESTREAMTOOL_H
#define TILEBYTESTREAM_TILEDIGITS_BYTESTREAMTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "TileByteStream/TileHid2RESrcID.h"

class TileHWID;
class TileDigitsContainer;

#include <string>

/**
 * @class TileDigitsContByteStreamTool
 * @brief AlgTool class to provide conversion from TileDigitsContainer
 * to ByteStream, and fill it in RawEvent. <p>
 * @author Hong Ma
 * @version  Created, Sept 25, 2002 <p>
 *  requirements:   typedef for DIGITS class method: <p> 
 *   StatusCode convert(DIGITS* cont, RawEvent* re); 
 */

class TileDigitsContByteStreamTool: public AthAlgTool {
  public:

    typedef TileDigitsContainer DIGITS;

    /** constructor
     */
    TileDigitsContByteStreamTool(const std::string& type, const std::string& name,
        const IInterface* parent);

    /** destructor
     */
    virtual ~TileDigitsContByteStreamTool();

    /** AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /** Provides conversion from TileDigitsContainer to BS
     */
    StatusCode convert(DIGITS* cont, FullEventAssembler<TileHid2RESrcID> *fea) const;

  private:

    Gaudi::Property<bool> m_doFragType1{this, "DoFragType1", false, "Do frag type 1"};
    Gaudi::Property<bool> m_doFragType5{this, "DoFragType5", false, "Do frag type 5"};
    Gaudi::Property<bool> m_initializeForWriting{this, "InitializeForWriting", false, "Initialize for writing"};

    SG::ReadCondHandleKey<TileHid2RESrcID> m_hid2RESrcIDKey{this,
        "TileHid2RESrcID", "TileHid2RESrcIDHLT", "TileHid2RESrcID key"};

    const TileHWID* m_tileHWID;

    bool m_verbose;
    int  m_runPeriod;
};

#endif
