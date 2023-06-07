/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/



#include "TRT_RodDecoder.h"

#include "CoralBase/Attribute.h"
// COOL API include files (CoolKernel)
#include "CoolKernel/DatabaseId.h"
#include "CoolKernel/Exception.h"
#include "CoolKernel/IDatabaseSvc.h"
#include "CoolKernel/IDatabase.h"
#include "CoolKernel/IFolder.h"
#include "CoolKernel/IObject.h"
#include "CoolKernel/IObjectIterator.h"
#include "CoolKernel/Record.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolApplication/DatabaseSvcFactory.h"
// COOL API include files (CoolApplication)
#include "CoolApplication/Application.h"
#include "StoreGate/ReadCondHandle.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"


#include "InDetByteStreamErrors/TRT_BSErrContainer.h"

/*
 * TRT Specific detector manager to get layout information
 */
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"  

#include "PathResolver/PathResolver.h"
#include <fstream>

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

/* -------------------------------------------------------
 * constructor
 * -------------------------------------------------------
 */

TRT_RodDecoder::TRT_RodDecoder
( const std::string& type, const std::string& name,const IInterface* parent )
  :  base_class              ( type,name,parent ),
     m_CablingSvc            ( "TRT_CablingSvc", name ),
     //     m_bsErrSvc              ( "TRT_ByteStream_ConditionsSvc", name ),
     m_recordBSErrors        ( true ),
     m_lookAtSidErrors       ( true ),
     m_lookAtErrorErrors     ( false ),
     m_lookAtL1idErrors      ( true ),
     m_lookAtBcidErrors      ( true ),
     m_lookAtMissingErrors   ( true ),
     m_loadCompressTableFile ( false ),
     m_loadCompressTableDB   ( true ),
     m_maxCompressionVersion ( 255 ),
     m_forceRodVersion       ( -1 ),
     m_trt_id                ( nullptr ),
     m_eventTypeIsSim        ( false ),
     //     m_Nsymbols              ( 0 ),
     m_escape_marker         ( 0x8000000 ),
     m_Nrdos                 ( 0 )

{
  declareProperty ( "TRT_Cabling", m_CablingSvc );
  declareProperty ( "RecordByteStreamErrors", m_recordBSErrors );
  declareProperty ( "LookAtSidErrors",        m_lookAtSidErrors );
  declareProperty ( "LookAtErrorErrors",      m_lookAtErrorErrors );
  declareProperty ( "LookAtL1idErrors",       m_lookAtL1idErrors );
  declareProperty ( "LookAtBcidErrors",       m_lookAtBcidErrors );
  declareProperty ( "LookAtMissingErrors",    m_lookAtMissingErrors );
  declareProperty ( "LoadCompressTableFile",  m_loadCompressTableFile );
  declareProperty ( "LoadCompressTableDB",    m_loadCompressTableDB );
  declareProperty ( "ForceRodVersion",        m_forceRodVersion );
  declareProperty ( "LoadCompressTableVersions", m_LoadCompressTableVersions );
}

/* ----------------------------------------------------------
 * destructor  
 * ----------------------------------------------------------
 */
TRT_RodDecoder::~TRT_RodDecoder()
{}


/* ----------------------------------------------------------
 * initialize
 * ----------------------------------------------------------
 */

StatusCode TRT_RodDecoder::initialize()
{
  ATH_MSG_DEBUG( " initialize " );

  ATH_CHECK( AlgTool::initialize() );

  m_CompressionTables.resize (m_maxCompressionVersion+1);
  for (std::atomic<EventContext::ContextEvt_t>& evt : m_lastPrint) {
    evt = EventContext::INVALID_CONTEXT_EVT;
  }

  /*
   * Retrieve id mapping 
   */
  ATH_CHECK ( m_CablingSvc.retrieve() );
  ATH_MSG_INFO( "Retrieved tool " << m_CablingSvc );


  /*
   * get detector manager
   */
  const InDetDD::TRT_DetectorManager* indet_mgr = nullptr;
  ATH_CHECK( detStore()->retrieve(indet_mgr,"TRT") );



  // get the helper
  ATH_CHECK( detStore()->retrieve(m_trt_id, "TRT_ID") );
  m_straw_layer_context = m_trt_id->straw_layer_context();


  // get layout
  std::string Layout;
  Layout = indet_mgr->getLayout();
  if ( Layout == "TestBeam" ||
       Layout == "SR1" ||
       Layout == "SR1-EndcapC" )
  {
     ATH_MSG_INFO( "Creating TRT_TB04_RawData RDOs." );
     m_TB04_RawData = true;
     m_LoLumRawData = false;
  }
  else
  {
     ATH_MSG_INFO( "Creating TRT_LoLumRawData RDOs." );
     // ME: bugfix
     m_TB04_RawData = false;
     m_LoLumRawData = true;
  }

  /*
   * Show Look At Me's
   */
  ATH_MSG_INFO( "Look at Sid Errors    : " << m_lookAtSidErrors );
  ATH_MSG_INFO( "Look at Error Errors  : " << m_lookAtErrorErrors );
  ATH_MSG_INFO( "Look at L1ID Errors   : " << m_lookAtL1idErrors );
  ATH_MSG_INFO( "Look at BCID Errors   : " << m_lookAtBcidErrors );
  ATH_MSG_INFO( "Look at Missing Errors: " << m_lookAtMissingErrors );

  // m_loadCompressTableDB is set to (globalflags.DataSource()!='geant4') in JO
  if(m_loadCompressTableDB) {
     ATH_CHECK(m_CompressKey.initialize());
  }else{
    m_eventTypeIsSim=true;
  }

  if ( m_loadCompressTableFile )
  {
    if  ( m_LoadCompressTableVersions.size() == 0 ) 
    {
      m_LoadCompressTableVersions.push_back( 4 );
    }

    std::vector<int>::iterator it;

    for ( it=m_LoadCompressTableVersions.begin() ; 
	  it < m_LoadCompressTableVersions.end(); ++it )
     {
       if ( (*it < 4) || (*it > m_maxCompressionVersion) )
       {
	 ATH_MSG_INFO( "Not reading bad Compress Table Version: " << *it );
       }
       else
       {
	 std::ostringstream ssVersion;
	 ssVersion << *it;
	 std::string compressTableFile = "Compress_Table-" + ssVersion.str() + ".dat";

	 ATH_MSG_INFO( "Reading Compress Table: " << compressTableFile );

	 ATH_CHECK( ReadCompressTableFile( compressTableFile ) );
       }
     }
  }


  if ( m_forceRodVersion > 0 )
  {
    ATH_MSG_WARNING( "****************************" );
    ATH_MSG_WARNING( "****************************" );
    ATH_MSG_WARNING( "* Forcing ROD Version to " << m_forceRodVersion << " *" );
    ATH_MSG_WARNING( "****************************" );
    ATH_MSG_WARNING( "****************************" );
  }

  return StatusCode::SUCCESS;
}


