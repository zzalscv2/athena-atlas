//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#include "LArCalibUtils/LArPhysWaveShifter.h"
#include "LArElecCalib/ILArPhysCaliTdiff.h"
#include "LArRawConditions/LArOFCBinComplete.h"
#include "GaudiKernel/ToolHandle.h"

#include "LArIdentifier/LArOnline_SuperCellID.h"

#include <iostream>
#include <fstream>
#include <string>

using PhysWaveIt = LArPhysWaveContainer::ConstConditionsMapIterator;

LArPhysWaveShifter::LArPhysWaveShifter (const std::string& name, ISvcLocator* pSvcLocator) 
: AthAlgorithm(name, pSvcLocator),
  m_onlineHelper(nullptr),
  m_larFEBTstart(nullptr),
  m_groupingType("ExtendedSubDetector") // SubDetector, Single, FeedThrough
{  
  // Input contaners' keys
  m_keylist.clear() ;
  declareProperty("KeyList", m_keylist);

  // Output container key
  declareProperty("KeyOutput", m_keyout = "LArPhysWave");

  //
  // Steering options to compute time shifts per FEB (whatever the gain)
  //
  declareProperty("ComputeTimeShiftByFEB",  m_compTimeShiftByFEB = false) ;
  declareProperty("TimeShiftByFEBMode",     m_modeTimeShiftByFEB = 3) ;
  declareProperty("TimeShiftByFEBDump",     m_dumpTimeShiftByFEB = false) ;
  declareProperty("TimeShiftByFEBDumpFile", m_fileTimeShiftByFEB = "TimeShiftFEB.py") ;
  
  //
  // Time shift modes
  // 
  declareProperty("TimeShiftByIndex" ,     m_timeShiftByIndex  = 0);
  declareProperty("TimeShiftByHelper",     m_timeShiftByHelper = false);

  declareProperty("TimeShiftFromPeak" ,    m_timeShiftFromPeak = false) ;
  declareProperty("NindexFromPeak" ,       m_nIndexFromPeak    = 0,  "Number of data points before the peak") ;
  declareProperty("Ndelays",               m_nDelays           = 24, "Number of Delay steps between two ADC samples");
  declareProperty("Nsamplings",            m_nSamplings        = 3,  "Number of ADC samples before the peak");

  declareProperty("TimeShiftByFEB"   ,     m_timeShiftByFEB    = false) ;
  declareProperty("TimeShiftGuardRegion" , m_timeShiftGuardRegion = 0 ) ;
  
  declareProperty("UsePhysCaliTdiff",      m_usePhysCaliTdiff = true);

  declareProperty("TimeShiftOffset",       m_timeShiftOffset = false);
  declareProperty("TimeShiftOffsetValue",  m_timeShiftOffsetValue = 0.);

  // Grouping type
  declareProperty("GroupingType",      m_groupingType);  

  declareProperty("OutputShiftsKey",   m_totalShiftsKey);
  declareProperty("CellByCellShiftsKey",    m_cellByCellShiftsKey);
  
  declareProperty("isSC",m_isSC=false);
  
}

LArPhysWaveShifter::~LArPhysWaveShifter() 
= default;

