/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//author Renato Febbraro
//renato.febbraro@cern.ch
//date February 2008

// Gaudi includes

// Atlas includes
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadCondHandle.h"

// Tile includes
#include "TileByteStream/TileLaserObjByteStreamTool.h"
#include "TileByteStream/TileROD_Decoder.h"
#include "TileByteStream/TileROD_Encoder.h"
#include "TileEvent/TileLaserObject.h"
#include "TileIdentifier/TileHWID.h"

#include "AthenaKernel/CLASS_DEF.h"



static const InterfaceID IID_ITileLaserObjByteStreamTool("TileLaserObjByteStreamTool", 1, 0);

const InterfaceID& TileLaserObjByteStreamTool::interfaceID() {
  return IID_ITileLaserObjByteStreamTool;
}


// default constructor
 TileLaserObjByteStreamTool::TileLaserObjByteStreamTool
( const std::string& type, const std::string& name,const IInterface* parent )
  : AthAlgTool(type,name,parent)
  , m_tileHWID(0)
  , m_verbose(false)
{
  declareInterface< TileLaserObjByteStreamTool  >( this );
}



// destructor 
TileLaserObjByteStreamTool::~TileLaserObjByteStreamTool() {
}



StatusCode TileLaserObjByteStreamTool::initialize() {

  ATH_MSG_INFO ("Initializing TileLaserObjByteStreamTool");

  ATH_CHECK( detStore()->retrieve(m_tileHWID, "TileHWID") );

  ToolHandle<TileROD_Decoder> dec("TileROD_Decoder");
  ATH_CHECK( dec.retrieve() );

  ATH_CHECK( m_hid2RESrcIDKey.initialize(m_initializeForWriting) );

  return StatusCode::SUCCESS;
}

StatusCode TileLaserObjByteStreamTool::finalize() {

  return StatusCode::SUCCESS;
}



