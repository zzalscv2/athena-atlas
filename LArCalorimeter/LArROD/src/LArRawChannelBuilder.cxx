/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawChannelBuilder.h"

#include "LArRawEvent/LArDigitContainer.h"

#include "LArElecCalib/ILArPedestal.h"
//#include "LArElecCalib/ILArRamp.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArShape.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <cmath>

using CLHEP::MeV;
using CLHEP::megahertz;
using CLHEP::nanosecond;
using CLHEP::picosecond;

LArRawChannelBuilder::LArRawChannelBuilder (const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator),
  m_onlineHelper(nullptr),
  m_Ecut(256*MeV),
  m_initialTimeSampleShift(0),
  m_ramp_max(),
  m_noEnergy(0),
  m_noTime(0),
  m_noShape(0),
  m_noShapeDer(0),
  m_saturation(0),
  m_lastNoEnergy(0),
  m_lastNoTime(0),
  m_lastNoShape(0),
  m_lastNoShapeDer(0),
  m_aveNoEnergy(0),
  m_aveNoTime(0),
  m_aveNoShape(0),
  m_aveNoShapeDer(0),
  m_aveSaturCells(0),
  m_nEvents(0),
  m_aveChannels(0),
  m_SamplingPeriodeUpperLimit(0),
  m_SamplingPeriodeLowerLimit(0),
  m_emId(nullptr),
  m_firstSample(0)
  , m_pedestalKey("LArPedestal")
  , m_shapesKey("LArShape")
 {
   //m_useIntercept={false,false,false,false};
 declareProperty("Ecut",                      m_Ecut);
 declareProperty("UseHighGainRampIntercept",  m_useIntercept[CaloGain::LARHIGHGAIN]=false);
 declareProperty("UseMedGainRampIntercept",   m_useIntercept[CaloGain::LARMEDIUMGAIN]=false);
 declareProperty("UseLowGainRampIntercept",   m_useIntercept[CaloGain::LARLOWGAIN]=false);
 declareProperty("InitialTimeSampleShift",    m_initialTimeSampleShift);
 declareProperty("NOFCTimeBins",              m_NOFCTimeBins=25); //Number of OFC time bins in a sampling periode
 m_NOFCPhases = m_NOFCTimeBins; 
 declareProperty("NOFCPhases",                m_NOFCPhases);      //Total number of available OFC sets
 declareProperty("UseOFCPhase",               m_useOFCPhase=false);
 declareProperty("PhaseInversion",            m_phaseInv=false);
 declareProperty("SamplingPeriod",            m_SamplingPeriode=1/(40.08*megahertz));
 declareProperty("OFCTimeBin",                m_OFCTimeBin=m_SamplingPeriode/m_NOFCTimeBins);
 declareProperty("BinHalfOffset",             m_binHalfOffset=false);
 declareProperty("AllowTimeSampleJump",       m_allowTimeJump=true);
 declareProperty("PedestalFallbackMode",      m_pedestalFallbackMode=0); // 0=only DB, 1=Only if missing,
 declareProperty("PedestalSample",            m_iPedestal=0);            // 2=Low, 3=Low+Me dium, 4=All LAr
 declareProperty("ShapeMode",                 m_shapeMode=0); 
 declareProperty("SkipSaturCellsMode",        m_skipSaturCells=0);
 declareProperty("ADCMax",                    m_AdcMax=4095);
 declareProperty("firstSample",               m_firstSample,"  first sample used in shape");
 declareProperty("PedestalKey",		      m_pedestalKey);
 declareProperty("ShapesKey",		      m_shapesKey);
}