/* ----------------------------------------------------------
 * finalize
 * ----------------------------------------------------------
 */
StatusCode TRT_RodDecoder::finalize() {

  ATH_MSG_VERBOSE( "in TRT_RodDecoder::finalize" );
  if (m_skip>0) {
     ATH_MSG_INFO( "Number of TRT RDOs created: " << m_Nrdos << " hashes: accept " << m_accept << " skipped " << m_skip );
  }
  else {
     ATH_MSG_INFO( "Number of TRT RDOs created: " << m_Nrdos );
  }

  return StatusCode::SUCCESS;
}


/* ----------------------------------------------------------
 * main method to fill the collections into the IDC
 *
 * this is just a switch to call the correct decoding based on the
 * format of the BS.  The version is based on the low order 8 bits of
 * the rod_version header word:
 *       0  : Only used for certain (old) simulation files. Expanded Mode
 *       1,2: Expanded Mode.  1 does not have a modern status block
 *       3  : Minimal Compression Mode.
 * If the version is none of these we complain and return FAILURE.
 * ----------------------------------------------------------
 */
StatusCode
TRT_RodDecoder::fillCollection ( const ROBFragment* robFrag,
				 TRT_RDO_Container* rdoIdc,
				 TRT_BSErrContainer* bsErr,
				 const std::vector<IdentifierHash>* vecHash ) const
{
  // update compression tables
  StatusCode sc;
  if(m_loadCompressTableDB) sc = update(); 

  int    RodBlockVersion          = (robFrag->rod_version() & 0xff);

  if ( m_forceRodVersion > 0 )
    RodBlockVersion = m_forceRodVersion;

  uint32_t robid = robFrag->rod_source_id();

  //  ATH_MSG_INFO( "fC: robid = " 
  //		<< MSG::hex 
  //		<< robid << " L1ID = " << robFrag->rod_lvl1_id()
  //		<< MSG::dec );

  /*
   * Save non-zero rob status to TRT BS Conditions Services
   */
  if ( robFrag->nstatus() )
  { 
    const uint32_t* rob_status;
    robFrag->status(rob_status);

    if ( *rob_status )
    {

      bsErr->add_rob_error( robFrag->rob_source_id(), *rob_status ); 
      


      /*
       * This is a hack to only print once per event.
       */

      const EventContext& ctx{Gaudi::Hive::currentContext()};
      std::atomic<EventContext::ContextEvt_t>* evt = m_lastPrint.get();
      EventContext::ContextEvt_t lastEvt = *evt;
      while (lastEvt != ctx.evt() && !evt->compare_exchange_strong (lastEvt, ctx.evt()))
        ;
      if (lastEvt != ctx.evt()) {  // New event in this slot
        *evt = ctx.evt();
	ATH_MSG_INFO( "Non-Zero ROB status word for ROB " 
		      << MSG::hex 
		      << robFrag->rob_source_id() 
		      << " ( " << *rob_status << " ) "
		      << MSG::dec
		      << "  Skipping decoding..." );
      }

      sc = StatusCode::RECOVERABLE;   // XXX - Evil, but cannot trust anything
      return sc;
    }
  }


  // get version to decide which method to use to decode !
  if ( 3 < RodBlockVersion && m_maxCompressionVersion >= RodBlockVersion )     // Full Compression
  {
    if ( m_CompressionTables[RodBlockVersion] )
      sc = int_fillFullCompress( robFrag, rdoIdc, 
				 *m_CompressionTables[RodBlockVersion],
				 vecHash );  
    else
    {
      if ( m_err_count_fillCollection < 100 )
      {
	ATH_MSG_WARNING( "Rod Version: " << RodBlockVersion		\
			 << ", but Compression Table not loaded!  ROD ID = " \
			 << MSG::hex << robid << MSG::dec );
	m_err_count_fillCollection++;
      }
      else if ( 100 == m_err_count_fillCollection )
      {
	ATH_MSG_WARNING( "Too many Rod Version messages.  "	\
			 << "Turning message off." );
	m_err_count_fillCollection++;
      }
      
      sc = StatusCode::FAILURE;
    }
  }
  else if ( 3 == RodBlockVersion )
    sc = int_fillMinimalCompress( robFrag, rdoIdc, vecHash );
  else if ( (2 == RodBlockVersion) || (1 == RodBlockVersion) )
    sc = int_fillExpanded( robFrag, rdoIdc, vecHash );
  else if ( 0 == RodBlockVersion )
  {
    if ( 0 == robFrag->rod_ndata() )
      return sc;


    if (! m_eventTypeIsSim)
    {
       ATH_MSG_FATAL( "ROD Format Version Number is ZERO.  " \
	    << "and event_type is not EventType::IS_SIMULATION, " \
	    << "ROD ID = " << MSG::hex << robid << MSG::dec );
      return StatusCode::FAILURE;
    }

    sc = int_fillExpanded( robFrag, rdoIdc, vecHash );
  }
  else
  {
  }
  
  /*
   * Decode Status Block, only if we have a modern block and 
   * are OK at this point
   */
  if ( (RodBlockVersion >= 2) && (sc == StatusCode::SUCCESS) )
    {
      /*
       * Error counters
       */
      int sid_errors     = 0;
      int error_errors   = 0;
      int l1id_errors    = 0;
      int bcid_errors    = 0;
      int missing_errors = 0;

      if ( m_recordBSErrors )
      {
	uint32_t rod_L1ID = robFrag->rod_lvl1_id();
	uint32_t rod_BCID = robFrag->rod_bc_id();
	uint32_t rod_SourceID = robFrag->rod_source_id();
      
	OFFLINE_FRAGMENTS_NAMESPACE::PointerType vint;
	robFrag->rod_status( vint );
      
	int v_index=0;
      
	/*
	 * skip mandatory status word
	 */
	//uint32_t mandatory = vint[v_index++];
	v_index++;
      
	uint32_t n_status = vint[v_index++];

        if (n_status > robFrag->rod_nstatus() ) {
           if (n_status > robFrag->rod_fragment_size_word()) {
              ATH_MSG_WARNING("Rejecting fragment because the number of status words exceeds the fragement size: "
                              << n_status << " > " << robFrag->rod_fragment_size_word()
                              << " (nstatus from fragment header = " << robFrag->rod_nstatus()  << ")");
              return StatusCode::RECOVERABLE;
           }
           else {
              ATH_MSG_WARNING("The number of status words exceeds the number of status words marked in the header: "
                              << n_status << " !< " << robFrag->rod_nstatus()
                              << " (fragment size = " << robFrag->rod_fragment_size_word() << ")");
           }
        }

      //     cout << "TRT v_size: " << v_size << " & n_status: " << n_status << endl;
      
      //     for (uint32_t ii=0; ii<v_size; ii++)
      //     	cout << "TRT vint[" << ii << "] = " << hex << vint[ii] << dec << endl;
      
	uint32_t i=1;
	while( i < n_status )
	{
	  uint32_t word = vint[v_index++];
	  
	  for ( int j=0; j<2; j++ )
	  {
	    uint32_t stat = (word >> (16*j)) & 0xffff;
	    if ( stat )   // Skip 0 words, bug in ROD???
	    {
	      int DTMROC_index = stat & 0x7f;
	      int DTMROC_head = (stat >> 7) & 0x1ff;
		  
	      uint32_t Index = (rod_SourceID << 8) | DTMROC_index;
		  
		  //	      cout << i << "/" << j << ":" << hex << Index << dec << " " 
		  //		   << hex << DTMROC_head << dec << ": " << endl;
		  
	      if ( DTMROC_head )
	      {
		int D_sid = DTMROC_head & 0x100;
		uint32_t D_L1ID = (DTMROC_head >> 5) & 0x7;
		uint32_t D_BCID = (DTMROC_head >> 1) & 0xf;
		int D_error = !(DTMROC_head & 1);
		      
		      
		if ( m_lookAtSidErrors && D_sid )
		{
		  //		    cout << "sid ";
		  bsErr->add_sid_error( Index );
		  sid_errors++;
		}
		      
		if ( m_lookAtErrorErrors && D_error )
		{
		  //		    cout << "err ";
		  bsErr->add_error_error( Index );
		  error_errors++;
		}
		      
		if ( m_lookAtL1idErrors && (D_L1ID != (rod_L1ID & 0x7)) )
		{
		  //		    cout << "l1(" << hex << D_L1ID << "/" 
		  //			 << (rod_L1ID & 0x7) << dec;
		  bsErr->add_l1id_error( Index, D_L1ID );
		  l1id_errors++;
		}

		/*
		 * We need to account for the fact that we EXPECT the BCIDs
		 * to be off by 12 for the first 7*16 bunch crossing due to
		 * the way our timing works.  ugh!
		 */
		uint32_t expected_BCID;
		if ( rod_BCID < 7*16 )
		  expected_BCID = (rod_BCID + 12) & 0xf;
		else
		  expected_BCID = rod_BCID & 0xf;

		if ( m_lookAtBcidErrors && (D_BCID != expected_BCID) )
		{
		  //		    cout << "bc(" << hex << D_BCID << "/" 
		  //			 << expected_BCID << dec;
		  bsErr->add_bcid_error( Index, D_BCID );
		  bcid_errors++;
		}
	      } 
	      else if ( m_lookAtMissingErrors )
	      {
		//		 cout << "mis ";
		bsErr->add_missing_error( Index );
		missing_errors++;
	      }
		  
	      //	      cout << endl;
	    } // end of non-zero status check
	  }
	  
	  i++;
	} // End of loop over status words
      }   // End of if on m_recordBSErrors

      uint32_t errorWord = 0;
      
      errorWord  = l1id_errors;
      errorWord |= (bcid_errors << 7);
      errorWord |= (missing_errors << 14);
      errorWord |= (error_errors << 21);
      if ( sid_errors > 0xf ) 
	sid_errors = 0xf;
      errorWord |= ((static_cast<uint32_t>(sid_errors) & 0xf) << 28);   // Only report first 15
      
      if ( errorWord )
      { 
	 //	 std::cout << "ROD id: " << std:: hex <<  rod_SourceID
	 //		   << "errorWord: " << errorWord << std::dec << std::endl;
	 sc=StatusCode::RECOVERABLE;
      }
      
    }

  //  ATH_MSG_INFO( "out of fillCollection: robid = " << MSG::hex << robid << MSG::dec );
  
    return sc;
}