StatusCode LArPhysWaveShifter::stop() {
  ATH_MSG_INFO( "... in stop()" ) ;
  
  LArWaveHelper larWaveHelper;

  // get LArOnlineID helper
  /*StatusCode sc = detStore()->retrieve(m_onlineHelper, "LArOnlineID");
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Could not get LArOnlineID" );
    return sc;
  }*/
  StatusCode sc;
  if ( m_isSC ) {
    const LArOnline_SuperCellID* ll;
    sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = (const LArOnlineID_Base*)ll;
      ATH_MSG_DEBUG("Found the LArOnlineID helper");
    }
  } else { // m_isSC
    const LArOnlineID* ll;
    sc = detStore()->retrieve(ll, "LArOnlineID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = (const LArOnlineID_Base*)ll;
      ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
    }
  }

  // Retrieve LArPhysWaveTool
  ToolHandle<LArPhysWaveTool> larPhysWaveTool("LArPhysWaveTool");
  sc=larPhysWaveTool.retrieve();
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( " Can't get LArPhysWaveTool " );
    return sc;
  }

  // retrieve PhysCaliTdiff
  const ILArPhysCaliTdiff* larPhysCaliTdiff = nullptr;
  if (m_usePhysCaliTdiff) {
    sc = detStore()->retrieve(larPhysCaliTdiff,m_cellByCellShiftsKey);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_WARNING( "Cannot retrieve LArPhysCaliTdiff with key " <<  m_cellByCellShiftsKey 
			<< ". Disabling use of PhysCaliTdiff values in wave shift." ); 
      m_usePhysCaliTdiff = false;
    }else {
      ATH_MSG_INFO( "LArPhysCaliTdiff successfully retrieved" );
    }
  }

  // compute FEB time shifts
  if ( m_compTimeShiftByFEB ) {
    if ( ( m_modeTimeShiftByFEB==2 || m_modeTimeShiftByFEB==3 ) && m_nIndexFromPeak==0 ) m_nIndexFromPeak=m_nSamplings*m_nDelays;
    sc = ComputeTimeShiftByFEB(m_modeTimeShiftByFEB);
    if(!sc.isSuccess()) {
      ATH_MSG_ERROR( "Can't compute time shifts by FEB." );
      return sc;
    }
  }

  // apply time shifts
  if ( m_timeShiftByHelper ) {
      ATH_MSG_INFO( "Will use helper class for start time." );
      m_timeShiftByIndex  = 0 ;
      m_timeShiftFromPeak = false ;
      m_timeShiftByFEB    = false ;
  }  

  if ( m_timeShiftByIndex != 0 ) {
      ATH_MSG_INFO( "Manually shifting pulses by time index " << m_timeShiftByIndex );
      m_timeShiftFromPeak = false ;
      m_timeShiftByFEB    = false ;
  }

  if ( m_timeShiftFromPeak ) {
      ATH_MSG_INFO( "Manually shifting pulses by a constant index from peak." );
      if ( m_nIndexFromPeak==0 ) m_nIndexFromPeak=m_nSamplings*m_nDelays;
      m_timeShiftByFEB    = false ;
  }

  if (m_timeShiftOffset) {
     m_timeShiftOffsetValue = m_timeShiftOffsetValue>0. ? (float)((long)(m_timeShiftOffsetValue+0.5)) : (float)((long)(m_timeShiftOffsetValue-0.5)); // round to nearest signed integer
     if (m_timeShiftOffsetValue <= -999. || m_timeShiftOffsetValue >= 999.) m_timeShiftOffsetValue = 0.; // avoid clearly nonsense values 
  }

  /* 
    Apply FEB time shifts:
    - can be conputed in the same job...
    - ...or simply read from DetStore (and leaded either from DB of jobOption using LArEventTest/FakeLArTimeOffset
  */
  const ILArFEBTimeOffset* larFebTshift = nullptr;
  if ( m_timeShiftByFEB ) {
      ATH_MSG_INFO( "Manually shifting pulses by *FEB* time indexes." );
      sc = detStore()->retrieve(larFebTshift);
      if (sc.isFailure()) {
        ATH_MSG_ERROR( "Cannot find any FEB time offsets. Please check." );
        return sc;
      }	 
  }
  
  //New TPhysCaliTimeDiff Container to store the final shift
  //LArPhysCaliTdiffComplete* totalShifts=new LArPhysCaliTdiffComplete();
  auto totalShifts = std::make_unique<LArOFCBinComplete>();
  if (totalShifts->setGroupingType(m_groupingType,msg()).isFailure()) {
    ATH_MSG_ERROR( "Failed to set grouping type for LArPhysCaliTdiffComplete object" );
    return StatusCode::FAILURE;
  }
  
  if(totalShifts->initialize().isFailure()) {
     ATH_MSG_ERROR( "Failed to initialize LArPhysCaliTdiffComplete object" );
     return StatusCode::FAILURE;
  }


  // Get the physics waveforms from the detector store 
  const LArPhysWaveContainer* larPhysWaveContainerOld;  
  
  int nchannel = 0 ;

  for (const std::string& key : m_keylist) { // Loop over all containers that are to be processed

    sc= detStore()->retrieve(larPhysWaveContainerOld,key);
    if (sc.isFailure()) {
      ATH_MSG_INFO( "LArPhysWaveContainer (key=" << key << ") not found in StoreGate" );
      continue ;
    }
    ATH_MSG_INFO( "Processing LArPhysWaveContainer from StoreGate, key = " << key );

    // loop over physwave in 'old' container
    for ( unsigned gain = CaloGain::LARHIGHGAIN ; gain < CaloGain::LARNGAIN; gain++ ) { // Loop over all gains
        
        // get iterator for all channels for a gain
        PhysWaveIt wave_it   = larPhysWaveContainerOld->begin(gain);
        PhysWaveIt wave_it_e = larPhysWaveContainerOld->end(gain);

        if ( wave_it ==  wave_it_e ) {
	  ATH_MSG_INFO( "LArPhysWaveContainer (key = " << key << ") has no wave with gain = " << gain );
	  continue; // skip to next gain
	}

        for ( ; wave_it!=wave_it_e; wave_it++) {
    
          if ( nchannel < 100 || ( nchannel < 1000 && nchannel%100==0 ) || nchannel%1000==0 ) 
             ATH_MSG_INFO( "Processing physics waveform number " << nchannel );
          nchannel++ ;
      
          const LArPhysWave* larPhysWave = &(*wave_it);
	  const HWIdentifier chid  = wave_it.channelId();      
	  
	  if ( larPhysWave->isEmpty() ) { 
	    ATH_MSG_VERBOSE("Channel 0x" << MSG::hex << chid.get_compact() << MSG::dec << " is empty. Skipping.");
            continue;
          }
	  
          float tstart = 0 ;      
      
          if ( m_timeShiftByHelper ) {
             tstart  = larWaveHelper.getStart(*larPhysWave) ;
	     tstart -= m_timeShiftGuardRegion ;
          }
      
          if ( m_timeShiftByIndex != 0  ) {
             tstart = m_timeShiftByIndex;
          }
      
          if ( m_timeShiftFromPeak ) {
              tstart = larWaveHelper.getMax(*larPhysWave)-m_nIndexFromPeak;
          }      
      
          if ( m_timeShiftByFEB ) {
            const HWIdentifier febid = m_onlineHelper->feb_Id(chid);
            tstart  = (int)larFebTshift->TimeOffset(febid);
	    tstart -= m_timeShiftGuardRegion ;
          }
      
	  if (m_usePhysCaliTdiff && larPhysCaliTdiff) {
	    float tdiff = larPhysCaliTdiff->Tdiff(chid,gain);
	    if (tdiff<=-999.) tdiff = 0.;
	    tdiff /= (25./24.); // in units of delay steps	    
	    tdiff = tdiff>0. ? (float)((long)(tdiff+0.5)) : (float)((long)(tdiff-0.5)); // round to nearest signed integer
	    ATH_MSG_VERBOSE("Channel 0x" << MSG::hex << chid.get_compact() << MSG::dec << " -> Tdiff = " << tdiff);
	    tstart -= tdiff; // subtract to tstart (should be used on top of timeShiftByFEB)
	  }

          if (m_timeShiftOffset) {
              tstart -= m_timeShiftOffsetValue;
          }

	  totalShifts->set(chid,gain,(int)(tstart));

          ATH_MSG_VERBOSE("PhysWave n. " << nchannel
			  << " --> Time shift for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
			  << " is " << tstart << " samples (" << tstart*larPhysWave->getDt() << " ns)");
      
	  /*
          // apply time shift	  
	  LArPhysWave newLArPhysWave( (larWaveHelper.translate(*larPhysWave,-tstart,0)).getWave() ,
                   		       larPhysWave->getDt(), 
				       larPhysWave->getFlag() ) ;
	  
	  
	  // save time shift in new LArPhysWave 	  
          if ( m_timeShiftByFEB ) {
	    // We save the timeShiftGuardRegion so that, when running
	    // the OFC iteration reconstruction, we can use it in
	    // association to the final phase to compute an absolute
	    // time with respect to the FEB "zero" time, defined as
	    // the peak time of the fastest pulse in the FEB
	    // -- Marco Delmastro and Guillaume Unal, June 2009
	    newLArPhysWave.setTimeOffset(-m_timeShiftGuardRegion);
	  } else {
	    newLArPhysWave.setTimeOffset(tstart);
	  }

          // Add 'new' physics wave to 'new' container
	  larPhysWaveContainerNew->setPdata(chid, newLArPhysWave, gain);
	  
	  // Clean the 'old' physics wave from memory (ok, not that elegant, but still...)
	  //LArPhysWave* oldLArPhysWave = const_cast<LArPhysWave*>(larPhysWave);
	  //oldLArPhysWave->clear(); // clear() method is (still) not implemeted on LArWave...
	  */
       }  // end loop over cells
    } // end of loop over gains
    
  }  // End loop over all PhysWave containers in m_keylist


  if (!m_totalShiftsKey.empty()) {
    sc=detStore()->record(std::move(totalShifts),m_totalShiftsKey);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to recrod LArPhysCaliTdiffComplete with key " << m_totalShiftsKey );
    }
  }

  ATH_MSG_INFO( "LArPhysWaveShifter stopped!" );  
  
  return StatusCode::SUCCESS;
}


