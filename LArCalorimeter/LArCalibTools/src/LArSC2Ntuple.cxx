/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArSC2Ntuple.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArRawEvent/LArSCDigit.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "TrigDecisionTool/ChainGroup.h"
#include "TrigDecisionTool/FeatureContainer.h"
#include "TrigDecisionTool/Feature.h"

LArSC2Ntuple::LArSC2Ntuple(const std::string& name, ISvcLocator* pSvcLocator):
  LArDigits2Ntuple(name, pSvcLocator) {
    m_ntTitle = "SCDigits";
    m_ntpath = "/NTUPLES/FILE1/SCDIGITS";
  }

StatusCode LArSC2Ntuple::initialize() {

  ATH_MSG_DEBUG( "LArSC2Ntuple in initialize" ); 

  m_isSC = true;
  if(m_fillTType && (! m_fillLB)) m_fillLB = true;
  if(m_fillCaloTT && (! m_fillLB)) m_fillLB = true;

  ATH_CHECK( LArDigits2Ntuple::initialize() );
  ATH_CHECK( m_scidtool.retrieve() );

  ATH_CHECK( m_cablingKeyAdditional.initialize(m_fillRawChan));
  ATH_CHECK( m_eventInfoKey.initialize() );
  ATH_CHECK( m_eventInfoDecorKey.initialize() );

  StatusCode sc=m_nt->addItem("latomeChannel",m_latomeChannel);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "addItem 'latomeChannel' failed" );
    return sc;
  }

  sc = m_nt->addItem("bcidVec",m_Nsamples, m_bcidVec);//here - > define length?
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "addItem 'bcidVec' failed" );
    return sc;
  }
  sc = m_nt->addItem("latomeSourceId",m_latomeSourceId);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "addItem 'latomeSourceId' failed" );
    return sc;
  }
  sc = m_nt->addItem("Net",m_ntNet);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "addItem 'Net' failed" );
    return sc;
  }
      
  // Loop over container keys
  for ( const std::string &ck : m_contKeys ){   
    if ( ck.find("SC")  == std::string::npos){	// main readout only
      if ( m_fillRawChan && ck == "LArRawChannels" ){
	sc = m_nt->addItem("ROD_energy", 16, m_ROD_energy);
	if (sc.isFailure()) {
	  ATH_MSG_ERROR( "addItem 'ROD_energy' failed" );
	  return sc;
	}
        sc = m_nt->addItem("ROD_time", 16, m_ROD_time);
        if (sc.isFailure()) {
          ATH_MSG_ERROR( "addItem 'ROD_time' failed" );
          return sc;
        }
        sc = m_nt->addItem("ROD_id", 16, m_ROD_id);
        if (sc.isFailure()) {
          ATH_MSG_ERROR( "addItem 'ROD_id' failed" );
          return sc;
        }
      }

    }else if ( ck == "SC_ADC_BAS" ){	// SC_ADC_BAS DigitContainer
      sc	   = m_nt->addItem("samples_ADC_BAS",m_Nsamples,m_samples_ADC_BAS);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'samples_ADC_BAS' failed" );
	return sc;
      }
 
      sc = m_nt->addItem("bcidVec_ADC_BAS",m_Nsamples, m_bcidVec_ADC_BAS);//here - > define length?
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'bcidVec_ADC_BAS' failed" );
	return sc;
      }
      
    }else if ( ck == "SC_LATOME_HEADER" ){	// SC LATOME HEADER
      sc	   = m_nt->addItem("bcidLATOMEHEAD",m_bcidLATOMEHEAD);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'bcidLATOMEHEAD' failed" );
	return sc;
      }
      
    }else if ( ck == "SC_ET" ){ // SC_ET RawSCContainer
      sc = m_nt->addItem("energyVec_ET", m_Net, m_energyVec_ET);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'energyVec_ET' failed" );
	return sc;
      }
      sc = m_nt->addItem("bcidVec_ET", m_Net, m_bcidVec_ET);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'bcidVec_ET' failed" );
	return sc;
      }
      sc = m_nt->addItem("saturVec_ET", m_Net, m_saturVec_ET);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'saturVec_ET' failed" );
	return sc;
      }
      
    }else if ( ck == "SC_ET_ID" ){	// SC_ET_ID RawSCContainer

      sc = m_nt->addItem("energyVec_ET_ID", m_Net, m_energyVec_ET_ID);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'energyVec_ET_ID' failed" );
	return sc;
      }
      sc = m_nt->addItem("bcidVec_ET_ID", m_Net, m_bcidVec_ET_ID);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'bcidVec_ET_ID' failed" );
	return sc;
      }
      sc = m_nt->addItem("saturVec_ET_ID", m_Net, m_saturVec_ET_ID);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'saturVec_ET_ID' failed" );
	return sc;
      }
    }
    
  }// end container key loop

  if(m_fillTType) {
     sc = m_evt_nt->addItem("TType",  m_TType);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'TType' failed" );
        return sc;
     } 

     sc = m_evt_nt->addItem("LArEventBits",  m_LArEventBits);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'LArEventBits' failed" );
        return sc;
     }
     sc = m_evt_nt->addItem("LArError",  m_LArInError);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'LArError' failed" );
        return sc;
     }
     //Adding trigger decision bit branches
     CHECK( m_trigDec.retrieve() );
     for ( const std::string &tn : m_trigNames ){
       sc = m_evt_nt->addItem(tn,m_trigNameMap[tn]);
       if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem  '"+tn+"' failed" );
         return sc;
       }
     }

  }//m_fillTType

  if(m_fillCaloTT) {
     sc = m_evt_nt->addItem("NTT",m_ntNTT,0,20000);
     if (sc.isFailure()) {
       ATH_MSG_ERROR( "addItem 'Net' failed" );
       return sc;
     }
     sc = m_evt_nt->addItem("TTeta",  m_ntNTT, m_TTeta);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'TTeta' failed" );
        return sc;
     }
     sc = m_evt_nt->addItem("TTphi",  m_ntNTT, m_TTphi);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'TTphi' failed" );
        return sc;
     }
     sc = m_evt_nt->addItem("TTEem", m_ntNTT, m_TTEem);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'TTEem' failed" );
        return sc;
     }
     sc = m_evt_nt->addItem("TTEhad", m_ntNTT, m_TTEhad);
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'TTEhad' failed" );
        return sc;
     }
  } // end m_fillCaloTT

  return StatusCode::SUCCESS;
  
}