/* ----------------------------------------------------------
 * internal method to fill the collections into the IDC, used for
 * Expanded Mode data.
 *
 * This mode pads the most significant 5 bits with 0s or 1s, depending
 * on the version of the software/hardware generating the BS.
 *
 * If we fail to add a collection to the IDC, we print an ERROR and
 * return the StatusCode from IDC::addOrDelete()
 *
 * If we have been set up in an inconsistant state, wrt type of RDO to
 * create, we have a *serious* problem.  We print a FATAL and return
 * FAILURE.
 * ----------------------------------------------------------
 */
StatusCode
TRT_RodDecoder::int_fillExpanded( const ROBFragment* robFrag,
				  TRT_RDO_Container* rdoIdc,
				  const std::vector<IdentifierHash>* vecHash ) const
{
  // get the ROBid
  uint32_t robid = robFrag->rod_source_id();

  // get the ROD version. It could be used to decode the data in one
  // way or another
  //  eformat::helper::Version rodVersion(robFrag->rod_version()); 
  //  const uint16_t rodMinorVersion= rodVersion.minor(); 

  // get the phasetime for CTB/Cosmics
  int phasetime = 0;
  if ( m_TB04_RawData ) phasetime = robFrag->rod_lvl1_trigger_type();

#ifdef TRT_BSC_DEBUG
  ATH_MSG_DEBUG( "fillCollections for " << MSG::hex << robid << MSG::dec );
#endif
  
  uint32_t         word;
  uint32_t         digit;
  Identifier       idStraw;
  IdentifierHash   idHash, skipHash=0xffffffff,lastHash=0xffffffff;
  const Identifier NULLId(0);

  TRT_RDORawData*                   rdo     = 0;

  // get the data of the fragment
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vint;
  robFrag->rod_data( vint );

  std::unordered_map<IdentifierHash, std::unique_ptr<TRT_RDO_Collection> > colls;
  
  // loop over the data in the fragment
  unsigned int i;
  uint32_t     v_size       = robFrag->rod_ndata();
  int          bufferOffset = -1;
  for ( i=0; i<v_size; i++ )
    {
      // increment offest
      bufferOffset++;

      // get the raw data bit word
      word  = vint[i];
      // mask it
      digit = word & 0x07ffffff;          // We only use 27 bits
      if ( !digit ) continue;             // Empty Straw

      
#ifdef TRT_BSC_DEBUG
      ATH_MSG_VERBOSE( (hex) << robid << " " << bufferOffset << " " << word \
		       << " " << idHash << (dec) << " " << m_trt_id->print_to_string(idStraw) )
#endif
      
      // Make an Identifier for the RDO and get the IdHash
      idStraw = m_CablingSvc->getIdentifier( (eformat::SubDetector) 0 /*unused*/, robid, 
					    bufferOffset, idHash );

      if ( NULLId == idStraw )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( "Null Identifier for bufferOffset " \
			  << bufferOffset << " word " << MSG::hex << word \
			  << MSG::dec )
#endif
	  continue;
	}

#ifdef TRT_BSC_DEBUG
      ATH_MSG_DEBUG( " Collection ID = " << idHash  \
	     << " Straw ID = " << m_trt_id->show_to_string( idStraw ) );
#endif
      
      // this option is for the trigger, if there is a vecHash* given, test it !
      if (vecHash)
	{
	  if (idHash == skipHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
	      continue;
	    }
	  else if (idHash != lastHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "New hash, see if we should decode it" );
#endif
	      lastHash = idHash;
	      // maybe the new hash is not in the list, so test it
	      std::vector<IdentifierHash>::const_iterator p = find(vecHash->begin(),vecHash->end(),idHash);
	      if (p == vecHash->end())
		{
#ifdef TRT_BSC_DEBUG
		   ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
		  // remember this one, so that we do not find(...) forever
		  skipHash = idHash;
		  continue;
		}
	    }
	}
      else {
	  if (idHash == skipHash)
	    {
               ++m_skip;
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
	      continue;
	    }
          ++m_accept;

      }
      // Skip if this collection has already been done.
      if (rdoIdc->indexFindPtr (idHash)) {
        continue;
      }
    
      // get the collection
      std::unique_ptr<TRT_RDO_Collection>& theColl = colls[idHash];

      // Check if the Collection is already created.
      if ( !theColl  )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( " Collection ID = " << idHash \
			  << " does not exist, create it " );