StatusCode LArRawChannelBuilder::initialize()
{
  ATH_CHECK( detStore()->retrieve(m_onlineHelper, "LArOnlineID") );

  ATH_CHECK( m_ofcKey.initialize() );
  ATH_CHECK( m_adc2mevKey.initialize() );
  
  // ***
  
  const CaloCell_ID* idHelper = nullptr;
  ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_ID") );
  m_emId=idHelper->em_idHelper();
  if (!m_emId) {
    ATH_MSG_ERROR( "Could not get lar EM ID helper"  );
    return StatusCode::FAILURE;
  }
  
  ATH_CHECK( m_cablingKey.initialize() );


  //Set counters for errors and warnings to zero
  m_noEnergy   = 0; // Number of events with at least completly failed channel
  m_noTime     = 0; // Number of events with at least one channel without time info
  m_noShape    = 0; // Number of events with at least one channel without Shape (=with not quality factor);
  m_noShapeDer = 0; // Number of events with at least one channel without ShapeDerivative (=not taken into accout for quality factor);
  m_saturation = 0; // Number of events with at least one saturating channel
  
  m_lastNoEnergy   = -1; // Number of completly failed channels in previous event
  m_lastNoTime     = -1; // Number of channels without time info in previous event
  m_lastNoShape    = -1; // Number of channels without Shape (=with not quality factor) in previous event
  m_lastNoShapeDer = -1; // Number of channels without ShapeDerivative in previous event
  
  //m_lastSaturCells = -1; // Number of saturating channels without in previous event (not used)
  
  m_aveNoEnergy    = 0.; // Average number of completly failed channels per event
  m_aveNoTime      = 0.; // Average number of channels without time info per event
  m_aveNoShape     = 0.; // Average number of channels without Shape (=with not quality factor) per event
  m_aveNoShapeDer  = 0.; // Average number of channels without ShapeDerivative per event
  m_aveSaturCells  = 0.; // Average number of saturating channels without per event
  
  m_nEvents        = 0 ; // Total number of processed events ;
  m_aveChannels    = 0 ; // Average number of readout channels per event

  if ( m_skipSaturCells > 2 ) m_skipSaturCells = 0 ;

  m_ramp_max[CaloGain::LARHIGHGAIN]   = 500.;
  m_ramp_max[CaloGain::LARMEDIUMGAIN] = 5000.;
  m_ramp_max[CaloGain::LARLOWGAIN]    = 50000.;

  // Validity range for a set of OFC's. If the time shift is larger than this number,
  // we make a ADC sample jump (e.g. from [0,5] to [1,6]. The second half of the uppermost 
  // bin should already be rounded to the 0th bin of the following ADC sample.  
  if ( m_binHalfOffset ) {
    m_SamplingPeriodeUpperLimit = m_SamplingPeriode-m_SamplingPeriode/(2*m_NOFCTimeBins);
    m_SamplingPeriodeLowerLimit = -m_SamplingPeriode/(2*m_NOFCTimeBins);
  } else {
    m_SamplingPeriodeUpperLimit = m_SamplingPeriode;
    m_SamplingPeriodeLowerLimit = 0;
  }

  ATH_MSG_DEBUG( "Number of OFC time bins per sampling periode=" << m_NOFCTimeBins  );
  ATH_MSG_DEBUG( "Sampling Periode=" << m_SamplingPeriode << "ns"  );
  ATH_MSG_DEBUG( "Sampling Periode Limits: (" << m_SamplingPeriodeLowerLimit
                 << "," << m_SamplingPeriodeUpperLimit << ") ns"  );

  ATH_CHECK( m_dataLocation.initialize() );
  ATH_CHECK( m_ChannelContainerName.initialize() );

  return StatusCode::SUCCESS;
}



