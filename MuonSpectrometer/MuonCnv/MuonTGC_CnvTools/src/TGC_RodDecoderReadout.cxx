/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TGC_RodDecoderReadout.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TGC_RodDecoderReadout.h"

#include "TgcByteStreamData.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonIdHelpers/TgcIdHelper.h"

#include "Identifier/Identifier.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"

using eformat::helper::SourceIdentifier;
using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;


//================ Constructor =================================================

Muon::TGC_RodDecoderReadout::TGC_RodDecoderReadout(
						   const std::string& t,
						   const std::string& n,
						   const IInterface*  p) :
  base_class(t, n, p),
  m_tgcIdHelper(nullptr)
{
  declareProperty("ShowStatusWords", m_showStatusWords=false);
  declareProperty("SkipCoincidence", m_skipCoincidence=false);
}

//================ Destructor =================================================

Muon::TGC_RodDecoderReadout::~TGC_RodDecoderReadout()
{}

//================ Initialisation =================================================

StatusCode Muon::TGC_RodDecoderReadout::initialize()
{
  StatusCode sc = AthAlgTool::initialize();
  
  if(sc.isFailure()) return sc;
  
  // Retrieve the TgcIdHelper
  if(detStore()->retrieve(m_tgcIdHelper, "TGCIDHELPER").isFailure())
    {
      ATH_MSG_FATAL( " Cannot retrieve TgcIdHelper " );
      return StatusCode::FAILURE;
    }
  
  ATH_MSG_INFO( "initialize() successful in " << name() );
  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode Muon::TGC_RodDecoderReadout::finalize()
{
  StatusCode sc = AthAlgTool::finalize();

  if(m_nCache>0 || m_nNotCache>0) {
    const float cacheFraction = ((float)m_nCache) / ((float)(m_nCache + m_nNotCache));
    ATH_MSG_INFO("Fraction of fills that use the cache = " << cacheFraction);
  }

  return sc;
}

//================ fillCollection ===============================================

StatusCode Muon::TGC_RodDecoderReadout::fillCollection(const ROBFragment& robFrag, TgcRdoContainer& rdoIdc) const
{
  try 
    {
      robFrag.check();
    } catch (eformat::Issue &ex) {// error in fragment  
      ATH_MSG_WARNING(ex.what());
      return StatusCode::SUCCESS;
    }


  // Get the identifier for this fragment
  uint32_t source_id = robFrag.rod_source_id();
  SourceIdentifier sid(source_id);
  
  // do not convert if the TGC collection is already in the converter
  uint16_t rdoId = TgcRdo::calculateOnlineId(sid.subdetector_id(), sid.module_id());
  TgcRdoIdHash rdoIdHash;
  int idHash = rdoIdHash(rdoId);

  // Get the IDC_WriteHandle for this hash (and check if it already exists)
  std::unique_ptr<TgcRdo> rdo(nullptr);
  TgcRdoContainer::IDC_WriteHandle lock = rdoIdc.getWriteHandle( idHash );
  if(lock.alreadyPresent() ){
  	ATH_MSG_DEBUG ( " TGC RDO collection already exist with collection hash = " 
  		<< idHash << ", ID = " << sid.human() << " - converting is skipped!");
        ++m_nCache;
  }
  else{
  	ATH_MSG_DEBUG( " Created new collection with ID = " << sid.human() << ", hash = " << idHash );
        ++m_nNotCache;
  	// Create collection
  	rdo = std::make_unique<TgcRdo>(rdoId, idHash);
  	// Adjust bytestream data
  	OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs;
  	robFrag.rod_data(bs);
  	// Get/fill the collection
  	getCollection(robFrag, rdo.get() );
  	// Add bytestream information
        if (sid.module_id()<13){// ROD
          byteStream2Rdo(bs, rdo.get(), robFrag.rod_source_id() );
        }else if (sid.module_id()>16){// SROD
          byteStreamSrod2Rdo(bs, rdo.get(), robFrag.rod_source_id(), robFrag.rod_ndata() );
        }else{
          ATH_MSG_DEBUG( "Error: Invalid [S]ROD number" );
          return StatusCode::FAILURE;
        }

  	// Add TgcRdo to the container
  	StatusCode status_lock = lock.addOrDelete( std::move( rdo ) );
  	if(status_lock != StatusCode::SUCCESS){
          ATH_MSG_ERROR(" Failed to add TGC RDO collection to container with hash " << idHash );
  	}
  	else{
          ATH_MSG_DEBUG(" Adding TgcRdo collection with hash " << idHash << ", source id = " << sid.human() << " to the TgcRdo Container");
  	}
  }
  
  return StatusCode::SUCCESS;
}

//================ getCollection ===============================================
void Muon::TGC_RodDecoderReadout::getCollection(const ROBFragment& robFrag, TgcRdo* rdo) const
{
  // Retrieve information and set in rdo
  uint32_t source_id = robFrag.rod_source_id();
  SourceIdentifier sid(source_id);
  
  uint16_t rdoId = TgcRdo::calculateOnlineId(sid.subdetector_id(), sid.module_id());
  TgcRdoIdHash rdoIdHash;
  int idHash = rdoIdHash(rdoId);  

  rdo->setL1Id(robFrag.rod_lvl1_id());
  rdo->setBcId(robFrag.rod_bc_id());
  rdo->setTriggerType(robFrag.rod_lvl1_trigger_type());
  rdo->setOnlineId(sid.subdetector_id(), sid.module_id()); // rodId = module_id (ROD: 1-12, SROD: 17-19)
  
  uint32_t nstatus = robFrag.rod_nstatus(); // 5 : ROD , 3 : SROD
  ATH_MSG_DEBUG( " Number of Status Words = " << nstatus ); 
  const uint32_t* status;
  robFrag.rod_status(status);
  if (nstatus == 5 && sid.module_id() < 13){ // ROD
    rdo->setErrors(status[0]);
    rdo->setRodStatus(status[1]);;
    rdo->setLocalStatus(status[3]);
    rdo->setOrbit(status[4]);
  }else if (nstatus == 3 && sid.module_id() > 16){// SROD
    rdo->setErrors(status[0]);
    rdo->setRodStatus(status[1]);
    rdo->setLocalStatus(status[2]);
  }

  if(m_showStatusWords) {
    showStatusWords(source_id, rdoId, idHash, nstatus, status);
  }
  
  return;
}

//================ byteStream2Rdo ===============================================

void Muon::TGC_RodDecoderReadout::byteStream2Rdo(OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs,
						 TgcRdo* rdo,
						 uint32_t source_id) const
{
  ATH_MSG_DEBUG( "Muon::TGC_RodDecoderReadout::byteStream2Rdo" );
  
  // Check that we are filling the right collection
  TGC_BYTESTREAM_SOURCEID sid;
  fromBS32(source_id, sid);
  
  if(rdo->identify() != TgcRdo::calculateOnlineId(sid.side, sid.rodid))
    {
      ATH_MSG_DEBUG( "Error: input TgcRdo id does not match bytestream id" );
      return;
    }
  
  //rdo.setOnlineId(sid.side, sid.rodid); // Standard data
  
  TGC_BYTESTREAM_FRAGMENTCOUNT counters[7] = {
    {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 8}, {0, 9}
  };
  TGC_BYTESTREAM_FRAGMENTCOUNT counter{};
  
  int iBs = 0;
  for(int iCnt = 0; iCnt < 7; iCnt++)
    {
      fromBS32(bs[iBs], counter);
      if(counter.id == counters[iCnt].id)
	{
	  counters[iCnt].count = counter.count;
	  iBs++;
        }
    }
  
  //iBs--;
  for(int iCnt = 0; iCnt < 7; iCnt++)
    {
      if(counters[iCnt].count == 0)
	continue;
      switch(counters[iCnt].id)
        {
        case 1: // Raw data format (SSW format)
	  {
	    ATH_MSG_DEBUG( "fragment" << counters[iCnt].id << 
			   " " << counters[iCnt].count << "words" );
	    for(unsigned iFrag = 0; iFrag < counters[iCnt].count; iFrag++)
	      {
		ATH_MSG_DEBUG( "WORD" << iFrag << ":" << MSG::hex << bs[iBs] );
		iBs++;
	      }
	    break;
	  }
        case 2: // TgcRawData::TYPE_HIT
	  {
	    ATH_MSG_DEBUG( "fragment" << counters[iCnt].id << 
			   " " << counters[iCnt].count << "words" );
	    TGC_BYTESTREAM_READOUTHIT roh;
	    for(unsigned iFrag = 0; iFrag < counters[iCnt].count; iFrag++)
	      {
		ATH_MSG_DEBUG( "WORD" << iFrag << ":" << MSG::hex << bs[iBs] );
		fromBS32(bs[iBs++], roh);

		ATH_MSG_DEBUG( " rdo.subDetectorId():" << rdo->subDetectorId()
			       << " rdo.rodId():" <<rdo->rodId()
			       << " roh.ldbId:" <<roh.ldbId
			       << " roh.sbId:" <<roh.sbId
			       << " rdo.l1Id():"<<rdo->l1Id()
			       << " rdo.bcId():"<<rdo->bcId() );

		uint16_t slbId = roh.sbId;
                // SBLOCs for EIFI are different in online (ByteStream) and offline (RDO).
                // bug #57051: Wrong numbering of SBLOC for Inner Stations (EI/FI) in 12-fold TGC cablings
		// ByteStream : slbId = SBLOC + ip*2 (ip=0, 1, 2), SBLOC = 8 or 9 (EI), 0 or 1 (FI)
		//              slbId =  8, 10, 12,  9, 11, 13,  0,  2,  4,  1,  3,  5
		// RDO        : slbId = SBLOC + ip*4 (ip=0, 1, 2), SBLOC = 1 or 3 (EI), 0 or 2 (FI)
		//              slbId =  1,  5,  9,  3,  7, 11,  0,  4,  8,  2,  6, 10
                if(roh.sbType==TgcRawData::SLB_TYPE_INNER_WIRE ||
                   roh.sbType==TgcRawData::SLB_TYPE_INNER_STRIP) {
                  if(roh.sbId<8) slbId =  roh.sbId   *2;
                  else           slbId = (roh.sbId-8)*2+1;
                }

		TgcRawData* raw =  new TgcRawData(bcTag(roh.bcBitmap),
						  rdo->subDetectorId(),
						  rdo->rodId(),
						  roh.ldbId,
						  slbId,
						  rdo->l1Id(),
						  rdo->bcId(),
						  // http://cern.ch/atlas-tgc/doc/ROBformat.pdf
						  // Table 7 : SB type, bits 15..13
						  // 0,1: doublet wire, strip
						  // 2,3: triplet wire, strip triplet;
						  // 4  : inner wire and strip
						  // TgcRawData::SlbType is defined in TgcRawData.h
						  // 0: SLB_TYPE_DOUBLET_WIRE,
						  // 1: SLB_TYPE_DOUBLET_STRIP,
						  // 2: SLB_TYPE_TRIPLET_WIRE,
						  // 3: SLB_TYPE_TRIPLET_STRIP,
						  // 4: SLB_TYPE_INNER_WIRE,
						  // 5: SLB_TYPE_INNER_STRIP,
						  // 6: SLB_TYPE_UNKNOWN
						  (TgcRawData::SlbType)roh.sbType,
						  (bool)roh.adj,
						  roh.tracklet,
						  roh.channel+40);
				rdo->push_back(raw);
	      }
	    break;
	  }
        case 3: // TgcRawData::TYPE_TRACKLET
	  {
	    if(m_skipCoincidence) break;

	    ATH_MSG_DEBUG( "fragment" << counters[iCnt].id << 
			   " " << counters[iCnt].count << "words" );
	    TGC_BYTESTREAM_READOUTTRIPLETSTRIP rostrip;
	    TGC_BYTESTREAM_READOUTTRACKLET rotrk;
	    for(unsigned iFrag = 0; iFrag < counters[iCnt].count; iFrag++)
	      {
		ATH_MSG_DEBUG( "WORD" << iFrag << ":" << MSG::hex << bs[iBs] );
		fromBS32(bs[iBs], rostrip);

		if(rostrip.slbType == TgcRawData::SLB_TYPE_TRIPLET_STRIP)
		  {
		    TgcRawData* raw = new TgcRawData(bcTag(rostrip.bcBitmap),
						     rdo->subDetectorId(),
						     rdo->rodId(),
						     rostrip.ldbId,
						     rostrip.sbId,
						     rdo->l1Id(),
						     rdo->bcId(),
						     TgcRawData::SLB_TYPE_TRIPLET_STRIP,
						     0,
						     rostrip.seg,
						     rostrip.subc,
						     rostrip.phi);
		    rdo->push_back(raw);
		  }
		else
		  {
		    fromBS32(bs[iBs], rotrk);

		    uint16_t slbId = rotrk.sbId;
		    // SBLOCs for EIFI are different in online (ByteStream) and offline (RDO).
		    // bug #57051: Wrong numbering of SBLOC for Inner Stations (EI/FI) in 12-fold TGC cablings
		    // ByteStream : slbId = SBLOC + ip*2 (ip=0, 1, 2), SBLOC = 8 or 9 (EI), 0 or 1 (FI)
		    //              slbId =  8, 10, 12,  9, 11, 13,  0,  2,  4,  1,  3,  5
		    // RDO        : slbId = SBLOC + ip*4 (ip=0, 1, 2), SBLOC = 1 or 3 (EI), 0 or 2 (FI)
		    //              slbId =  1,  5,  9,  3,  7, 11,  0,  4,  8,  2,  6, 10
		    if(rotrk.slbType==TgcRawData::SLB_TYPE_INNER_WIRE ||
		       rotrk.slbType==TgcRawData::SLB_TYPE_INNER_STRIP) {
		      if(rotrk.sbId<8) slbId =  rotrk.sbId   *2;
		      else             slbId = (rotrk.sbId-8)*2+1;
		    }

		    TgcRawData* raw = new TgcRawData(bcTag(rotrk.bcBitmap),
						     rdo->subDetectorId(),
						     rdo->rodId(),
						     rotrk.ldbId,
						     slbId,
						     rdo->l1Id(),
						     rdo->bcId(),
						     // http://cern.ch/atlas-tgc/doc/ROBformat.pdf 
						     // Table 8 : Slave Board type, bits 30..28
						     // 0,1: doublet wire, strip
						     // 2,3: triplet wire, strip triplet; 
						     // 4,5: inner wire, strip
						     // TgcRawData::SlbType is defined in TgcRawData.h 
						     // 0: SLB_TYPE_DOUBLET_WIRE,
						     // 1: SLB_TYPE_DOUBLET_STRIP,
						     // 2: SLB_TYPE_TRIPLET_WIRE,
						     // 3: SLB_TYPE_TRIPLET_STRIP,
						     // 4: SLB_TYPE_INNER_WIRE,
						     // 5: SLB_TYPE_INNER_STRIP,
						     // 6: SLB_TYPE_UNKNOWN
						     (TgcRawData::SlbType)rotrk.slbType,
						     rotrk.delta,
						     rotrk.seg,
						     rotrk.subm,
						     rotrk.rphi);
		    rdo->push_back(raw);
		  }
		iBs++;
	      }
	    break;
	  }
        case 8: // TgcRawData::TYPE_HIPT
	  {
	    if(m_skipCoincidence) break;

	    ATH_MSG_DEBUG( "fragment" << counters[iCnt].id << 
			   " " << counters[iCnt].count << "words" );
	    TGC_BYTESTREAM_HIPT hpt;
	    TGC_BYTESTREAM_HIPT_INNER hptinner;
	    for(unsigned iFrag = 0; iFrag < counters[iCnt].count; iFrag++)
	      {
		ATH_MSG_DEBUG( "WORD" << iFrag << ":" << MSG::hex << bs[iBs] );
		fromBS32(bs[iBs], hptinner);
		if(hptinner.sector & 4){
                  TgcRawData* raw = new TgcRawData(bcTag(hptinner.bcBitmap),
                                                   rdo->subDetectorId(),
                                                   rdo->rodId(),
                                                   rdo->l1Id(),
                                                   rdo->bcId(),
                                                   hptinner.strip,
                                                   0,
                                                   hptinner.sector,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   hptinner.inner);
                  rdo->push_back(raw);
                }else{
                  fromBS32(bs[iBs], hpt);
                  TgcRawData* raw = new TgcRawData(bcTag(hpt.bcBitmap),
                                                   rdo->subDetectorId(),
                                                   rdo->rodId(),
                                                   rdo->l1Id(),
                                                   rdo->bcId(),
                                                   hpt.strip,
                                                   hpt.fwd,
                                                   hpt.sector,
                                                   hpt.chip,
                                                   hpt.cand,
                                                   hpt.hipt,
                                                   hpt.hitId,
                                                   hpt.sub,
                                                   hpt.delta,
                                                   0);
                  rdo->push_back(raw);
                }
                iBs++;
              }
	    break;
	  }
        case 9: // TgcRawData::TYPE_SL
	  {
	    if(m_skipCoincidence) break;

	    ATH_MSG_DEBUG( "fragment" << counters[iCnt].id << 
			   " " << counters[iCnt].count << "words" );
	    TGC_BYTESTREAM_SL sl;
	    for(unsigned iFrag = 0; iFrag < counters[iCnt].count; iFrag++)
	      {
		ATH_MSG_DEBUG( "WORD" << iFrag << ":" << MSG::hex << bs[iBs] );
		fromBS32(bs[iBs++], sl);

		TgcRawData* raw = new TgcRawData(bcTag(sl.bcBitmap),
						 rdo->subDetectorId(),
						 rdo->rodId(),
						 rdo->l1Id(),
						 rdo->bcId(),
						 sl.cand2plus,
						 static_cast<bool>(sl.fwd),
						 sl.sector,
						 sl.cand,
						 sl.sign,
						 sl.threshold,
						 sl.overlap,
						 sl.veto,
						 sl.roi);
		rdo->push_back(raw);
	      }
	    break;
	  }
        default:
	  ATH_MSG_DEBUG( "Error: Muon::TGC_RodDecoderReadout::byteStream2Rdo Unsupported fragment type "
			 << counters[iCnt].id );
	  break;
        }
    }

  ATH_MSG_DEBUG( "Decoded " << MSG::dec << rdo->size() << " elements" );
  ATH_MSG_DEBUG( "Muon::TGC_RodDecoderReadout::byteStream2Rdo done" );
}



void Muon::TGC_RodDecoderReadout::byteStreamSrod2Rdo(OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs,
                                                     TgcRdo* rdo,
                                                     uint32_t source_id,
                                                     uint32_t ndata) const
{
  ATH_MSG_DEBUG( "Muon::TGC_RodDecoderReadout::byteStreamSrod2Rdo" );
  
  // Check that we are filling the right collection
  TGC_BYTESTREAM_SOURCEID sid;
  fromBS32(source_id, sid);
  //uint16_t rod = sid.rodid & 0x0F; // 1-3
  
  if(rdo->identify() != TgcRdo::calculateOnlineId(sid.side, sid.rodid))  // 24-29
    {
      ATH_MSG_DEBUG( "Error: input TgcRdo id does not match bytestream id" );
      return;
    }

  TGC_BYTESTREAM_NSL_ROI tmpfrag;
  for(uint32_t iBs = 0; iBs < ndata; iBs++){
    ATH_MSG_DEBUG( "WORD" << iBs << ":" << MSG::hex << bs[iBs] );
    fromBS32(bs[iBs], tmpfrag);
    switch(tmpfrag.type)
      {
      case 0 : // RoI
        {
          TGC_BYTESTREAM_NSL_ROI roi;
          fromBS32(bs[iBs], roi);
          TgcRawData* raw = new TgcRawData(roi.bcBitmap+1,
                                           rdo->subDetectorId(),
                                           rdo->rodId(),
                                           rdo->l1Id(),
                                           rdo->bcId(),
                                           static_cast<bool>(roi.fwd),
                                           roi.sector,
                                           roi.innerflag,
                                           roi.coinflag,
                                           static_cast<bool>(roi.charge),
                                           roi.pt,
                                           roi.roi);
          rdo->push_back(raw);
          break;
        }
      case 1 : // HiPT
        {
          TGC_BYTESTREAM_NSL_HIPT hipt;
          fromBS32(bs[iBs], hipt);
          TgcRawData* raw = new TgcRawData(hipt.bcBitmap+1,
                                           rdo->subDetectorId(),
                                           rdo->rodId(),
                                           rdo->l1Id(),
                                           rdo->bcId(),
                                           static_cast<bool>(hipt.strip),
                                           static_cast<bool>(hipt.fwd),
                                           hipt.sector,
                                           hipt.chip,
                                           hipt.cand,
                                           static_cast<bool>(hipt.hipt),
                                           hipt.hitId,
                                           hipt.sub,
                                           hipt.delta,
                                           0);
          rdo->push_back(raw);
          break;
        }
      case 2 : // EIFI
        {
          TGC_BYTESTREAM_NSL_EIFI eifi;
          fromBS32(bs[iBs], eifi);
          for ( int isector = 0; isector < 2; isector++ ){// duplicate for the neighboring trigger sector
            TgcRawData* raw = new TgcRawData(eifi.bcBitmap+1,
                                             rdo->subDetectorId(),
                                             rdo->rodId(),
                                             rdo->l1Id(),
                                             rdo->bcId(),
                                             static_cast<bool>(eifi.fwd),
                                             eifi.sector + isector,
                                             eifi.ei,
                                             eifi.fi,
                                             eifi.chamberid);
            rdo->push_back(raw);
          }
          break;
        }
      case 3 : // TMDB
        {
          TGC_BYTESTREAM_NSL_TMDB tmdb;
          fromBS32(bs[iBs], tmdb);
          for ( int isector = 0; isector < 2; isector++ ){// duplicate for the neighboring trigger sector
            TgcRawData* raw = new TgcRawData(tmdb.bcBitmap+1,
                                             rdo->subDetectorId(),
                                             rdo->rodId(),
                                             rdo->l1Id(),
                                             rdo->bcId(),
                                             false,
                                             tmdb.sector + isector,
                                             tmdb.module,
                                             tmdb.bcid);
            rdo->push_back(raw);
          }
          break;
        }
      case 4 : // NSW 4-5
        {
          TGC_BYTESTREAM_NSW_POS nswpos;
          fromBS32(bs[iBs], nswpos);
          iBs++;
          TGC_BYTESTREAM_NSW_ANG nswang;
          fromBS32(bs[iBs], nswang);
          if (nswang.type != 5){
            ATH_MSG_DEBUG( "Error: non-consecutive NSW words " );
            iBs--;
            continue;
          }
          if (( nswpos.bcBitmap != nswang.bcBitmap )||
              ( nswpos.cand     != nswang.cand     )||
              ( nswpos.input    != nswang.input    )){
            ATH_MSG_DEBUG( "Error: inconsistent NSW words" );
            iBs--;
            continue;
          }

	  uint16_t cand_input_bcid
	    = (nswpos.cand  << TgcRawData::NSW_CAND_BITSHIFT)
	    + (nswang.input << TgcRawData::NSW_INPUT_BITSHIFT)
	    + (nswang.bcid  << TgcRawData::NSW_BCID_BITSHIFT);

          for ( int isector = 0; isector < 2; isector++ ){// duplicate for the neighboring trigger sector
	    TgcRawData* raw = new TgcRawData(nswpos.bcBitmap+1,
					     rdo->subDetectorId(),
					     rdo->rodId(),
					     rdo->l1Id(),
					     static_cast<uint16_t>(rdo->bcId()),
					     static_cast<bool>(nswpos.fwd),
					     static_cast<uint16_t>(nswpos.sector) + isector,
					     nswpos.eta,
					     nswpos.phi,
					     cand_input_bcid, // was nswpos.cand,
					     nswang.angle,
					     nswang.phires,
					     nswang.lowres,
					     nswang.nswid);
	    rdo->push_back(raw);
	  }
          break;
        }
      case 6 : // RPC BIS78 6-7
        {
          TGC_BYTESTREAM_RPCBIS78_POS rpcpos;
          fromBS32(bs[iBs], rpcpos);
          iBs++;
          TGC_BYTESTREAM_RPCBIS78_COIN rpccoin;
          fromBS32(bs[iBs], rpccoin);
          if (rpccoin.type != 7){
            ATH_MSG_DEBUG( "Error: non-consecutive RPCBIS78 words " );
            iBs--;
            continue;
          }
          if (( rpcpos.bcBitmap != rpccoin.bcBitmap )||
              ( rpcpos.cand     != rpccoin.cand     )){
            ATH_MSG_DEBUG( "Error: inconsistent RPCBIS78 words" );
            iBs--;
            continue;
          }

	  uint16_t flag_cand_bcid
	    = (rpccoin.flag << TgcRawData::RPC_FLAG_BITSHIFT)
	    + (rpccoin.cand << TgcRawData::RPC_CAND_BITSHIFT)
	    + (rpccoin.bcid << TgcRawData::RPC_BCID_BITSHIFT);

          for ( int isector = 0; isector < 2; isector++ ){// duplicate for the neighboring trigger sector
            TgcRawData* raw = new TgcRawData(rpcpos.bcBitmap+1,
                                             rdo->subDetectorId(),
                                             rdo->rodId(),
                                             rdo->l1Id(),
                                             static_cast<uint16_t>(rdo->bcId()),
                                             static_cast<bool>(rpcpos.fwd),
                                             static_cast<uint16_t>(rpcpos.sector) + isector,
                                             rpcpos.eta,
                                             rpcpos.phi,
                                             flag_cand_bcid, // was static_cast<uint16_t>(rpccoin.flag),
                                             rpccoin.deta,
                                             rpccoin.dphi);
            rdo->push_back(raw);
          }
          break;
        }
      } // switch
  } // for iBs

  ATH_MSG_DEBUG( "Decoded " << MSG::dec << rdo->size() << " elements" );
  ATH_MSG_DEBUG( "Muon::TGC_RodDecoderReadout::byteStreamSrod2Rdo done" );
}


void Muon::TGC_RodDecoderReadout::showStatusWords(const uint32_t source_id, const uint16_t rdoId, const int idHash, 
						  const uint32_t nstatus, const uint32_t* status) const {
  static const unsigned int maxNStatus = 5;
  static const std::string statusDataWord[maxNStatus] = {
    "First status word specific|generic:", // 0
    "TGC ROD event status              :", // 1
    "ROD VME fileter bits | SSW timeout:", // 2
    "Local status word    | presence   :", // 3
    "orbit count                       :"  // 4
  };

  static const unsigned int maxFirstStatus = 5;
  static const std::string firstStatus[maxFirstStatus] = {
    "incorrect BCID", // 0
    "incorrect L1AID", // 1
    "Timeout occurred in at least one of the FE links. Fragment is incomplete.", // 2 
    "Data may be incorrect, see TGC ROD event status word", // 3
    "An overflow in one of the ROD internal buffers has occurred. The fragment is incomplete." // 4
  };

  static const unsigned int maxTgcRodEventStatus = 31;
  static const std::string tgcRodEventStatus[maxTgcRodEventStatus] = {
    "EC_RXsend    : Error in request to send an event via RXlink", //  0
    "EC_FELdown   : A Front End link has gone down - abandoned", //  1 
    "EC_frame     : Invalid FE link framing words", //  2
    "EC_Glnk      : Front End link G-link error", //  3
    "EC_xor       : Invalid XOR event checksum", //  4
    "EC_ovfl      : Input FE event is too long or FE FIFO overflow", //  5
    "EC_timeout   : Timeout expired for at least one FE link", //  6
    "EC_xormezz   : Bad XOR checksum from mezz board", //  7
    "EC_wc0       : Event has WC=0 or WX>max WC", //  8
    "EC_L1ID      : L1ID mismatch (TTC EVID FIFO vs local).", //  9
    "EC_nohdr     : First word is not header", // 10
    "EC_rectype   : Unrecognized record type", // 11
    "EC_null      : Unexpected nulls in FE input", // 12
    "EC_order     : Word is out of order", // 13
    "EC_LDB       : Invalid or unexpected Star Switch ID", // 14
    "EC_RXovfl    : RXfifo has overflowed", // 15
    "EC_SSWerr    : SSW reports T1C, NRC, T2C, or GlinkNoLock error", // 16
    "EC_sbid      : SBid does not match SBinfo table", // 17
    "EC_SBtype    : SBtype does not match SBinfo table", // 18 
    "EC_duprx     : RX ID is duplicated in the event", // 19
    "EC_ec4       : Unexpected SB L1 Event ID(lo 4)", // 20
    "EC_bc        : Unexpected SB BCID", // 21
    "EC_celladr   : Invalid cell address", // 22
    "EC_hitovfl   : Too many hits in event", // 23
    "EC_trgbit    : Unexpected trigger bits", // 24
    "EC_badEoE    : Bad End-of-event marker received, not 0xFCA", // 25
    "EC_endWCnot0 : WC not 0 after End-of-event marker", // 26
    "EC_noEoE     : No End-of-event marker received", // 27
    "EC_SLGlink   : Sector Logic reports G-Link error", // 28 
    "EC_SLbc      : Sector Logic BCID[2:0] does not match its SB BCID", // 29 
    "EC_unxrxid   : Data from disabled SSW RX ID" // 30 
  }; 

  static const unsigned int maxSSWs = 12;

  static const unsigned int maxPresence = 10;
  static const std::string presence[maxPresence] = {
    "", // 0
    "raw data", // 1
    "hits in readout fmt", // 2
    "tracklets in readout fmt", // 3 
    "hits in chamber fmt", // 4
    "tracklets in chamber fmt", // 5 
    "", // 6
    "", // 7
    "HipT output", // 8
    "Sector Logic" // 9
  };

  static const unsigned int maxLocalStatus = 16;
  static const std::string localStatus[maxLocalStatus] = {
    "hit BCs are merged", //  0
    "tracklet BCs are merged", //  1
    "hits are sorted", //  2
    "tracklets are sorted", //  3 
    "", //  4
    "", //  5
    "", //  6
    "", //  7
    "", //  8
    "", //  9
    "", // 10
    "", // 11
    "", // 12
    "", // 13
    "ROI in this fragment", // 14
    "no L1AID, BCID check wrt ROD" // 15
  };

  ATH_MSG_INFO("***** TGC ROD STATUS WORDS for " 
	       << "source_id=0x" << source_id << ", " 
	       << "rdoId=0x" << rdoId << (rdoId<16 ? " , " : ", ")
	       << "idHash=0x" << idHash << (idHash<16 ? " , " : ", ")
	       << (idHash<12 ? "A" : "C") << (idHash%12+1<10 ? "0" : "") << std::dec << idHash%12+1 
	       << " ******");
  ATH_MSG_INFO("***** Based on http://cern.ch/atlas-tgc/doc/ROBformat.pdf ****************************");
  
  for(uint32_t i=0; i<nstatus && i<maxNStatus; i++) {
    ATH_MSG_INFO(statusDataWord[i] << " status[" << i << "]=0x" << std::hex << status[i]);  
    
    if(i==0) {
      // Table 2 ATLAS standard, first status word, all zero means no known errors 
      for(unsigned int j=0; j<maxFirstStatus; j++) {
	if((status[i] >> j) & 0x1) {
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << firstStatus[j]);
	}
      }
    } else if(i==1) {
      // Table 3 TGC ROD event status word 
      for(unsigned int j=0; j<maxTgcRodEventStatus; j++) {
	if((status[i] >> j) & 0x1) { 
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << tgcRodEventStatus[j]);
	}
      }
    } else if(i==2) {
      // Table 4 Star switch time-out status and ROD VME filter bits
      for(unsigned int j=0; j<maxSSWs; j++) {
	if((status[i] >> j) & 0x1) { 
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << "time-out for SSW" << j);
	}
      }
      for(unsigned int j=0+16; j<=maxSSWs+16; j++) {
	if((status[i] >> j) & 0x1) { 
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << "data from SSW" << j-16 << " gave filter \"accept\"");
	}
      }
    } else if(i==3) {
      // Table 6 Presence bits
      for(unsigned int j=0; j<maxPresence; j++) {
	if(j==0 || j==6 || j==7) continue;
	if((status[i] >> j) & 0x1) { 
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << presence[j]);
	}
      }
      
      // Table 5 Local status word 
      for(unsigned int j=0+16; j<maxLocalStatus+16; j++) {
	if((j>=4+16 && j<=13+16)) continue;
	if((status[i] >> j) & 0x1) { 
	  ATH_MSG_INFO(std::dec << std::setw(3) << j <<  " : " << localStatus[j-16]);
	}
      }
    }
  }
  
  ATH_MSG_INFO("**************************************************************************************");
}