#endif
	  // create new collection
          theColl = std::make_unique<TRT_RDO_Collection> ( idHash );
	  // get identifier from the hash, this is not nice
	  Identifier ident;
	  m_trt_id->get_id(idHash,ident,&m_straw_layer_context);
	  // get the Identifier to be nice to downstream clients
	  theColl->setIdentifier(ident);
	}

      // Now the Collection is there for sure. Create RDO and push it
      // into Collection. 
      if ( m_TB04_RawData && ! m_LoLumRawData )
	rdo = new TRT_TB04_RawData( idStraw, digit, phasetime );
      else if ( m_LoLumRawData && ! m_TB04_RawData )
	rdo = new TRT_LoLumRawData( idStraw, digit ); 
      else
	{
	   ATH_MSG_FATAL( " Inconsistient setting of decoder, this is a bug" );
	  return StatusCode::FAILURE;
	}

      // add the RDO
      theColl->push_back( rdo );
      
    } // End of loop over all words in ROD

  // add collections into IDC
  for (auto& p : colls) {
    ATH_CHECK( rdoIdc->addOrDelete (std::move(p.second), p.first) );
  }
  
  return StatusCode::SUCCESS;
}


/* ----------------------------------------------------------
 * internal method to fill the collections into the IDC, used for
 * Minimal Compression Mode data.
 *
 * This is a reduced case of Full Compression Mode.  Straw words that
 * are 0 are compressed to 1 bit: "1".  Everything else is expanded by
 * prepending 5 "0" bits to the literal straw word.  
 *
 * Note that the input data words are filled "backwords": from LSB to
 * MSB.  For literal straw words, this means that we reading from LSB
 * to MSB, we have 5 0s, followed by the LSB of the straw word.
 *
 * If the incoming BS is not valid, ie the first 0 bit isn't followed
 * by 4 more 0s, we print an ERROR and return FAILURE.
 *
 * If we fail to add a collection to the IDC, we print an ERROR and
 * return the StatusCode from IDC::addOrDelete()
 *
 * If we have been set up in an inconsistant state, wrt type of RDO to
 * create, we have a *serious* problem.  We print a FATAL and return
 * FAILURE.
 * ----------------------------------------------------------
 */

StatusCode
TRT_RodDecoder::int_fillMinimalCompress( const ROBFragment *robFrag,
					TRT_RDO_Container* rdoIdc,
					const std::vector<IdentifierHash>* vecHash) const
{
  uint32_t robid = robFrag->rod_source_id();
  
  // get the ROD version. It could be used to decode the data in one
  // way or another
  //  eformat::helper::Version rodVersion(m_robFrag->rod_version()); 
  //  const uint16_t rodMinorVersion= rodVersion.minor(); 
  
  int phasetime = 0;
  if ( m_TB04_RawData ) phasetime = robFrag->rod_lvl1_trigger_type();
  
#ifdef TRT_BSC_DEBUG
  ATH_MSG_DEBUG( "fillCollection3 for " \
		 << MSG::hex << robid << MSG::dec );
#endif
  
  uint32_t         word;
  uint32_t         digit;
  Identifier       idStraw;
  IdentifierHash   idHash, skipHash=0xffffffff, lastHash=0xffffffff;
  const Identifier NULLId(0);
  
  TRT_RDORawData*                   rdo     = 0;

  
  // get the data of the fragment
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vint;
  robFrag->rod_data( vint );
  
  std::unordered_map<IdentifierHash, std::unique_ptr<TRT_RDO_Collection> > colls;

  int bit=0;
  int v;
  
  // loop over the data in the fragment and decode the bits
  unsigned int in_ptr       = 0;
  unsigned int out_ptr      = 0;
  int          bufferOffset = -1;
  uint32_t     v_size       = robFrag->rod_ndata();

  //  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );

  while ( (in_ptr < v_size) && (out_ptr < 1920) )    // XXX -- avoid HardCode!
    {
      // increment buffer offset
      bufferOffset++;
      out_ptr++;

      // get the next word from the bits
      word = 0;
      v    = (vint[in_ptr] >> bit) & 0x1;
      bit++;
      if ( bit > 31 ) {
	in_ptr++;
	bit = 0;
	//  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );
      }
      if ( v ) continue;
      else {
	for ( int i=0; i<4; i++ ) {          // Look for 4 more 0's
	  v = (vint[in_ptr] >> bit) & 0x1;
	  bit++;
	  if ( bit > 31 ) {
	    in_ptr++;
	    bit = 0;
	    //  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );
	  }
	  if ( v ) {
	    if ( m_err_count_int_fillMinimalCompress < 100 ) {
	       ATH_MSG_WARNING( "Invalid ByteStream, ROD ID = " \
				<< MSG::hex << robid << MSG::dec );
	      m_err_count_int_fillMinimalCompress++;
	    } else if ( 100 == m_err_count_int_fillMinimalCompress ) {
	       ATH_MSG_WARNING( "Too many Invalid ByteStream messages  " \
				<< "Turning message off." );
	      m_err_count_int_fillMinimalCompress++;
	    }
	    return StatusCode::RECOVERABLE;
	  }
	}
	for ( int i=0; i<27; i++ ) {
	  word = word  | (((vint[in_ptr] >> bit) & 0x1) << i);
	  bit++;
	  if ( bit > 31 ) {
	    in_ptr++;
	    bit = 0;
	    //  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );
	  }
	}
      }
      
      // get data word
      digit = word;                       // We only use 27 bits

      
#ifdef TRT_BSC_DEBUG
      ATH_MSG_VERBOSE( (hex) << robid << " " << bufferOffset << " " << word \
		       << " " << idHash << (dec) << " " << m_trt_id->print_to_string(idStraw) );
#endif
    
      // Make an Identifier for the RDO and get the IdHash
      idStraw = m_CablingSvc->getIdentifier( (eformat::SubDetector) 0 /*unused*/, robid, 
					    bufferOffset, idHash );

      if ( NULLId == idStraw )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( "Null Identifier for bufferOffset " << bufferOffset );
#endif
	continue;
      }

#ifdef TRT_BSC_DEBUG
      ATH_MSG_DEBUG( " Collection ID = " << idHash  \
		    << " Straw ID = " << m_trt_id->show_to_string( idStraw ) );
#endif
      
      // this option is for the trigger, if there is a vecHash* given, test it !
      if (vecHash)
	{
	  if (idHash == skipHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
	      continue;
	    }
	  else if (idHash != lastHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "New hash, see if we should decode it" );
#endif
	      lastHash = idHash;
	      // maybe the new hash is not in the list, so test it
	      std::vector<IdentifierHash>::const_iterator p = find(vecHash->begin(),vecHash->end(),idHash);
	      if (p == vecHash->end())
		{
#ifdef TRT_BSC_DEBUG
		   ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
		  // remember this one, so that we do not find(...) forever
		  skipHash = idHash;
		  continue;
		}
	    }
	}
      
      // Skip if this collection has already been done.
      if (rdoIdc->indexFindPtr (idHash)) {
        continue;
      }
    
      // get the collection
      std::unique_ptr<TRT_RDO_Collection>& theColl = colls[idHash];
      
      // Check if the Collection is already created.
      if (  !theColl )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( " Collection ID = " << idHash \
			  << " does not exist, create it " );
