/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArRamps2Ntuple.h"
#include "LArRawConditions/LArRawRampContainer.h"
#include "LArRawConditions/LArRampComplete.h"

#include <cmath>

LArRamps2Ntuple::LArRamps2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator),
  m_rampKey("LArRamp"){

  declareProperty("ContainerKey",   m_contKey,
		  "List of keys of RawRamp containers");
  declareProperty("RampKey",   m_rampKey,
		  "Key of LArRampComplete or LArRampMC objects");
  declareProperty("NtupleName",     m_ntName  ="RAMPS");
  declareProperty("RawRamp",        m_rawRamp = false); 
  declareProperty("SaveAllSamples", m_saveAllSamples = false);
  declareProperty("ApplyCorr",      m_applyCorr=false);
  declareProperty("AddCorrUndo",    m_addCorrUndo=true);
}

StatusCode LArRamps2Ntuple::initialize() {
  m_ntTitle="Ramps";
  m_ntpath=std::string("/NTUPLES/FILE1/")+m_ntName;

  ATH_CHECK(m_rampKey.initialize());
  return LArCond2NtupleBase::initialize();
}


LArRamps2Ntuple::~LArRamps2Ntuple() 
= default;

StatusCode LArRamps2Ntuple::stop() {
  bool hasRawRampContainer=false;
  StatusCode sc;
  NTuple::Item<long> cellIndex;
  NTuple::Item<long> gain;
  NTuple::Item<long> corrUndo;
  NTuple::Array<float> SampleMax;
  NTuple::Array<float> TimeMax;
  NTuple::Array<float> DAC;
  NTuple::Array<float> ADC;
  NTuple::Array<long>  NTriggers;
  
  // individual samples. Very dirty :-(
  NTuple::Array<float> Sample0;
  NTuple::Array<float> Sample1;
  NTuple::Array<float> Sample2;
  NTuple::Array<float> Sample3;
  NTuple::Array<float> Sample4;
  NTuple::Array<float> Sample5;
  NTuple::Array<float> Sample6;
  
  // idem for RMS
  NTuple::Array<float> RMS0;
  NTuple::Array<float> RMS1;
  NTuple::Array<float> RMS2;
  NTuple::Array<float> RMS3;
  NTuple::Array<float> RMS4;
  NTuple::Array<float> RMS5;
  NTuple::Array<float> RMS6;

  NTuple::Item<unsigned long> DACIndex;
  NTuple::Array<float> coeffs;
  NTuple::Item<unsigned long> coeffIndex;
  
  NTuple::Item<float> RampRMS;

 if (m_rawRamp && !m_contKey.empty()) {
   //Retrieve Raw Ramp Container
   for (const std::string& key : m_contKey) {
       LArRawRampContainer* rawRampContainer=nullptr;
       sc=m_detStore->retrieve(rawRampContainer,key);
       if (sc!=StatusCode::SUCCESS || !rawRampContainer) {
         ATH_MSG_WARNING( "Unable to retrieve LArRawRampContainer with key " << key );
       } 
       else {
         ATH_MSG_DEBUG( "Got LArRawRampContainer with key " << key );
         hasRawRampContainer = true;
       }
   }
   if (!hasRawRampContainer) ATH_MSG_WARNING( " No LArRawRampContainer found. Only fitted ramp in ntuple " );

 } 
 //end-if m_rawRamp

 
 // For compatibility with existing configurations, look in the detector
 // store first, then in conditions.
 const ILArRamp* ramp =
   detStore()->tryConstRetrieve<ILArRamp> (m_rampKey.key());
 if (!ramp) {
   SG::ReadCondHandle<ILArRamp> readHandle{m_rampKey};
   ramp = *readHandle;
 }
 
 LArRampComplete* rampComplete_nc=nullptr; //for (potential) applyCorr

 if (ramp==nullptr) {
   ATH_MSG_WARNING( "Unable to retrieve ILArRamp with key: "<<m_rampKey << " from DetectorStore" );
 }

 if (!ramp && !hasRawRampContainer) {
   ATH_MSG_ERROR( "Have neither Raw Ramp nor Fitted Ramp. No Ntuple produced." );
   return StatusCode::FAILURE;
 }

 sc=m_nt->addItem("cellIndex",cellIndex,0,2000);
 if (sc!=StatusCode::SUCCESS) {
   ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
   return StatusCode::FAILURE;
 }

 sc=m_nt->addItem("gain",gain,0,3);
 if (sc!=StatusCode::SUCCESS) {
   ATH_MSG_ERROR( "addItem 'gain' failed" );
   return StatusCode::FAILURE;
 }

 if (m_addCorrUndo) {
   sc=m_nt->addItem("corrUndo",corrUndo,0,1);
   if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "addItem 'corrUndo' failed" );
     return StatusCode::FAILURE;
   }
 }

 if (hasRawRampContainer) 
   {
     sc=m_nt->addItem("DACIndex",DACIndex,0,800);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'DACIndex' failed" );
       return StatusCode::FAILURE;
     }
     
     sc=m_nt->addItem("SampleMax",DACIndex,SampleMax);
     if (sc!=StatusCode::SUCCESS)
       {ATH_MSG_ERROR( "addItem 'SampleMax' failed" );
        return StatusCode::FAILURE;
       }
      
     sc=m_nt->addItem("TimeMax",DACIndex,TimeMax);
     if (sc!=StatusCode::SUCCESS)
       {ATH_MSG_ERROR( "addItem 'TimeMax' failed" );
        return StatusCode::FAILURE;
       }
     sc=m_nt->addItem("ADC",DACIndex,ADC);
     if (sc!=StatusCode::SUCCESS)
       {ATH_MSG_ERROR( "addItem 'ADC' failed" );
        return StatusCode::FAILURE;
       }
      
     sc=m_nt->addItem("DAC",DACIndex,DAC);
     if (sc!=StatusCode::SUCCESS)
       {ATH_MSG_ERROR( "addItem 'DAC' failed" );
        return StatusCode::FAILURE;
       }
      
     sc=m_nt->addItem("NTriggers",DACIndex,NTriggers);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'NTriggers' failed" );
       return StatusCode::FAILURE;
     }

     if(m_saveAllSamples){
       sc=m_nt->addItem("Sample0",DACIndex,Sample0);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample0' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample1",DACIndex,Sample1);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample1' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample2",DACIndex,Sample2);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample2' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample3",DACIndex,Sample3);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample3' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample4",DACIndex,Sample4);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample4' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample5",DACIndex,Sample5);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample5' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("Sample6",DACIndex,Sample6);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'Sample6' failed" );
	  return StatusCode::FAILURE;
	 }
       
       sc=m_nt->addItem("RMS0",DACIndex,RMS0);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS0' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS1",DACIndex,RMS1);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS1' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS2",DACIndex,RMS2);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS2' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS3",DACIndex,RMS3);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS3' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS4",DACIndex,RMS4);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS4' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS5",DACIndex,RMS5);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS5' failed" );
	  return StatusCode::FAILURE;
	 }
       sc=m_nt->addItem("RMS6",DACIndex,RMS6);
       if (sc!=StatusCode::SUCCESS)
	 {ATH_MSG_ERROR( "addItem 'RMS6' failed" );
	  return StatusCode::FAILURE;
	 }
     }// end-if Save all samples
   }//end if rawRampContainer

 if (ramp) {
   sc=m_nt->addItem("Xi",coeffIndex,0,7);
   if (sc!=StatusCode::SUCCESS)
     {ATH_MSG_ERROR( "addItem 'coeffIndex' failed" );
      return StatusCode::FAILURE;
     }
   
   sc=m_nt->addItem("X",coeffIndex,coeffs);
   if (sc!=StatusCode::SUCCESS)
     {ATH_MSG_ERROR( "addItem 'coeff' failed" );
      return StatusCode::FAILURE;
     }

   if (hasRawRampContainer) { //== RampComplete && RawRamp
     sc=m_nt->addItem("RampRMS",RampRMS,-1000,1000);
     if (sc!=StatusCode::SUCCESS)
       {ATH_MSG_ERROR( "addItem 'RampRMS' failed" );
        return StatusCode::FAILURE;
       }
   }

   if (m_applyCorr) {
     const LArRampComplete* rampComplete=dynamic_cast<const LArRampComplete*>(ramp);
     if (!rampComplete) {
       ATH_MSG_WARNING("Failed to dyn-cast to ILArRamp to LArRampComplete. Cannot apply corrections");
       m_applyCorr=false;
     }
     if (rampComplete and !rampComplete->correctionsApplied()) {
	rampComplete_nc=const_cast<LArRampComplete*>(rampComplete);
	sc=rampComplete_nc->applyCorrections();
	if (sc.isFailure()) {
	  ATH_MSG_ERROR( "Failed to apply corrections to LArRampComplete!" );
	}
	else
	  ATH_MSG_INFO( "Applied corrections to LArRampComplete" );
      }
      else {
	ATH_MSG_WARNING( "Corrections already applied. Can't apply twice!" );
      }
    }// end if applyCorr
 }//end-if ramp 

 const LArOnOffIdMapping *cabling=nullptr;
 if(m_isSC) {
   ATH_MSG_DEBUG( "LArRamps2Ntuple: using SC cabling" );
   SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingSCKey};
   cabling=*cablingHdl;
 }else{
   SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
   cabling=*cablingHdl;
 }


 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 unsigned cellCounter=0;
 if (hasRawRampContainer) { //Loop over raw ramp container and fill ntuple

   //Retrieve Raw Ramp Container
   for (const std::string& key : m_contKey) {
       LArRawRampContainer* rawRampContainer=nullptr;
       sc=m_detStore->retrieve(rawRampContainer,key);
       if (sc!=StatusCode::SUCCESS || !rawRampContainer) {
         ATH_MSG_WARNING( "Unable to retrieve LArRawRampContainer with key " << key );
         continue;
       }
       for (const LArRawRamp* rawramp : *rawRampContainer) {
         const std::vector<LArRawRamp::RAMPPOINT_t>& singleRamp=rawramp->theRamp();

         for (DACIndex=0;DACIndex<singleRamp.size();DACIndex++) {
           SampleMax[DACIndex] = singleRamp[DACIndex].iMaxSample;
           TimeMax[DACIndex]   = singleRamp[DACIndex].TimeMax;
           ADC[DACIndex]       = singleRamp[DACIndex].ADC;
	   NTriggers[DACIndex] = singleRamp[DACIndex].NTriggers;
           DAC[DACIndex]       = singleRamp[DACIndex].DAC;

           if(m_saveAllSamples){
	     
	     if ( singleRamp[DACIndex].Samples.empty() || singleRamp[DACIndex].RMS.empty() ) {     
	       ATH_MSG_WARNING( "Cannot save all samples, vector empty" );
	     } else {
	       
	       Sample0[DACIndex]=singleRamp[DACIndex].Samples[0];
	       Sample1[DACIndex]=singleRamp[DACIndex].Samples[1];
	       Sample2[DACIndex]=singleRamp[DACIndex].Samples[2];
	       Sample3[DACIndex]=singleRamp[DACIndex].Samples[3];
	       Sample4[DACIndex]=singleRamp[DACIndex].Samples[4];
	       if(singleRamp[DACIndex].Samples.size()>5) Sample5[DACIndex]=singleRamp[DACIndex].Samples[5];
	       if(singleRamp[DACIndex].Samples.size()>6) Sample6[DACIndex]=singleRamp[DACIndex].Samples[6];
	 
	       RMS0[DACIndex]=singleRamp[DACIndex].RMS[0];
	       RMS1[DACIndex]=singleRamp[DACIndex].RMS[1];
	       RMS2[DACIndex]=singleRamp[DACIndex].RMS[2];
	       RMS3[DACIndex]=singleRamp[DACIndex].RMS[3];
	       RMS4[DACIndex]=singleRamp[DACIndex].RMS[4];
	       if(singleRamp[DACIndex].RMS.size()>5) RMS5[DACIndex]=singleRamp[DACIndex].RMS[5];
	       if(singleRamp[DACIndex].RMS.size()>6) RMS6[DACIndex]=singleRamp[DACIndex].RMS[6];
	     }
	   }	   

         }    

        HWIdentifier chid=rawramp->channelID();
	unsigned igain = (unsigned)rawramp->gain();
	gain = igain;
	if (m_addCorrUndo) corrUndo=0;
	if (ramp && cabling->isOnlineConnected(chid)) {

          //FT move to here
	  fillFromIdentifier(chid);
	  
          unsigned nDAC=0;
          unsigned nCoeff=0;
          const ILArRamp::RampRef_t  rampcoeff = ramp->ADC2DAC(chid,igain);
          if (rampcoeff.size()==0) {
             ATH_MSG_WARNING( "Can't get fitted Ramp slot=" << static_cast<long>(m_slot) << 
                          " channel=" << static_cast<long>(m_channel) << " gain=" << igain );
          }
          for (coeffIndex=0;coeffIndex<rampcoeff.size();coeffIndex++) coeffs[coeffIndex]=rampcoeff[coeffIndex];
          nDAC = singleRamp.size();
          nCoeff = rampcoeff.size();

	  if (nDAC>1 && nCoeff>0) {
	    
	    double rampDev=0;
	    for (unsigned iDAC=1;iDAC<nDAC;iDAC++) {
	      double fittedResult=0;
	      for (unsigned icoeff=0;icoeff<nCoeff;icoeff++)
		fittedResult+=coeffs[icoeff]*pow(ADC[iDAC],icoeff);
	      rampDev+=(fittedResult-DAC[iDAC])*(fittedResult-DAC[iDAC]);
	    }//end loop over DAC values
	    RampRMS=1./(nDAC-1)*sqrt(rampDev);
	  }//end if nDAC>0 && nCoeff>0
	  else
	    RampRMS=-999;

	sc=ntupleSvc()->writeRecord(m_nt);
	if (sc!=StatusCode::SUCCESS) {
	  ATH_MSG_ERROR( "writeRecord failed" );
	  return StatusCode::FAILURE;
	}

	}//end if rampComplete or rampMC
	
	cellCounter++;
       
       }//end loop over container
   }    // loop over gains
 } else {//have only fitted ramp

   //Iterate over gains and cells
   for ( unsigned igain=CaloGain::LARHIGHGAIN; 
	 igain<CaloGain::LARNGAIN ; ++igain )
   {
     for (HWIdentifier chid : m_onlineId->channel_range()) {
       if (cabling->isOnlineConnected(chid)) {
	 gain  = (long)igain;
	 if (m_addCorrUndo) corrUndo = 0;
         const ILArRamp::RampRef_t  rampcoeff=ramp->ADC2DAC(chid, gain);
         if (rampcoeff.size()==0) continue; // No ramp for this cell
         cellIndex  = cellCounter;
         fillFromIdentifier(chid);
         for (coeffIndex=0;coeffIndex<rampcoeff.size();coeffIndex++) coeffs[coeffIndex]=rampcoeff[coeffIndex];
     
	 sc=ntupleSvc()->writeRecord(m_nt);

	 if (sc!=StatusCode::SUCCESS) {
	   ATH_MSG_ERROR( "writeRecord failed" );
	   return StatusCode::FAILURE;
	 }
       }// end if isConnected
       cellCounter++;
     }//end loop over cells 
   }//end loop over gains

 } //end else have only fitted ramp

 if (ramp && m_addCorrUndo) {
   //Now loop over undoCorrections:
   for ( unsigned igain=CaloGain::LARHIGHGAIN; 
	 igain<CaloGain::LARNGAIN ; ++igain ) {
     LArRampComplete::ConstCorrectionIt itUndo,itUndo_e;
     itUndo_e = itUndo;
     const LArRampComplete *rampComplete=dynamic_cast<const LArRampComplete *>(ramp);
     if(rampComplete) {
       itUndo = rampComplete->undoCorrBegin(igain);
       itUndo_e=rampComplete->undoCorrEnd(igain);

       for(;itUndo!=itUndo_e;itUndo++) {
          const HWIdentifier chid(itUndo->first);
          gain  = (long)igain;
          corrUndo = 1;
          const std::vector<float>& rampcoeff=itUndo->second.m_vRamp;
          if (rampcoeff.empty())
            continue; // No ramp for this cell
 
          cellIndex  = cellCounter;
          fillFromIdentifier(chid);
 
          for (coeffIndex=0;coeffIndex<rampcoeff.size();coeffIndex++) coeffs[coeffIndex]=rampcoeff[coeffIndex];
        
          sc=ntupleSvc()->writeRecord(m_nt);
 
          if (sc!=StatusCode::SUCCESS) {
            ATH_MSG_ERROR( "writeRecord failed" );
            return StatusCode::FAILURE;
          }
       }//end loop over cells
     }
   }//end loop over gains
 }//end if add corrections

 if (rampComplete_nc) {
   ATH_CHECK(rampComplete_nc->undoCorrections());
   ATH_MSG_INFO("Reverted corrections of LArRampComplete container");
 }

 ATH_MSG_INFO( "LArRamps2Ntuple has finished." );
 return StatusCode::SUCCESS;

} // end finalize-method.