StatusCode LArRawChannelBuilder::execute()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  //Counters for errors & warnings per event
  int noEnergy   = 0; // Number of completly failed channels in a given event
  int BadTiming  = 0; // Number of channels with bad timing in a given event
  int noTime     = 0; // Number of channels without time info in a given event
  int noShape    = 0; // Number of channels without Shape (= with no quality factor) in a given event 
  int noShapeDer = 0; // Number of channels without ShapeDerivative in a given event 
  int highE      = 0; // Number of channels with 'high' (above threshold) energy in a given event 
  int saturation = 0; // Number of saturating channels in a given event   
  
  //Pointer to conditions data objects 
  const ILArPedestal* larPedestal=nullptr;
  const ILArShape* larShape=nullptr;
  //Retrieve Digit Container

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
  const LArOnOffIdMapping* cabling{*cablingHdl};
  if(!cabling) {
     ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKey.key() );
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArADC2MeV> adc2mev (m_adc2mevKey, ctx);

  //Pointer to input data container
  SG::ReadHandle<LArDigitContainer> digitContainer (m_dataLocation, ctx);
  
  //Retrieve calibration data
  ATH_CHECK( detStore()->retrieve(larPedestal,m_pedestalKey) );
  ATH_CHECK( detStore()->retrieve(larShape,m_shapesKey) );

  ATH_MSG_DEBUG( "Retrieving LArOFC object"  );
  SG::ReadCondHandle<ILArOFC> larOFC (m_ofcKey, ctx);

  auto larRawChannelContainer = std::make_unique<LArRawChannelContainer>();
  larRawChannelContainer->reserve(digitContainer->size());

  // Average number of LArDigits per event
  m_nEvents++;
  m_aveChannels += digitContainer->size();

  // Now all data is available, start loop over Digit Container
  int ntot_raw=0;

  for (const LArDigit* digit : *digitContainer) {

    //Data that goes into RawChannel:
    float energy=0;
    float time=0;
    float quality=0;

    int OFCTimeBin=0;
    int timeSampleShift=m_initialTimeSampleShift;

    //Get data from LArDigit
    const std::vector<short>& samples=digit->samples();
    const unsigned nSamples=samples.size(); 
    const HWIdentifier chid=digit->channelID();
    const CaloGain::CaloGain gain=digit->gain();
    
    // to be used in case of DEBUG output
    int layer  = -99999 ;
    int eta    = -99999 ; 
    int phi    = -99999 ; 
    int region = -99999 ;    
    if (msgLvl (MSG::DEBUG)) {
      Identifier id ;
      try {
        id = cabling->cnvToIdentifier(chid);
      } catch ( LArID_Exception & except ) {
        ATH_MSG_DEBUG( "A Cabling exception was caught for channel 0x!" 
                       << MSG::hex << chid.get_compact() << MSG::dec  );
        continue ;
      }
      layer  = m_emId->sampling(id);
      eta    = m_emId->eta(id); 
      phi    = m_emId->phi(id);
      region = m_emId->region(id);    
      //log << MSG::VERBOSE << "Channex 0x" << MSG::hex << chid.get_compact() << MSG::dec 
      //                    << " [ Layer = " << layer << " - Eta = " << eta 
      //			  << " - Phi = " << phi << " - Region = " << region << " ] " << endmsg ;
    }
        
    // check for saturation, in case skip channel
    int nSatur=-1 ;
    for (unsigned iSample=0;iSample<samples.size();iSample++) {
      if (samples[iSample]>=m_AdcMax) {
        nSatur++;
        break ;
      }
    }
    if ( nSatur>-1 ) {
      if (msgLvl (MSG::DEBUG))msg() << MSG::DEBUG << "Saturation on channel 0x" << MSG::hex << chid.get_compact() << MSG::dec ;         
      saturation++;
    }
    if ( m_skipSaturCells && nSatur>-1 ) {
      msg() << ". Skipping channel." << endmsg; 
      continue; // Ignore this cell, saturation on at least one sample
    } else if ( nSatur>-1 ) {
      msg() << "." << endmsg;
    }   
    
    //Get conditions data for this channel:

    // Pedestal
    float pedestal=larPedestal->pedestal(chid,gain);
    float pedestalAverage;
    if (pedestal <= (1.0+LArElecCalib::ERRORCODE)) {
      if( m_pedestalFallbackMode >= 1 ) {
        ATH_MSG_DEBUG( "No pedestal found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                       << " Gain " << gain <<".  Using time sample " << m_iPedestal  );
        pedestalAverage=samples[m_iPedestal];
      } else {              
        ATH_MSG_DEBUG( noEnergy << ". No pedestal found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                       << " [ Layer = " << layer << " - Eta = " << eta << " - Phi = " << phi << " - Region = " << region << " ]"
                       << " Gain = " << gain << ". Skipping channel."  );
	noEnergy++;
        continue;
      }
    } else {
      if( ( m_pedestalFallbackMode>=2 && gain==CaloGain::LARLOWGAIN )   ||
          ( m_pedestalFallbackMode>=3 && gain==CaloGain::LARMEDIUMGAIN ) ||
          ( m_pedestalFallbackMode>=4 && gain==CaloGain::LARHIGHGAIN )       ) {
        ATH_MSG_DEBUG( "Forcing pedestal fallback for  channel 0x" << MSG::hex << chid.get_compact()
                       << MSG::dec   << " Gain=" << gain << ". Using time sample " << m_iPedestal  );
        pedestalAverage=samples[m_iPedestal];
      } else {
        //Assume there is only one pedestal value, even the ILArPedestal object returns a vector<float>
        pedestalAverage=pedestal;
      }      
    }

    // Optimal Filtering Coefficients
    ILArOFC::OFCRef_t ofc_a;
    ILArOFC::OFCRef_t ofc_b;
    {// get OFC from Conditions Store
      double timeShift=0;
      if (m_useOFCPhase) {
	const double ofcTimeOffset=larOFC->timeOffset(chid,gain);
	timeShift+=ofcTimeOffset;
	if (msgLvl (MSG::VERBOSE)) msg() << MSG::VERBOSE << " OFC=" << ofcTimeOffset;
      }

      if (msgLvl (MSG::VERBOSE)) msg() << MSG::VERBOSE << " Total=" << timeShift << endmsg;
      
      if (m_allowTimeJump && timeShift >= m_NOFCPhases*m_OFCTimeBin ) {
	if (msgLvl (MSG::VERBOSE)) msg() << MSG::VERBOSE << "Time Sample jump: +1" << endmsg;
	timeSampleShift += 1;
	//timeShift       -= m_NOFCTimeBins*m_OFCTimeBin ;
	timeShift       -= m_SamplingPeriode ;
      }
      else if (m_allowTimeJump && timeShift < 0 ) {
	ATH_MSG_VERBOSE( "Time Sample jump: -1"  );
	timeSampleShift -= 1;
	//timeShift       += m_NOFCTimeBins*m_OFCTimeBin ;
        timeShift       += m_SamplingPeriode ;
      }

      if (m_allowTimeJump && ( timeShift > m_NOFCPhases*m_OFCTimeBin || timeShift < 0 ) ) {
	BadTiming++;
	noEnergy++;
	ATH_MSG_ERROR( noEnergy << ". Time offset out of range for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                       << " Found " << timeShift << ",  expected ( 0 - " << m_NOFCPhases*m_OFCTimeBin << ") ns. Skipping channel."  );
	continue;
      }
      
      if (m_allowTimeJump && timeSampleShift < 0) {
	BadTiming++;
	noEnergy++;
	ATH_MSG_ERROR( noEnergy << ". Negative time sample (" << timeSampleShift << ") shift for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                       << " Found. Skipping channel."  );
	continue;
      }

      OFCTimeBin = (int) ( timeShift / m_OFCTimeBin );
      
      if ( !m_phaseInv ) // if not done with PhaseTime at the beginning, invert time bin for OFC bin selection
         OFCTimeBin = ( m_NOFCTimeBins - 1 ) - OFCTimeBin;
      // do not use the following: 24<PhaseTime<25 you always get OFCTimeBin = -1!
      //else 
      //   OFCTimeBin -= 1 ; 
      
      ATH_MSG_VERBOSE( "OFC bin width = " << m_OFCTimeBin << " - OFCBin = " << OFCTimeBin << " - timeShift = " << timeShift  );
      
      if ( OFCTimeBin < 0 ) {
        ATH_MSG_ERROR( "Channel " << MSG::hex << chid.get_compact() << MSG::dec << " asks for OFC bin = " << OFCTimeBin << ". Set to 0."  );
        OFCTimeBin=0;
      } else if ( OFCTimeBin >= m_NOFCPhases ) {
        ATH_MSG_ERROR( "Channel " << MSG::hex << chid.get_compact() << MSG::dec << " asks for OFC bin = " << OFCTimeBin << ". Set to (NOFCPhases-1) =" << m_NOFCTimeBins-1  );
        OFCTimeBin = m_NOFCPhases-1;
      }

      ofc_a=larOFC->OFC_a(chid,gain,OFCTimeBin);
      //ofc_b=&(larOFC->OFC_b(chid,gain,OFCTimeBin)); retrieve only when needed
    }
    
    //Check if we have OFC for this channel and time bin
    if (ofc_a.size()==0) {
      noEnergy++;
      ATH_MSG_DEBUG( noEnergy << ". No OFC's found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                     << " [ Layer = " << layer << " - Eta = " << eta << " - Phi = " << phi << " - Region = " << region << " ]"
                     << " Time bin = " << OFCTimeBin << ", Gain = " << gain << ". Skipping channel."  );
      continue;
    } 
    if (ofc_a.size()+timeSampleShift>nSamples) {
      BadTiming++;
      noEnergy++;
      if (msgLvl (MSG::DEBUG)) {
        if (timeSampleShift==0)
	  ATH_MSG_DEBUG( "Found LArDigit with " << nSamples << " samples, but OFCs for " 
                         << ofc_a.size() << " samples. Skipping Channel " );
        else //have time sample shift
	  ATH_MSG_DEBUG( "After time sample shift of " << timeSampleShift << ", " << nSamples-timeSampleShift
                         << " samples left, but have OFCs for " << ofc_a.size() << " samples. Skipping Channel " );
      }
      continue;
    } 

    //Now apply Optimal Filtering to get ADC peak
    float ADCPeak=0;
    for (unsigned i=0;i<(ofc_a.size());i++) 
      ADCPeak+=(samples[i+timeSampleShift]-pedestalAverage)*ofc_a.at(i);
    
    ATH_MSG_VERBOSE( "ADC Height calculated " << ADCPeak << " TimeBin=" << OFCTimeBin   );
     
    //ADC2MeV (a.k.a. Ramp)   
    LArVectorProxy ramp = adc2mev->ADC2MEV(chid,gain);
    //Check ramp coefficents
    if (ramp.size()==0) {
      noEnergy++;
      ATH_MSG_DEBUG( noEnergy << ". No ADC2MeV data found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec
                     << " [ Layer = " << layer << " - Eta = " << eta << " - Phi = " << phi << " - Region = " << region << " ]"
                     << " Gain = " << gain << ". Skipping channel."  );
      continue;
    } 

    // temporary fix for bad ramps... should be done in the DB
    // if(ramp[1]>500 || ramp[1]<0) {

    if(ramp[1]>m_ramp_max[gain] || ramp[1]<0) {
      noEnergy++;
      ATH_MSG_DEBUG( "Bad ramp for channel " << chid << " (ramp[1] = " << ramp[1] << "): skip this channel"  );
      continue;
    } 

    float ADCPeakPower=ADCPeak;

    if (m_useIntercept[gain])
      energy=ramp[0];
    //otherwise ignore intercept, E=0;
    for (unsigned i=1;i<ramp.size();i++)
      {energy+=ramp[i]*ADCPeakPower; //pow(ADCPeak,i);
       //std::cout << "Step "<< i <<":" << ramp[i] << " * " << pow(ADCPeak,i) << "Sum=" << energy << std::endl;
       ADCPeakPower*=ADCPeak;
      }

    //Check if energy is above threshold for time & quality calculation
    if (energy>m_Ecut) {
      highE++;
      ofc_b=larOFC->OFC_b(chid,gain,OFCTimeBin);
      if (ofc_b.size() != ofc_a.size()) {//don't have proper number of coefficients
        if (msgLvl (MSG::DEBUG)) {
	  if (ofc_b.size()==0)
	    ATH_MSG_DEBUG( "No time-OFC's found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                           << " Gain "<< gain << " found. Time not calculated."  );
	  else
	    ATH_MSG_DEBUG( "OFC for time size " << ofc_b.size() 
                           << " not equal to OFC for energy size " << ofc_a.size() 
                           << "   Time not calculated "  );
        }
	noTime++;
      }else{
	for (unsigned i=0;i<(ofc_b.size());i++) 
	  time+=(samples[i+timeSampleShift]-pedestalAverage)*ofc_b.at(i);
	time/=ADCPeak;
	// !! Time is now in ns with respect to calibration pulse shape
	// Used to calculate quality factor
      }
      ATH_MSG_VERBOSE( "Time calculated " << time << " TimeBin=" << OFCTimeBin   );

      //Calculate Quality factor
      if (larShape) { //Have shape object
	//Get Shape & Shape Derivative for this channel

	//const std::vector<float>& shape=larShape->Shape(chid,gain,OFCTimeBin);
	//const std::vector<float>& shapeDer=larShape->ShapeDer(chid,gain,OFCTimeBin);
        // ###
        ILArShape::ShapeRef_t shape=larShape->Shape(chid,gain,OFCTimeBin,m_shapeMode);
        ILArShape::ShapeRef_t shapeDer=larShape->ShapeDer(chid,gain,OFCTimeBin,m_shapeMode);
        // ###
        // fixing HEC to move +1 in case of 4 samples and firstSample 0
        int ihecshift=0;
        if(nSamples == 4 && m_firstSample == 0 ){
          Identifier id ;
          try {
               id = cabling->cnvToIdentifier(chid); 
               if(m_emId->is_lar_hec(id)) {
                  ihecshift=1;
                  ATH_MSG_DEBUG( "Setting firstSample to 1 for HEC channel "<< chid.get_compact()  );
               }
          } catch ( LArID_Exception & except ) {
                  ATH_MSG_DEBUG( "A Cabling exception was caught for channel 0x!"
                                 << MSG::hex << chid.get_compact() << MSG::dec );
          }
        }
	
	//Check Shape
        if ((shape.size()+m_firstSample+ihecshift) < ofc_a.size()) {
          if (msgLvl (MSG::DEBUG)) {
	    if (shape.size()==0) 
              ATH_MSG_DEBUG( "No Shape found for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                             << " Gain "<< gain << ". Quality factor not calculated."  );
            else
              ATH_MSG_DEBUG( "Shape size " << shape.size() 
                             << "smaller than OFC size " << ofc_a.size() 
                             << "for channel 0x" << MSG::hex << chid.get_compact() 
                             << MSG::dec << ". Quality factor not calculated."  );
          }
	  quality=0;  //Can't calculate chi^2, assume good hit.
	  noShape++;
	}
	else {//Shape ok
	  if (time!=0 && shapeDer.size()!=shape.size()) { 
	    //Send warning
	    ATH_MSG_DEBUG( "Shape-Derivative has different size than Shape for channel 0x" << MSG::hex << chid.get_compact() << MSG::dec 
                           << ". Derivative not taken into accout for quality factor."  );
	    noShapeDer++;
	  }//end-if 
	  if (time==0 || shapeDer.size()!=shape.size() ) { //Calculate Q without time info
	    for (unsigned i=0;i<(ofc_a.size());i++)
	      quality+=((samples[i+timeSampleShift]-pedestalAverage)-shape[i+m_firstSample+ihecshift]*ADCPeak)*
		       ((samples[i+timeSampleShift]-pedestalAverage)-shape[i+m_firstSample+ihecshift]*ADCPeak);
	  }
	  else { //All input data ok, calculate Q with time info
	    for (unsigned i=0;i<(ofc_a.size());i++) 
	      quality+=((samples[i+timeSampleShift]-pedestalAverage)-((shape[i+m_firstSample+ihecshift]-shapeDer[i+m_firstSample+ihecshift]*time)*ADCPeak))*
		       ((samples[i+timeSampleShift]-pedestalAverage)-((shape[i+m_firstSample+ihecshift]-shapeDer[i+m_firstSample+ihecshift]*time)*ADCPeak));
	  }
	} // end else (Shape ok)

      } //end if larShape
      else { //No Shape found at all 
	quality=0; //Can't calculate chi^2, assume good hit.
	noShape++;
      }
    }// end-if energy>Ecut
    else 
      quality=-1; //in case E<Ecut
    //time*=1000.0; 
    time=time*(nanosecond/picosecond); //Convert time to ps
    //Make LArRawChannel Object with new data

    uint16_t iqual=0;
    uint16_t iprov=0xA5;
    if (quality>=0) {
       iqual = ( (int)(quality) ) & 0xFFFF;
       iprov = iprov | 0x2000;
    }

    LArRawChannel larRawChannel(chid,(int)energy,(int)time,iqual,iprov, gain);   
    larRawChannelContainer->push_back(larRawChannel); //Add to container 
    ntot_raw++;
    ATH_MSG_VERBOSE( "Got LArRawChannel #" << ntot_raw << ", chid=0x" << MSG::hex << chid.get_compact() << MSG::dec  
                     << " e=" << energy << " t=" << time << " Q=" << quality  );
  } // End loop over LArDigits

  ATH_MSG_DEBUG(  ntot_raw << " channels successfully processed, (" << highE << " with high energy)"  );

  // deal with bad timing
  if(BadTiming>=128){
    ATH_MSG_ERROR( "Too many channels (" <<BadTiming<<  " !) have a bad timing !!"  );
    ATH_MSG_ERROR( "OFC time constants should be revisited !!!"  );
    ATH_MSG_ERROR( "Event is skipped"  );
    larRawChannelContainer->clear();
    //return StatusCode::SUCCESS;
  }
  
  // in case of at least one saturating cell, skip all event (if selected) 
  if ( saturation && m_skipSaturCells == 2 ) {
    ATH_MSG_ERROR( saturation << " saturating channels were found. Event is skipped."  );
    larRawChannelContainer->clear();
  }
  
  //Put this LArRawChannel container in the transient store
  //sc = evtStore()->record(m_larRawChannelContainer, m_ChannelContainerName);
  //if(sc.isFailure()) {
  // log << MSG::ERROR << "Can't record LArRawChannelContainer in StoreGate" << endmsg;
  //}
  //else
  //  std::cout << "Successfully recorded LArRawChannelContainer to StoreGate" << std::endl;
  
  /*
   
   Error & Warning summary *per event*
   
   Strategy: 'No Energy' is an ERROR, no time or no quality is a WARNING
   
   Missing calibration constants are most likly missing for an entire run, threfore:
   In DEBUG: Print summary for each event if something is missing
   otherwise: Print summary only for new problems (different number of missing channels) 
  
   Saturatin cells summary is shown in any case, WARNING if not skipped, ERROR if skipped
   
  */
  
  if (noEnergy)   m_noEnergy++;
  if (noTime)     m_noTime++;
  if (noShape)    m_noShape++;
  if (noShapeDer) m_noShapeDer++;
  if (saturation) m_saturation++;
  
  m_aveNoEnergy   += noEnergy;  
  m_aveNoTime     += noTime;
  m_aveNoShape    += noShape;
  m_aveNoShapeDer += noShapeDer;
  m_aveSaturCells += saturation;
  
  if ( (   noEnergy!=m_lastNoEnergy 
        || noTime!=m_lastNoTime 
        || noShape>m_lastNoShape 
        || noShapeDer>m_lastNoShapeDer 
        || saturation>0 ) 
	   || ( msgSvc()->outputLevel(name()) <= MSG::DEBUG && ( noEnergy || noTime || noShape || noShapeDer || saturation ) )
      ) {
    
    m_lastNoEnergy = noEnergy;
    m_lastNoTime   = noTime;
    if (noShape>m_lastNoShape) m_lastNoShape=noShape;
    if (noShapeDer>m_lastNoShapeDer) m_lastNoShapeDer=noShapeDer;
    //m_lastSaturCells = saturation ;

    MSG::Level msglvl;
    if (noEnergy) 
      msglvl=MSG::ERROR;
    else if (noTime || noShape || noShapeDer || saturation)
      msglvl=MSG::WARNING;
    else
      msglvl=MSG::INFO;

    msg() << msglvl << " *** Error & Warning summary for this event *** " << std::endl;
    
    if ( noEnergy ) {
      msg() << msglvl << "   " << noEnergy << " out of " 
	  << digitContainer->size() << " channel(s) skipped due to a lack of basic calibration constants." 
	  << std::endl;  	  
    }
    if ( noTime ) {
      msg() << msglvl << "   " << noTime << " out of " 
	  << highE << " high-enegy channel(s) have no time-info due to a lack of Optimal Filtering Coefficients." 
	  << std::endl;
    }
    if ( noShape ) {
      msg() << msglvl << "   " << noShape << " out of " 
	  << highE << " high-enegy channel(s) have no quality factor due to a lack of shape." 
	  << std::endl;
    }
    if ( noShapeDer ) { 
      msg() << msglvl << "   " << noShapeDer << " out of " 
	  << highE << " high-enegy channel(s) lack the derivative of the shape. Not taken into accout for Quality factor." 
	  << std::endl;
    }
    if ( saturation ) {
      if ( m_skipSaturCells == 2 )
          msg() << MSG::ERROR << "   " << saturation << " out of " 
	      << digitContainer->size() << " channel(s) showed saturations. The complete event was skipped." << std::endl;
      else if ( m_skipSaturCells == 1 )
        msg() << MSG::ERROR << "   " << saturation << " out of " 
	      << digitContainer->size() << " channel(s) showed saturations and were skipped." << std::endl;
      else
        msg() << MSG::WARNING << "   " << saturation << " out of " 
	      << digitContainer->size() << " channel(s) showed saturations." << std::endl;
    }
    msg() << endmsg;
  }
    
  ATH_CHECK( SG::makeHandle(m_ChannelContainerName, ctx).record (std::move (larRawChannelContainer)) );

  return StatusCode::SUCCESS;
}

StatusCode LArRawChannelBuilder::finalize()
{ 

  if (m_noEnergy>0) m_aveNoEnergy   /= m_noEnergy;
  else m_aveNoEnergy=0;
  if (m_noTime>0) m_aveNoTime     /= m_noTime;
  else m_aveNoTime=0;
  if (m_noShape>0) m_aveNoShape    /= m_noShape;
  else m_aveNoShape=0;
  if (m_noShapeDer>0) m_aveNoShapeDer /= m_noShapeDer;
  else m_aveNoShapeDer=0;
  if (m_saturation>0) m_aveSaturCells /= m_saturation;
  else m_aveSaturCells=0;

  if (m_nEvents>0) m_aveChannels   /= m_nEvents;
  else m_aveChannels =0;

  // Error and Warning Summary for this job:
  
  ATH_MSG_DEBUG( "  LArRawChannelBuilder::finalize " 
                 << m_noEnergy << " " << m_noTime << " " << m_noShape << " " << m_noShapeDer << " " << m_saturation  );
  
  if ( m_noEnergy || m_noTime || m_noShape || m_noShapeDer || m_saturation ) {
    MSG::Level msglvl;
    if ( m_noEnergy || m_skipSaturCells ) 
      msglvl=MSG::ERROR;
    else
      msglvl=MSG::WARNING;
    msg() << msglvl << " *** Error & Warning Summary for all events *** " << std::endl ;
    
    if (m_noEnergy)
      msg() << msglvl << "   " << m_noEnergy << " events had on average " << (int)std::round(m_aveNoEnergy) 
          << " channels out of " << (int)std::round(m_aveChannels) << " without basic calibration constants." 
	  << std::endl;
    
    if (m_noTime) 
      msg() << msglvl << "   " << m_noTime  << " events had on average " << (int)std::round(m_aveNoTime) 
          << " channels out of " << (int)std::round(m_aveChannels) << " without OFCs for timing." 
	  << std::endl ;
   
    if (m_noShape)
      msg() << msglvl << "   " << m_noShape << " events had on average " << (int)std::round(m_aveNoShape) 
          << " channels out of " << (int)std::round(m_aveChannels) << " without shape information." 
	  << std::endl;
    
    if (m_noShapeDer)
      msg() << msglvl << "   " << m_noShapeDer << " events had on average " << (int)std::round(m_aveNoShapeDer) 
          << " channels out of " << (int)std::round(m_aveChannels) << " without shape derivative." 
	  << std::endl;
	
    if ( m_saturation )
      msg() << msglvl << "   " << m_saturation << " events had on average " << (int)std::round(m_aveSaturCells) 
          << " out of " << (int)std::round(m_aveChannels) << " saturating channels."  
	  << std::endl ;
    
    msg() << endmsg;
  } 
  else
    ATH_MSG_INFO( "LArRawChannelBuilder finished without errors or warnings."  );

  //if (m_larRawChannelContainer) {
    //m_larRawChannelContainer->release();
    //m_larRawChannelContainer = 0;
  //}

  return StatusCode::SUCCESS;
}