#endif
	  // create new collection
          theColl = std::make_unique<TRT_RDO_Collection> ( idHash );
	  // get identifier from the hash, this is not nice
	  Identifier ident;
	  m_trt_id->get_id( idHash, ident, &m_straw_layer_context );
	  // get the Identifier to be nice to downstream clients
	  theColl->setIdentifier(ident);
	}
      
      // Now the Collection is there for sure. Create RDO and push it
      // into Collection. 

      //ATH_MSG_INFO ( "idStraw: " << idStraw 
      //               << " digit: " << MSG::hex << digit << MSG::dec );

      if ( m_TB04_RawData )
      {
	rdo = new TRT_TB04_RawData( idStraw, digit, phasetime );
	m_Nrdos++;
      }
      else if ( m_LoLumRawData )
      {
	rdo = new TRT_LoLumRawData( idStraw, digit ); 
	m_Nrdos++;
      }
      else
      {
	ATH_MSG_FATAL( " Inconsistient setting of decoder, this is a bug" );
	return StatusCode::FAILURE;
      }
      
      // get the collection
      // add the RDO
      theColl->push_back( rdo );
      
    }  //   End of loop over all words in ROD
  
  // add collections into IDC
  for (auto& p : colls) {
    ATH_CHECK( rdoIdc->addOrDelete (std::move(p.second), p.first) );
  }
  
  return StatusCode::SUCCESS;
}


/* ----------------------------------------------------------
 * internal method to fill the collections into the IDC, used for
 * Full Compression Mode data.
 *
 * Note that the input data words are filled "backwords": from LSB to
 * MSB.  For literal straw words, this means that we reading from LSB
 * to MSB, we have 5 0s, followed by the LSB of the straw word.
 *
 * If the incoming BS is not valid, ie the first 0 bit isn't followed
 * by 4 more 0s, we print an ERROR and return FAILURE.
 *
 * If we fail to add a collection to the IDC, we print an ERROR and
 * return the StatusCode from IDC::addOrDelete()
 *
 * If we have been set up in an inconsistant state, wrt type of RDO to
 * create, we have a *serious* problem.  We print a FATAL and return
 * FAILURE.
 * ----------------------------------------------------------
 */