StatusCode LArSC2Ntuple::execute()
{

  StatusCode	sc;
  
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<xAOD::EventInfo>evt (m_eventInfoKey, ctx);
  ATH_CHECK(evt.isValid());

  unsigned long long thisevent	  = evt->eventNumber();
  unsigned short thislb           = evt->lumiBlock();

  // This should be used for main readout later, once TDAQ fill event headers also in calib. runs properly
  unsigned long thisbcid	  = evt->bcid();
  unsigned long  thisttype = evt->level1TriggerType();
  //
  /// set it here once and no need to set at each SC/cell
  bool hasDigitContainer=true;
  const LArDigitContainer *DigitContainer   = nullptr;
  if(!m_contKey.key().empty()) {
     SG::ReadHandle<LArDigitContainer> hdlDigit(m_contKey, ctx);
     if(!hdlDigit.isValid()) {
       ATH_MSG_WARNING( "Unable to retrieve LArDigitContainer with key " << m_contKey << " from DetectorStore. " );
       hasDigitContainer=false;
     } else {
       ATH_MSG_DEBUG( "Got LArDigitContainer with key " << m_contKey.key() );
       DigitContainer   = hdlDigit.cptr();
     }
  } else hasDigitContainer=false;   

  const LArDigitContainer*	DigitContainer_next	   = nullptr;
  const LArRawSCContainer*	etcontainer	   = nullptr;
  const LArRawSCContainer*	etcontainer_next	   = nullptr;
  const LArRawChannelContainer*	RawChannelContainer   = nullptr;
  const LArLATOMEHeaderContainer*headcontainer	   = nullptr;
  std::map<unsigned int, const LArLATOMEHeader*> LATOMEHeadMap;
  rawChanMap_t 	rawChannelMap; 

  if ((std::find(m_contKeys.begin(), m_contKeys.end(), "LArRawChannels")	  != m_contKeys.end()) ){
    sc	   = evtStore()->retrieve(RawChannelContainer,"LArRawChannels");  
    if (sc.isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve LArRawChannelContainer with key LArRawChannels from DetectorStore. " );
    } 
    else
      ATH_MSG_DEBUG( "Got LArRawChannelContainer with key LArRawChannels" );
  }

  if ((std::find(m_contKeys.begin(), m_contKeys.end(), "SC_ADC_BAS")  != m_contKeys.end()) ){
    sc	   = evtStore()->retrieve(DigitContainer_next,"SC_ADC_BAS");  
    if (sc.isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve LArDigitContainer with key SC_ADC_BAS from DetectorStore. " );
    } 
    else 
      ATH_MSG_DEBUG( "Got additional LArDigitContainer with key SC_ADC_BAS " );
  }
  
  if ((std::find(m_contKeys.begin(), m_contKeys.end(), "SC_ET")  != m_contKeys.end()) ){
    sc	   = evtStore()->retrieve(etcontainer,"SC_ET");  
    if (sc.isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key SC_ET from DetectorStore. " );
    } 
    else
      ATH_MSG_DEBUG( "Got LArRawSCContainer with key SC_ET " );
  }
  
  if ((std::find(m_contKeys.begin(), m_contKeys.end(), "SC_ET_ID")  != m_contKeys.end()) ){
    sc	   = evtStore()->retrieve(etcontainer_next,"SC_ET_ID");  
    if (sc.isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key SC_ET_ID from DetectorStore. " );
    } 
    else
      ATH_MSG_DEBUG( "Got LArRawSCContainer with key SC_ET_ID" );
  }
  
  if ((std::find(m_contKeys.begin(), m_contKeys.end(), "SC_LATOME_HEADER")	  != m_contKeys.end()) ){
    sc	   = evtStore()->retrieve(headcontainer,"SC_LATOME_HEADER");  
    if (sc.isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve LArLATOMEHeaderContainer with key SC_LATOME_HEADER from DetectorStore. " );
    } 
    else
      ATH_MSG_DEBUG( "Got LArLATOMEHeaderContainer with key SC_LATOME_HEADER " ); 
  }
  
  if (headcontainer){// loop through header container and fill map
    for (const LArLATOMEHeader* hit : *headcontainer) {
      LATOMEHeadMap.try_emplace ( hit->SourceId(), hit );
    }
  }
  if(m_fillRawChan && RawChannelContainer){
    for (const LArRawChannel& raw : *RawChannelContainer) {
       rawChannelMap.try_emplace( raw.channelID(), &raw );
    }
  }
  const LArOnOffIdMapping* cabling=nullptr;
  const LArOnOffIdMapping* cablingROD=nullptr;
  if(m_fillRawChan){
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{cablingKey()};
     cabling=*cablingHdl;
     if(!cabling) {
        ATH_MSG_ERROR( "Do not have cabling for SC!" );
        return StatusCode::FAILURE;
     }
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdlROD{m_cablingKeyAdditional};
     cablingROD=*cablingHdlROD;
     if(!cablingROD) {
        ATH_MSG_ERROR( "Do not have cabling for ROD!" );
        return StatusCode::FAILURE;
     }
  }

  /// set container pointers to nullptr if size is 0 (avoid checking again the size in many places)
  if( DigitContainer_next && DigitContainer_next->size()  == 0 ) DigitContainer_next = nullptr;

  if( etcontainer && etcontainer->size()	  == 0 ) etcontainer = nullptr;

  if( etcontainer_next && etcontainer_next->size()	  == 0 ) etcontainer_next = nullptr;
  
  int cellsno   = 0;
  if (hasDigitContainer) {
     if( !DigitContainer->empty() ) cellsno = DigitContainer->size();
     else {
       ATH_MSG_WARNING("DigitContainer has zero size, but asked, will be not filled... ");
       return StatusCode::SUCCESS;
     }
  }
  ATH_MSG_DEBUG("DigitContainer has size: "<<cellsno<<" hasDigitContainer: "<<hasDigitContainer);

  if (DigitContainer_next){
    if ( cellsno == 0 ){ 
      cellsno	   = DigitContainer_next->size();
    }else	if(DigitContainer_next->size()	  != (unsigned)cellsno ){ ATH_MSG_ERROR(" NOOOOOOO! Different number of entries in DigitContainer_next"<< cellsno << " " << DigitContainer_next->size() );
    }
  }
  if (etcontainer){
    if ( cellsno == 0 ){ 
      cellsno	   = etcontainer->size();
    }else	if(etcontainer->size()  != (unsigned)cellsno ){ ATH_MSG_ERROR(" NOOOOOOO! Different number of entries in etcontainer"<< cellsno << " " << etcontainer->size() );
    }
  }
  if (etcontainer_next){
    if ( cellsno == 0 ){ 
      cellsno	   = etcontainer_next->size();
    }else	if(etcontainer_next->size()  != (unsigned)cellsno ){ ATH_MSG_ERROR(" NOOOOOOO! Different number of entries in etcontainer_next"<< cellsno << " " << etcontainer_next->size() );
    }
  }
  unsigned	cellCounter	   = 0;
  ATH_MSG_DEBUG("cellsno size: "<<cellsno);
  for( int c    = 0;c<cellsno;++c ){
    if(m_fillBCID) m_bcid	   = thisbcid; 

    m_IEvent	   = thisevent;
    if(m_overwriteEventNumber) m_IEvent   = ctx.evt();

    if( hasDigitContainer ){

      const LArDigit* digi   = DigitContainer->at(c);     
      // ======================

      if(m_FTlist.size() > 0) {	// should do a selection
	if(std::find(std::begin(m_FTlist), std::end(m_FTlist), m_FT)  == std::end(m_FTlist)) {	// is our FT in list ?
	  continue;
	}
      }

      unsigned int trueMaxSample	   = digi->nsamples();

      if(trueMaxSample>m_Nsamples){
	if(!m_ipass){
	  ATH_MSG_WARNING( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
	  m_ipass   = 1;
	}
	trueMaxSample   = m_Nsamples;
      }
      m_ntNsamples   = trueMaxSample;

      fillFromIdentifier(digi->hardwareID());      

      for(unsigned i =	0; i<trueMaxSample;++i) m_samples[i]	   = digi->samples().at(i);

      const LArSCDigit*	scdigi   = dynamic_cast<const LArSCDigit*>(digi);
      if(!scdigi){ 
         ATH_MSG_DEBUG(" Can't cast digi to LArSCDigit*");
      }else{
	  if (headcontainer){
            ATH_MSG_DEBUG(" Accessing LATOME header ");
	    const LArLATOMEHeader*headmap   = LATOMEHeadMap[scdigi->SourceId()];
	    if(headmap){
	      m_bcidLATOMEHEAD   = headmap->BCId();
	    }
	  }   
	  m_latomeChannel	   = scdigi->Channel();
          unsigned int trueMaxBcid = trueMaxSample;
          if(trueMaxBcid > scdigi->BCId().size()) trueMaxBcid=scdigi->BCId().size();
	  for( unsigned i = 0; i<trueMaxBcid; ++i){
	     m_bcidVec[i]	   = scdigi->BCId().at(i);
	  }	 
	  m_latomeSourceId	   = scdigi->SourceId();
      }
    

      if( m_fillRawChan && RawChannelContainer ){
	fillRODEnergy(digi->hardwareID(), rawChannelMap, cabling, cablingROD);
      }
    }//hasDigitContainer
    ATH_MSG_DEBUG("After hasDigitContainer ");
    

    // DigitContainer 1 -> SC_ADC_BAS
    if( DigitContainer_next ){
      
      const LArDigit* digi = DigitContainer_next->at(c);

      unsigned int trueMaxSample = digi->nsamples();
    
      if(trueMaxSample>m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
          m_ipass=1;
        }
        trueMaxSample = m_Nsamples;
      }
      m_ntNsamples = trueMaxSample;

      if( !hasDigitContainer){ //// already filled in DigitContainer
        fillFromIdentifier(digi->hardwareID());
        if( m_fillRawChan && RawChannelContainer ){
	   fillRODEnergy(digi->hardwareID(), rawChannelMap, cabling, cablingROD);
        }
      }
         
     for(unsigned i =	0; i<trueMaxSample;++i) m_samples_ADC_BAS[i]   = digi->samples().at(i);

     const LArSCDigit*	scdigi   = dynamic_cast<const LArSCDigit*>(digi);
     if(!scdigi){ ATH_MSG_DEBUG(" Can't cast digi to LArSCDigit*");
      }else{
	  if (headcontainer){
	    const LArLATOMEHeader*headmap   = LATOMEHeadMap[scdigi->SourceId()];
	    if(headmap){
	      m_bcidLATOMEHEAD   = headmap->BCId();
	    }
	  }   
	  m_latomeChannel	   = scdigi->Channel();
	  for( unsigned i = 0; i<trueMaxSample;++i){
	    m_bcidVec[i]	   = scdigi->BCId().at(i);
	  }	 
	  m_latomeSourceId	   = scdigi->SourceId();
      }

      if( !hasDigitContainer && m_fillRawChan && RawChannelContainer ){
        fillRODEnergy(digi->hardwareID(), rawChannelMap, cabling, cablingROD);
      }
    }
    ATH_MSG_DEBUG("After DigitContainer_next ");
    

    // DigitContainer 1 -> SC_ADC_BAS
    if( DigitContainer_next ){
      
      const LArDigit* digi = DigitContainer_next->at(c);

      unsigned int trueMaxSample = digi->nsamples();
    
      if(trueMaxSample>m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
          m_ipass=1;
        }
        trueMaxSample = m_Nsamples;
      }
      m_ntNsamples = trueMaxSample;
      ATH_MSG_DEBUG("m_ntNsamples: "<<m_ntNsamples);

      if( !hasDigitContainer){ //// already filled in DigitContainer?
        fillFromIdentifier(digi->hardwareID());
        if( m_fillRawChan && RawChannelContainer ){
	   fillRODEnergy(digi->hardwareID(), rawChannelMap, cabling, cablingROD);
        }
      }
         
     for(unsigned i =	0; i<trueMaxSample;++i) m_samples_ADC_BAS[i]   = digi->samples().at(i);

     const LArSCDigit*	scdigi   = dynamic_cast<const LArSCDigit*>(digi);
     if(!scdigi){ 
        ATH_MSG_DEBUG(" Can't cast digi to LArSCDigit*");
     }else{
       if ( !hasDigitContainer){
         if (headcontainer){
           const LArLATOMEHeader*headmap   = LATOMEHeadMap[scdigi->SourceId()];
           if(headmap){
             m_bcidLATOMEHEAD	   = headmap->BCId();
           }
         }
         m_latomeChannel	   = scdigi->Channel();
         m_latomeSourceId	   = scdigi->SourceId();
       }

       for( unsigned i = 0; i<scdigi->BCId().size();++i){
         m_bcidVec_ADC_BAS[i]	   = scdigi->BCId().at(i);
       }
     }
    }//DigitContainer_next

    // etcontainer -> SC_ET
    if( etcontainer ){
      const LArRawSC*rawSC   = etcontainer->at(c);
       
      if ( !hasDigitContainer && !DigitContainer_next ){
        fillFromIdentifier(rawSC->hardwareID());
	m_latomeChannel	   = rawSC->chan();
	if (headcontainer){
	  const LArLATOMEHeader*headmap   = LATOMEHeadMap[rawSC->SourceId()];
	  if(headmap){
	    m_bcidLATOMEHEAD	   = headmap->BCId();
	  }
	}
        if( m_fillRawChan && RawChannelContainer ){
	   fillRODEnergy(rawSC->hardwareID(), rawChannelMap, cabling, cablingROD);
        }
      }
      unsigned int truenet = m_Net;
      if(truenet > rawSC->bcids().size()) truenet=rawSC->bcids().size();
      for( unsigned i=0; i<truenet;++i){	// just use the vector directly?
	m_bcidVec_ET[i]	   = rawSC->bcids().at(i);
      }
      if(truenet > rawSC->energies().size()) truenet=rawSC->energies().size();
      for( unsigned i=0; i<truenet;++i){	// just use the vector directly?
	m_energyVec_ET[i]	   = rawSC->energies().at(i);
      }
      if(truenet > rawSC->satur().size()) truenet=rawSC->satur().size();
      for( unsigned i = 0; i<truenet;++i){	// just use the vector directly?
	m_saturVec_ET[i]	   = rawSC->satur().at(i);
      }
      m_Net=truenet;
      m_ntNet=truenet;

    }
    // etcontainer_next -> SC_ET_ID
    if( etcontainer_next ){
      const LArRawSC*rawSC   = etcontainer_next->at(c);

      if ( !hasDigitContainer && !DigitContainer_next && !etcontainer ){
        fillFromIdentifier(rawSC->hardwareID());
	m_latomeChannel	   = rawSC->chan();
	if (headcontainer){
	  const LArLATOMEHeader*headmap   = LATOMEHeadMap[rawSC->SourceId()];
	  if(headmap){
	    m_bcidLATOMEHEAD	   = headmap->BCId();
	  }
	}
        if( m_fillRawChan && RawChannelContainer ){
	   fillRODEnergy(rawSC->hardwareID(), rawChannelMap, cabling, cablingROD);
        }
      }
      for( unsigned i=0; i<rawSC->bcids().size();++i){	// just use the vector directly?
	m_bcidVec_ET_ID[i]	   = rawSC->bcids()[i];
      }
      for( unsigned i=0; i<rawSC->energies().size();++i){	// just use the vector directly?
	m_energyVec_ET_ID[i]	   = rawSC->energies()[i];
      }
      for( unsigned i = 0; i<rawSC->satur().size();++i){	// just use the vector directly?
	m_saturVec_ET_ID[i]	   = rawSC->satur()[i];
      }
    }

    sc   = ntupleSvc()->writeRecord(m_nt);
    if (sc != StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "writeRecord failed" );
      return sc;
    }
    cellCounter++;
  }// over cells 
  if(m_fillTType) {
     m_TType = thisttype;
     m_IEventEvt   = thisevent;
     if(m_overwriteEventNumber) m_IEventEvt   = m_event;
     m_LB = thislb;

     for (auto const & x : m_trigNames) {
       if( ! m_trigDec->getListOfTriggers(x).empty() ){
         m_trigNameMap[x] = m_trigDec->isPassedBits( x );
       }
     }
     m_LArEventBits = evt->eventFlags(xAOD::EventInfo::LAr);
     m_LArInError   = 0;
     if(evt->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error) m_LArInError = m_LArInError | 0x1; 
     if(evt->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Warning) m_LArInError = m_LArInError | 0x2; 

  }
  if(m_fillCaloTT){
    const DataVector<LVL1::TriggerTower>* TTVector;
    if ( evtStore()->retrieve(TTVector,m_triggerTowerKey).isFailure() ) {
       ATH_MSG_WARNING("Could not get the Calo TTs, will not fill...");
    } else {
      unsigned count=0; 
      ATH_MSG_INFO("Got TT vector of the sixe " <<  TTVector->size());
      DataVector<LVL1::TriggerTower>::const_iterator x; 
      for ( x = TTVector->begin(); x < TTVector->end(); ++x ){
           m_TTeta[count]=(*x)->eta();
           m_TTphi[count]=(*x)->phi();
           m_TTEem[count]=(*x)->emEnergy();
           m_TTEhad[count]=(*x)->hadEnergy();
           ++count;
           if(count==20000) break;
      }
      m_ntNTT=count;
    }
  }

  if(m_fillTType || m_fillCaloTT){
     sc   = ntupleSvc()->writeRecord(m_evt_nt);
     if (sc != StatusCode::SUCCESS) {
          ATH_MSG_ERROR( "writeRecord failed" );
          return sc;
     }
  }

  ATH_MSG_DEBUG( "LArSC2Ntuple has finished, filled " << cellCounter << " cells");
  return StatusCode::SUCCESS;
}// end finalize-method.

void LArSC2Ntuple::fillRODEnergy(HWIdentifier SCId, rawChanMap_t &rawChanMap, const LArOnOffIdMapping* cabling, const LArOnOffIdMapping* cablingROD)
{
 const Identifier offId = cabling->cnvToIdentifier(SCId);
 const std::vector<Identifier> cellIds = m_scidtool->superCellToOfflineID(offId);
 std::fill(m_ROD_energy.begin(), m_ROD_energy.end(), 0.);
 std::fill(m_ROD_time.begin(), m_ROD_time.end(), 0.);
 std::fill(m_ROD_id.begin(), m_ROD_id.end(), 0.);
 for(unsigned i=0; i<cellIds.size(); ++i ) {
    const HWIdentifier hwcell=cablingROD->createSignalChannelID(cellIds[i]);
    if (hwcell.is_valid()  && (rawChanMap.count(hwcell) != 0) ) {
       m_ROD_energy[i] = rawChanMap[hwcell]->energy();
       m_ROD_time[i] = rawChanMap[hwcell]->time();
       m_ROD_id[i] = rawChanMap[hwcell]->hardwareID().get_identifier32().get_compact();
    } else {
       ATH_MSG_WARNING(i<<"-th cell invalid Id");
    }
 }

}