StatusCode LArPhysWaveShifter::ComputeTimeShiftByFEB(unsigned mode=2)
{
  StatusCode sc ;

  LArWaveHelper larWaveHelper;

  // This is to *compute* the smaller time shifts by FEB
  m_larFEBTstart = new LArFEBTimeOffset();
  m_larFEBTstart->setDefaultReturnValue(999);
  if (mode==3) {
    std::vector<HWIdentifier>::const_iterator it   = m_onlineHelper->feb_begin();
    std::vector<HWIdentifier>::const_iterator it_e = m_onlineHelper->feb_end();
    for (;it!=it_e;++it)       
      m_larFEBTstart->setTimeOffset(*it,0); 
  }

  // This is a counter to compute the average time shift per FEB
  LArFEBTimeOffset nChanInFEB;
  nChanInFEB.setDefaultReturnValue(0);

  // Get the physics waveforms from the detector store 
  const LArPhysWaveContainer* larPhysWaveContainerOld;  

  for (const std::string& key : m_keylist) { // Loop over all containers that are to be processed

    sc=detStore()->retrieve(larPhysWaveContainerOld,key);
    if (sc.isFailure()) {
      ATH_MSG_INFO( "LArPhysWaveContainer (key=" << key << ") not found in StoreGate" );
      continue ;
    }
    if ( larPhysWaveContainerOld == nullptr ) {
      ATH_MSG_INFO( "LArPhysWaveContainer (key=" << key << ") is empty" );
      continue ;
    }
    
    ATH_MSG_INFO( "ComputeTimeShiftByFEB(): processing LArPhysWaveContainer from StoreGate, key = " << key );

    for ( unsigned gain = CaloGain::LARHIGHGAIN ; gain < CaloGain::LARNGAIN; gain++ ) { // Loop over all gains
        
        PhysWaveIt wave_it   = larPhysWaveContainerOld->begin(gain);
        PhysWaveIt wave_it_e = larPhysWaveContainerOld->end(gain);

        if ( wave_it ==  wave_it_e ) {
	  ATH_MSG_INFO( "ComputeTimeShiftByFEB(): LArPhysWaveContainer (key = " << key << ") has no wave with gain = " << gain );
	  continue; // skip to next gain
	}
	
        for ( ; wave_it!=wave_it_e; wave_it++) {
      
          const LArPhysWave* larPhysWave = &(*wave_it);
	  if ( larPhysWave->isEmpty() ) continue;
	  
	  const HWIdentifier chid  = wave_it.channelId();
          const HWIdentifier febid = m_onlineHelper->feb_Id(chid);
          
	  unsigned oldFEBTstart = (unsigned)m_larFEBTstart->TimeOffset(febid);      
          unsigned newFEBTstart = 999;
	  unsigned theChanInFEB = 0;
	  float bindiff = 0.;

	  switch (mode) {
	  case 1 :
	    newFEBTstart = (unsigned)larWaveHelper.getStart(*larPhysWave);
	    if ( newFEBTstart < oldFEBTstart ) m_larFEBTstart->setTimeOffset(febid,newFEBTstart);
	    break;
	  case 2:
	    newFEBTstart = (unsigned)(larWaveHelper.getMax(*larPhysWave)-m_nIndexFromPeak);
	    if ( newFEBTstart < oldFEBTstart ) m_larFEBTstart->setTimeOffset(febid,newFEBTstart);
	    break;
	  case 3:
	    bindiff = larWaveHelper.getMax(*larPhysWave)-m_nIndexFromPeak;
            ATH_MSG_VERBOSE(std::hex << chid << std::dec<< " TimeOffset: "<<bindiff);
            if(bindiff >0.)
	      newFEBTstart = (unsigned)(bindiff);  
	    else
	      newFEBTstart = 0;
	    m_larFEBTstart->setTimeOffset(febid,oldFEBTstart+newFEBTstart); // accumulate offsets per FEB
	    theChanInFEB = static_cast<unsigned>(nChanInFEB.TimeOffset(febid)+1);
	    nChanInFEB.setTimeOffset(febid,theChanInFEB); // increment channel counter;
	    break;
	  default:
	    newFEBTstart = 0;
	    break;
	  }
	  
        }  // end loop over cells
       
     } // end of loop over gains
                
  } // end loop over all PhysWave containers in m_keylist

  if ( m_larFEBTstart->size()>0) {
    
    std::vector<HWIdentifier>::const_iterator it   = m_onlineHelper->feb_begin();
    std::vector<HWIdentifier>::const_iterator it_e = m_onlineHelper->feb_end();
    unsigned int nFeb=0;
    for (;it!=it_e;++it) {
      if ( (int)m_larFEBTstart->TimeOffset(*it) != 999 ) {
        nFeb++;
	if ( mode==3 && nChanInFEB.TimeOffset(*it) ) { // average time offset
	  float timeoff = m_larFEBTstart->TimeOffset(*it)/nChanInFEB.TimeOffset(*it);
	  m_larFEBTstart->setTimeOffset(*it,timeoff); 
	}
	ATH_MSG_INFO( nFeb << ". FEB ID 0x" << std::hex << (*it).get_compact() << std::dec 
            << " - Tstart = " <<  (int)m_larFEBTstart->TimeOffset(*it) ) ;

      } else {
        m_larFEBTstart->setTimeOffset(*it,0);
      }

    }
    
    if ( m_dumpTimeShiftByFEB ) {
      std::fstream outfile ;
      outfile.open("TimeShiftFEB.py",std::ios::out) ;
      outfile << "FakeLArTimeOffset.FEBids          = [ " ;
      it = m_onlineHelper->feb_begin();
      unsigned i=0; 
      for (;it!=it_e;++it) {
        outfile << "0x" << std::hex << (*it).get_compact() << std::dec ;
        i++ ;
        if ( i<nFeb ) outfile << ", " ; 
      }
      outfile << " ]" << std::endl ;
      outfile << "FakeLArTimeOffset.FEbTimeOffsets  = [ " ;
      it = m_onlineHelper->feb_begin();
      i = 0 ;
      for (;it!=it_e;++it) {
        outfile << (int)m_larFEBTstart->TimeOffset(*it) ; 
        i++ ;
        if ( i<nFeb ) outfile << ", " ; 
      }
      outfile << " ]" << std::endl ;
      outfile.close() ;
      ATH_MSG_INFO( "Minimum Tstart per FEB (all gain) saved in " << m_fileTimeShiftByFEB ) ;
    }
  
  } else {
    return StatusCode::FAILURE;
  }
  
  sc=detStore()->record(m_larFEBTstart,"FebTimeOffset");
  if(sc.isFailure()) {
     ATH_MSG_ERROR( "Can't record LArFEBTimeOffset to DetectorStore" );
     return StatusCode::FAILURE;
  }

  const ILArFEBTimeOffset* ilarFEBTimeOffset=nullptr;
  sc=detStore()->symLink(m_larFEBTstart,ilarFEBTimeOffset);
  if(sc.isFailure()) {
    ATH_MSG_ERROR( "Can't symlink LArFEBTimeOffset to abstract interface in  DetectorStore" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