StatusCode
TRT_RodDecoder::int_fillFullCompress( const ROBFragment *robFrag,
				      TRT_RDO_Container* rdoIdc,
				      const t_CompressTable& Ctable,
				      const std::vector<IdentifierHash>* vecHash) const
{
  int phase;
  for ( phase=0; phase<2; phase++ )
  {

  uint32_t robid = robFrag->rod_source_id();
  
  // get the ROD version. It could be used to decode the data in one
  // way or another
  //  eformat::helper::Version rodVersion(m_robFrag->rod_version()); 
  //  const uint16_t rodMinorVersion= rodVersion.minor(); 
  
  int phasetime = 0;
  if ( m_TB04_RawData ) phasetime = robFrag->rod_lvl1_trigger_type();
  
#ifdef TRT_BSC_DEBUG
  ATH_MSG_DEBUG( "fillCollection3 for " \
		 << MSG::hex << robid << MSG::dec );
#endif
  
  uint32_t         word;
  uint32_t         digit;
  Identifier       idStraw;
  IdentifierHash   idHash, skipHash=0xffffffff, lastHash=0xffffffff;
  const Identifier NULLId(0);
  
  TRT_RDORawData*                   rdo     = 0;
  
  // get the data of the fragment
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vint;
  robFrag->rod_data( vint );
  
  std::unordered_map<IdentifierHash, std::unique_ptr<TRT_RDO_Collection> > colls;

  int bit=0;
  int v,l;
  int i;
  
  // loop over the data in the fragment and decode the bits
  unsigned int in_ptr       = 0;
  unsigned int out_ptr      = 0;
  int          bufferOffset = -1;
  uint32_t     v_size       = robFrag->rod_ndata();

  //  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );

  while ( (in_ptr < v_size) && (out_ptr < 1920) )    // XXX -- avoid HardCode!
  {
    // increment buffer offset
    bufferOffset++;
    out_ptr++;

    // get the next word from the bits
    word = 0;
    v    = (vint[in_ptr] >> bit) & 0x1;
    bit++;
    if ( bit > 31 ) 
    {
      in_ptr++;
      bit = 0;
      //  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );
    }
    l = 1;

    //ATH_MSG_INFO( "l, firstcode, v " << l << " " 
    //		  << MSG::hex << Ctable.m_firstcode[l] << " " << v << MSG::dec );

    while ( v < Ctable.m_firstcode[l] )
    {
      v = 2 * v + ((vint[in_ptr] >> bit) & 0x1);
      
      bit++;
      if ( bit > 31 ) 
      {
	in_ptr++;
	bit = 0;
	//  ATH_MSG_WARNING( "vint[" << in_ptr << "] = " << MSG::hex << vint[in_ptr] << MSG::dec );
      }

      l++;

      //ATH_MSG_INFO( "l, firstcode, v " << l << " " 
      //		  << MSG::hex << Ctable.m_firstcode[l] << " " << v << MSG::dec );
    }

    int idx = Ctable.m_lengths_integral[l] + (v - Ctable.m_firstcode[l]);

    //ATH_MSG_INFO ( "lengths_int, idx, syms " << 
    //	   Ctable.m_lengths_integral[l] << " " << idx << " " << MSG::hex <<
    //	   idx << " " << Ctable.m_syms[idx] << MSG::dec );

    if ( idx <= Ctable.m_Nsymbols )
      word = Ctable.m_syms[idx];
    else
    {
      if ( m_err_count_int_fillFullCompress < 100 ) 
      {
	ATH_MSG_WARNING( "Invalid ByteStream, ROD ID = "		\
			 << MSG::hex << robid << MSG::dec );
	m_err_count_int_fillFullCompress++;
      }
      else if ( 100 == m_err_count_int_fillFullCompress ) 
      {
	ATH_MSG_WARNING( "Too many Invalid ByteStream messages  "	\
			 << "Turning message off." );
	m_err_count_int_fillFullCompress++;
      }

      return StatusCode::RECOVERABLE;
    }


    /*
     * Handle case of escaped literal
     */
    if ( word == m_escape_marker )
    {
      word = 0;
      for ( i=0; i<27; i++ )
      {
	word = word  | (((vint[in_ptr] >> bit) & 0x1) << i);
	bit++;
	if ( bit > 31 )
	{
	  in_ptr++;
	  bit = 0;
	}
      }
    }


    if ( 1 == phase )
    {

    if ( word )
    {
      //      std::cout << "PTK: " << bufferOffset << " : " << out_ptr << " : " << word << std::endl;

      // get data word
      digit = word;                       // We only use 27 bits

      
#ifdef TRT_BSC_DEBUG
      ATH_MSG_VERBOSE( (hex) << robid << " " << bufferOffset << " " << word \
		       << " " << idHash << (dec) << " " << m_trt_id->print_to_string(idStraw) );
#endif
    
      // Make an Identifier for the RDO and get the IdHash
      idStraw = m_CablingSvc->getIdentifier( (eformat::SubDetector) 0 /*unused*/, robid, 
					    bufferOffset, idHash );
    
      if ( NULLId == idStraw )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( "Null Identifier for bufferOffset " << bufferOffset );
#endif
	continue;
      }

#ifdef TRT_BSC_DEBUG
      ATH_MSG_DEBUG( " Collection ID = " << idHash  \
		    << " Straw ID = " << m_trt_id->show_to_string( idStraw ) );
#endif
      
      // this option is for the trigger, if there is a vecHash* given, test it !
      if (vecHash)
	{
	  if (idHash == skipHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
	      continue;
	    }
	  else if (idHash != lastHash)
	    {
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "New hash, see if we should decode it" );
#endif
	      lastHash = idHash;
	      // maybe the new hash is not in the list, so test it
	      std::vector<IdentifierHash>::const_iterator p = find(vecHash->begin(),vecHash->end(),idHash);
	      if (p == vecHash->end())
		{
#ifdef TRT_BSC_DEBUG
		   ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
		  // remember this one, so that we do not find(...) forever
		  skipHash = idHash;
		  continue;
		}
	    }
	}
      else {
         if (idHash == skipHash)
	    {
               ++m_skip;
#ifdef TRT_BSC_DEBUG
	       ATH_MSG_DEBUG( "Collection for Hash not to be decoded, skip" );
#endif
               continue;
	    }
         ++m_accept;
      }

      // Skip if this collection has already been done.
      if (rdoIdc->indexFindPtr (idHash)) {
        continue;
      }
    
      // get the collection
      std::unique_ptr<TRT_RDO_Collection>& theColl = colls[idHash];
      
      // Check if the Collection is already created.
      if ( !theColl )
	{
#ifdef TRT_BSC_DEBUG
	   ATH_MSG_DEBUG( " Collection ID = " << idHash \
			  << " does not exist, create it " );
#endif
	  // create new collection
          theColl = std::make_unique<TRT_RDO_Collection> ( idHash );
	  // get identifier from the hash, this is not nice
	  Identifier ident;
	  m_trt_id->get_id( idHash, ident, &m_straw_layer_context );
	  // get the Identifier to be nice to downstream clients
	  theColl->setIdentifier(ident);
	}
      
      // Now the Collection is there for sure. Create RDO and push it
      // into Collection. 

      //      ATH_MSG_INFO ( "idStraw: " << idStraw 
      //		     << " digit: " << MSG::hex << digit << MSG::dec );

      if ( m_TB04_RawData )
      {
	rdo = new TRT_TB04_RawData( idStraw, digit, phasetime );
	m_Nrdos++;
      }
      else if ( m_LoLumRawData )
      {
	rdo = new TRT_LoLumRawData( idStraw, digit ); 
	m_Nrdos++;
      }
      else
	{
	   ATH_MSG_FATAL( " Inconsistient setting of decoder, this is a bug" );
	  return StatusCode::FAILURE;
	}
      
      // get the collection
      // add the RDO
      theColl->push_back( rdo );
    }
    } // if phase == 1
  }  //   End of loop over all words in ROD

  // add collections into IDC
  for (auto& p : colls) {
    ATH_CHECK( rdoIdc->addOrDelete (std::move(p.second), p.first) );
  }

  //  ATH_MSG_INFO( "Input: " << in_ptr << " / " << v_size << "   Output: " << out_ptr << " / 1920" );

  if ( (out_ptr != 1920) || ((in_ptr == v_size) && bit != 0) ||
       ((in_ptr == (v_size - 1)) && (bit == 0)) || (in_ptr < (v_size -1)) )
  {
    ATH_MSG_WARNING( "Decode error: " 
		     << "L1ID = " << MSG::hex << robFrag->rod_lvl1_id()
		     << " ROD = " << robFrag->rod_source_id() << MSG::dec
		     << " bit = " << bit << "  "
		     << in_ptr << " / " << v_size 
		     << " : " << out_ptr << " / 1920" );
    return StatusCode::RECOVERABLE;
  }

  }

  return StatusCode::SUCCESS;
}


/*
 * Read Compression Table from file
 */
