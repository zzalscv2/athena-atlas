/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHVCorrToSCHVCorr.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArElecCalib/LArCalibErrorCode.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"

#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/IOVInfiniteRange.h"



LArHVCorrToSCHVCorr::LArHVCorrToSCHVCorr( const std::string& name, 
				      ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{
}

StatusCode LArHVCorrToSCHVCorr::initialize()
{

  ATH_CHECK(m_scidTool.retrieve());
  ATH_CHECK(m_cablingKeySC.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_contKey.initialize());
  ATH_CHECK(m_outKey.initialize(!m_outKey.empty()));

  return StatusCode::SUCCESS;
}


StatusCode LArHVCorrToSCHVCorr::stop()
{  
  
  //Retrieve HVCorr for regular cells
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<ILArHVScaleCorr> hvHdl(m_contKey, ctx);
  if(!hvHdl.isValid()) {
      ATH_MSG_ERROR( "Do not have HVScaleCorr from key " << m_contKey.key() );
      return StatusCode::FAILURE;
  }

  //Retrive SuperCell online id
  const LArOnline_SuperCellID* onlSCID = nullptr;
  CHECK(detStore()->retrieve(onlSCID));

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling{*cablingHdl};
  if(!cabling) {
      ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKey.key() );
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdlSC{m_cablingKeySC, ctx};
  const LArOnOffIdMapping* cablingSC{*cablingHdlSC};
  if(!cablingSC) {
      ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKeySC.key() );
      return StatusCode::FAILURE;
  }

  const CaloCell_SuperCell_ID* calosccellID=0;
  ATH_CHECK( detStore()->retrieve (calosccellID, "CaloCell_SuperCell_ID") );
  const unsigned hashMax=calosccellID->calo_cell_hash_max();
  ATH_MSG_INFO("SuperCell hash max: " << hashMax);

  //Set up AttributeListCollection 
  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification();
  spec->extend("HVScaleCorr", "blob");
  spec->extend<unsigned>("version");
  
  coral::AttributeList attrList(*spec);   
  attrList["version"].setValue(0U);
  coral::Blob& hvBlob=attrList["HVScaleCorr"].data<coral::Blob>();

  spec->release();
  // cppcheck-suppress memleak
  spec = nullptr;

  // Important, blob is ordered by LAr online hash, but LArHVCorr by cell offline hash !!!!
  hvBlob.resize(onlSCID->channelHashMax()*sizeof(float));
    
  float *pHV=static_cast<float*>(hvBlob.startingAddress());
  std::vector<float> vScale;
  //Initialize blobs to ERRORCODE
  for (unsigned i=0;i<onlSCID->channelHashMax();++i) {
    pHV[i]=LArElecCalib::ERRORCODE;
  }
  if(!m_outKey.empty()) vScale.resize(hashMax,(float)1.0);
   
  unsigned nTileIds=0;
  unsigned nTotalIds=0;
  for (unsigned i=0; i < hashMax; ++i) {
    if(calosccellID->sub_calo(IdentifierHash(i)) > 2) continue; // not a LAr channel  
    const Identifier scId=calosccellID->cell_id(IdentifierHash(i));

    const std::vector<Identifier> &cellIds=m_scidTool->superCellToOfflineID(scId);
    if (cellIds.empty()) {
      ATH_MSG_ERROR("Got empty vector of cell ids for super cell id 0x" 
		      << std::hex << scId.get_compact()<<std::dec);
		      return StatusCode::FAILURE;
    }
    float hvcorr=0.;
    for(const Identifier cellId : cellIds) {
        hvcorr += hvHdl->HVScaleCorr(cabling->createSignalChannelID(cellId));
    }
    hvcorr /= cellIds.size();

    // Important, blob is ordered by LAr online hash, but LArHVCorr by cell offline hash !!!!
    pHV[onlSCID->channel_Hash(cablingSC->createSignalChannelID(scId))]=hvcorr;

    if(!m_outKey.empty()) vScale[i]=hvcorr;
    
  }//end loop over super-cell hash
  
  //Add to collection
  CondAttrListCollection* coll=new CondAttrListCollection(true);
  CHECK(detStore()->record(coll,m_folderName));
  coll->add(0,attrList);

  ATH_MSG_INFO("Total number of SuperCells:" << nTotalIds << ", Tile:" << nTileIds 
               << ", LAr: " << nTotalIds-nTileIds);

  if(!m_outKey.empty()) {
     SG::WriteCondHandle<LArHVCorr> writeHandle{m_outKey, ctx};
     const EventIDRange fullRange=IOVInfiniteRange::infiniteMixed();
     writeHandle.addDependency (fullRange);
     writeHandle.addDependency(hvHdl);
     writeHandle.addDependency(cablingHdl);
     writeHandle.addDependency(cablingHdlSC);

     auto scHvCorr = std::make_unique<LArHVCorr>(std::move(vScale), cablingSC, calosccellID);
     if (writeHandle.record(std::move(scHvCorr)).isFailure()) {
       ATH_MSG_ERROR("Could not record LArHVCorr object with " << m_outKey.key()
                   << " with EventRange " << writeHandle.getRange() << " into Conditions Store");
       return StatusCode::FAILURE;
     }
     ATH_MSG_INFO("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");
   ILArHVScaleCorr *imscale = nullptr;
   if(detStore()->symLink(scHvCorr.get(), imscale).isFailure()) {
      ATH_MSG_WARNING("Could not symlink " << writeHandle.key() << " to ILArHVScaleCorr");
   }   
  }
  return StatusCode::SUCCESS;
}