StatusCode
TRT_RodDecoder::ReadCompressTableFile(  const std::string& 
#ifdef TRT_READCOMPTABLE_FILE
TableFilename
#endif
 )
{  
   ATH_MSG_FATAL( "Reading Compression Table from File is not supported anymore!" );

   return StatusCode::FAILURE;

#ifdef TRT_READCOMPTABLE_FILE

  auto t_CompressTable Ctable = std::make_unique<t_CompressTable>();

  ATH_MSG_INFO( "Reading Compress Table File: " << TableFilename );

  std::string file = PathResolver::find_file ( TableFilename, "DATAPATH" );
  std::ifstream inFile ( file.c_str() );

  if (!inFile.is_open())
  {
     ATH_MSG_FATAL( "Could not open Compression Table File " 
		    << TableFilename );
     return StatusCode::FAILURE;
  }


#define MAXLINE 1024

  char line[MAXLINE];
  char *tok;

  int *lengths=0;                // Array of codeword lengths
  int *codewords=0;              // Array of codewords


  Ctable->m_Nsymbols = 0;

  while( ! inFile.eof() )
  {
    inFile.getline( line, MAXLINE );

    tok = strtok( line, " \t\n<" );
    if ( ! tok )
      continue;


    /*************************************/
    if ( ! strncmp( tok, "Version", 7 ) )
    {
      tok = strtok( NULL, " \t\n" );
      Ctable->m_TableVersion = atoi( tok );

      ATH_MSG_DEBUG( "Table Version = " << Ctable->m_TableVersion );

      tok = strtok( NULL, " \t\n" );
      if ( ! tok )
      {
	inFile.getline( line, MAXLINE );

	tok = strtok( line, " \t\n" );
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in Version!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

      Ctable->m_syms       = std::make_unique< unsigned int[] >( Ctable->m_Nsymbols );

      if ( lengths )
	 delete[] lengths;
      lengths    = new int[ Ctable->m_Nsymbols ];

      if ( codewords )
	 delete[] codewords;
      codewords  = new int[ Ctable->m_Nsymbols ];
    }


    /*************************************/
    if ( ! strncmp( tok, "Nsymbols", 8 ) )
    {
      tok = strtok( NULL, " \t\n" );
      Ctable->m_Nsymbols = atoi( tok );

      ATH_MSG_DEBUG( "Nsymbols = " << Ctable->m_Nsymbols );

      tok = strtok( NULL, " \t\n" );
      if ( ! tok )
      {
	inFile.getline( line, MAXLINE );

	tok = strtok( line, " \t\n" );
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in Nsymbols!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

      Ctable->m_syms       = std::make_unique< unsigned int[] >( Ctable->m_Nsymbols );

      if ( lengths )
	 delete[] lengths;
      lengths    = new int[ Ctable->m_Nsymbols ];

      if ( codewords )
	 delete[] codewords;
      codewords  = new int[ Ctable->m_Nsymbols ];
    }


    /*************************************/
    if ( ! strncmp( tok, "syms", 4 ) )
    {

      if ( ! Ctable->m_syms )
      {
	ATH_MSG_WARNING( "Invalid file format Nsymbols must come first!" );
	inFile.close();
	
	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

      int i=0;

      tok = strtok( NULL, " \t\n" );
      while( i < Ctable->m_Nsymbols )
      {
	while ( (tok) && (i < Ctable->m_Nsymbols) )
	{
	  Ctable->m_syms[i++] = atoi( tok );
	  tok = strtok( NULL, " \t\n" );
	}

	if ( !tok )
	{
	  inFile.getline( line, MAXLINE );
	  tok = strtok( line, " \t\n" );
	}
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in syms!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }
    }

    /*************************************/
    if ( ! strncmp( tok, "codewords", 9 ) )
    {
      if ( ! codewords )
      {
	ATH_MSG_WARNING( "Invalid file format Nsymbols must come first!" );
	inFile.close();
	
	if ( lengths )
	  delete[] lengths;

	return StatusCode::FAILURE;
      }


      int i=0;

      tok = strtok( NULL, " \t\n" );
      while( i < Ctable->m_Nsymbols )
      {
	while ( (tok) && (i < Ctable->m_Nsymbols) )
	{
	  codewords[i++] = atoi( tok );
	  tok = strtok( NULL, " \t\n" );
	}

	if ( !tok )
	{
	  inFile.getline( line, MAXLINE );
	  tok = strtok( line, " \t\n" );
	}
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in codewords!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

    }


    /*************************************/
    if ( ! strncmp( tok, "firstcode",9 ) )
    {
      int i=1;

      tok = strtok( NULL, " \t\n" );
      while( i < 33 )
      {
	while ( (tok) && (i < 33) )
	{
	  Ctable->m_firstcode[i++] = atoi( tok );
	  tok = strtok( NULL, " \t\n" );
	}

	if ( !tok )
	{
	  inFile.getline( line, MAXLINE );
	  tok = strtok( line, " \t\n" );
	}
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in firstcode" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

    }


    /*************************************/
    if ( ! strncmp( tok, "lengths_integral", 16 ) )
    {
      int i=1;

      tok = strtok( NULL, " \t\n" );
      while( i < 33 )
      {
	while ( (tok) && (i < 33) )
	{
	  Ctable->m_lengths_integral[i++] = atoi( tok );
	  tok = strtok( NULL, " \t\n" );
	}

	if ( !tok )
	{
	  inFile.getline( line, MAXLINE );
	  tok = strtok( line, " \t\n" );
	}
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in lengths_integral!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

    }

    /*************************************/
    if ( ! strncmp( tok, "lengths", 7 ) )
    {
      if ( ! lengths )
      {
	ATH_MSG_WARNING( "Invalid file format Nsymbols must come first!" );
	inFile.close();
	
	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

      int i=0;

      tok = strtok( NULL, " \t\n" );
      while( i < Ctable->m_Nsymbols )
      {
	while ( (tok) && (i < Ctable->m_Nsymbols) )
	{
	  lengths[i++] = atoi( tok );
	  tok = strtok( NULL, " \t\n" );
	}

	if ( !tok )
	{
	  inFile.getline( line, MAXLINE );
	  tok = strtok( line, " \t\n" );
	}
      }

      if ( strncmp( tok, ">", 1 ) )
      {
	ATH_MSG_WARNING( "Invalid file format in lengths!" );
	inFile.close();

	if ( lengths )
	  delete[] lengths;

	if( codewords )
	  delete[] codewords;

	return StatusCode::FAILURE;
      }

    }

  }

  inFile.close();



  if ( ! codewords || ! lengths )
  {
    ATH_MSG_WARNING( "Invalid file format Nsymbols must come first!" );
	
    if ( lengths )
      delete[] lengths;

    if( codewords )
      delete[] codewords;

    return StatusCode::FAILURE;
  }



  /*
   * Recover escape info from the table
   */
  int i=Ctable->m_Nsymbols-1;
  int escape_length=0;
  //  uint32_t escape_codeword;  // Set but not used

  while ( (m_escape_marker != Ctable->m_syms[i]) && (i >=0) )
    i--;

  if ( i < 0 )
  {
    ATH_MSG_WARNING( "Escape code not found!" );

    if ( lengths )
      delete[] lengths;

    if( codewords )
      delete[] codewords;

    return StatusCode::FAILURE;
  }
  else
  {
     //    escape_codeword = codewords[i];   // Set but not used
    escape_length = lengths[i];

    if ( escape_length != 5 )
      ATH_MSG_WARNING( "WARNING!  Escape code length is " << escape_length \
		       << " rather than 5!" );
  }

#ifdef PRINT_TABLE
  for ( int j=0; j<Ctable->m_Nsymbols; j++ )
  {
    ATH_MSG_INFO( "Table: " << j << " " 
		  << lengths[j] << " "  
		  << MSG::hex << codewords[j] << " " 
		  << Ctable->m_syms[j] << std::dec );
  }
#endif // PRINT_TABLE

  if ( lengths )
    delete[] lengths;

  if( codewords )
    delete[] codewords;


  if ( Ctable->m_TableVersion  > m_maxCompressionVersion )
  {
    ATH_MSG_WARNING( "Invalid Compression Table Version: " <<
		     Ctable->m_TableVersion );

    return StatusCode::FAILURE;
  }


  if ( m_CompressionTables[Ctable->m_TableVersion] ) 
  {
    ATH_MSG_WARNING( "Table " << Ctable->m_TableVersion 
		     << " already loaded!  Not overwriting" );
  }
  else
  {
    ATH_MSG_INFO( "Loaded Compress Table Version: " << Ctable->m_TableVersion );
    m_CompressionTables[Ctable->m_TableVersion].store (std::move(Ctable))
  }


  return StatusCode::SUCCESS;

#endif /* TRT_READCOMPTABLE_FILE */

}



/*
 * Read Compression Table from DB on IOV change
 */
StatusCode
TRT_RodDecoder::update() const
{  
  /*
   * function to update compression table when condDB data changes:
   */


  SG::ReadCondHandle<CondAttrListCollection> rst(m_CompressKey);
  const CondAttrListCollection* catrlist = *rst;
  if(!catrlist) {
    ATH_MSG_ERROR( "No Compression Table found in condDB " );
    return StatusCode::FAILURE;
  }


    CondAttrListCollection::const_iterator catrIt (catrlist->begin());
    CondAttrListCollection::const_iterator last_catr (catrlist->end());

    while ( catrIt != last_catr )
    {
       const coral::AttributeList& atrlist = catrIt->second;
     

       int TableVersion = (atrlist)["Version"].data<cool::Int32>();

       if ( TableVersion  > m_maxCompressionVersion )
       {
	 ATH_MSG_WARNING( "Invalid Compression Table Version: " <<
			  TableVersion );

	 ++catrIt;

	 continue;
       }


       if ( m_CompressionTables[TableVersion] ) 
       {
	 ATH_MSG_DEBUG( "Table " << TableVersion 
			  << " already loaded!  Not overwriting" );
         ++catrIt;

	 continue;
       }


       auto Ctable = std::make_unique<t_CompressTable>();
       Ctable->m_TableVersion = TableVersion;
       Ctable->m_Nsymbols = (atrlist)["Nsymbols"].data<cool::Int32>();
       ATH_MSG_DEBUG( "Nsymbols = " << Ctable->m_Nsymbols );


       Ctable->m_syms       = std::make_unique< unsigned int[] >( Ctable->m_Nsymbols );

       const cool::Blob16M& blob = (atrlist)["syms"].data<cool::Blob16M> ();

       if ( blob.size() != (unsigned int) (Ctable->m_Nsymbols * sizeof(unsigned int)) )
       {
	  ATH_MSG_ERROR( "Unexpected size of symbol table! ( " << blob.size()
			 << " != " 
			 << (Ctable->m_Nsymbols * sizeof(unsigned int))
			 << " )" );

	  return StatusCode::FAILURE;
       }

       const unsigned char* BlobStart =
	  static_cast<const unsigned char*> (blob.startingAddress());
       int j = 0;
       for (int i = 0; (i < blob.size()) && (j < Ctable->m_Nsymbols);
	    i += sizeof(unsigned int))
       {
	  Ctable->m_syms[j++] = *((unsigned int*) (BlobStart + i));
       }

       std::istringstream 
	  iss((atrlist)["firstcode"].data<cool::String4k>());
       std::string tok;
       int i = 1;
       while ( getline(iss, tok, ' ') && (i < CTABLE_FC_LENGTH) ) 
       {
	  Ctable->m_firstcode[i++] = atoi(tok.c_str());
       }


       std::istringstream 
	  iss2((atrlist)["lengths_integral"].data<cool::String4k>());
       i = 1;
       while ( getline(iss2, tok, ' ') && (i < CTABLE_LI_LENGTH) ) 
       {
	  Ctable->m_lengths_integral[i++] = atoi(tok.c_str());
       }


#ifdef NOTDEF
       if ( 0 )
       {
	  ATH_MSG_INFO( "Compress Table Version : " << Ctable->m_TableVersion );
	  ATH_MSG_INFO( "Compress Table Nsymbols: " << Ctable->m_Nsymbols );

	  int i;
	  for ( i=0; i<32; i++ )
	     ATH_MSG_INFO( "Compress Table firstcode  : " << i 
			   << " " << Ctable->m_firstcode[i] );

	  for ( i=0; i<32; i++ )
	     ATH_MSG_INFO( "Compress Table lengths_int: " << i
			   << " " << Ctable->m_lengths_integral[i] );


	  for ( i=0; i<10; i++ )
	     ATH_MSG_INFO( "Compress Table syms: " << i
			   << " " << Ctable->m_syms[i] );

	  ATH_MSG_INFO( "Compress Table syms: [...]" );

	  for ( i=0; i<10; i++ )
	     ATH_MSG_INFO( "Compress Table syms: " << (Ctable->m_Nsymbols - 10 + i)
			   << " " << Ctable->m_syms[(Ctable->m_Nsymbols - 10 + i)] );
       }
#endif /* NOTDEF */

       ATH_MSG_INFO( "Loaded Compress Table Version: " <<
		     Ctable->m_TableVersion );
       m_CompressionTables[Ctable->m_TableVersion].set (std::move(Ctable));


       ++catrIt;

    }

  return StatusCode::SUCCESS;
}

