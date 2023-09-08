/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibDataQuality/LArAutoCorrValidationAlg.h"
#include <cmath>
#include "CaloIdentifier/CaloCell_ID.h"

LArAutoCorrValidationAlg::LArAutoCorrValidationAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  LArAutoCorrValidationBase(name,pSvcLocator) {

  declareProperty("AutoCorrTolerance",m_covToleranceInit,
		  "CaloCellGroup of allowed deviation of cov[i]");
  declareProperty("AutoCorrToleranceFEB",m_covToleranceFEBInit,
		  "CaloCellGroup of allowed deviation of cov[i] (average over one FEB,");

  declareProperty("NSamplesToCheck",m_nSamplesToCheck=1, 
		  "Number of samples (covr elements) to check");

  
  declareProperty("CheckFifthSample",m_checkFifthSample=false); 
  m_covGlobalRef=0.;
  m_covGlobalVal=0.;
  m_nEntriesGlobal=0;
}


StatusCode LArAutoCorrValidationAlg::preLoop() {
  m_covGlobalRef=0.;
  m_covGlobalVal=0.;
  m_nEntriesGlobal=0;

  bool stat;

  ATH_MSG_INFO ( "Initialize covariance tolerances (CaloCellGroup)" ) ;
  stat=m_covTolerance.setDefinition(m_caloId,m_covToleranceInit,msg());
  if (!stat) {
    ATH_MSG_ERROR ( "Failed to initialize CaloCellGroup of covariance tolerances!" ) ;
    return StatusCode::FAILURE;
  }
  if (m_covTolerance.getDefaults().size()!=3) {
    ATH_MSG_ERROR ( "Configuration error: Expected three covariance tolerance values (one per gain), got " 
                    << m_covTolerance.getDefaults().size() ) ;
    return StatusCode::FAILURE;
  }
  if (this->msgLvl(MSG::DEBUG)) m_covTolerance.printDef();//for debugging....


  
  ATH_MSG_INFO ( "Initialize covariance FEB tolerances (CaloCellGroup)" ) ;
  stat=m_covToleranceFEB.setDefinition(m_caloId,m_covToleranceFEBInit,msg());
  if (!stat) {
    ATH_MSG_ERROR ( "Failed to initialize CaloCellGroup of covariance tolerances!" ) ;
    return StatusCode::FAILURE;
  }
  if (m_covToleranceFEB.getDefaults().size()!=3) {
    ATH_MSG_ERROR ( "Configuration error: Expected three covariance tolerance values (one per gain), got " 
                    << m_covToleranceFEB.getDefaults().size() ) ;
    return StatusCode::FAILURE;
  }
  if (this->msgLvl(MSG::DEBUG)) m_covToleranceFEB.printDef();//for debugging....


  return StatusCode::SUCCESS;
  
}

bool LArAutoCorrValidationAlg::validateChannel(const LArCondObj& ref, const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling,const LArBadChannelCont *bcCont) {
  
if (gain<0 || gain>2) {
     ATH_MSG_ERROR ( "Unexpected gain value " << gain ) ;
     return false;
  }

  if (val.m_vAutoCorr.size()==0) {
    msg() <<  this->m_myMsgLvl << "Empty! No AC found for " << channelDescription(chid,cabling, bcCont, gain) << endmsg;
    return false;
  }
  if (ref.m_vAutoCorr.size()==0) {
    ATH_MSG_WARNING ( "No reference value found for " << channelDescription(chid,cabling, bcCont, gain) ) ;
    return false;
  }

  const float covVal=val.m_vAutoCorr[0];
  const float covRef=ref.m_vAutoCorr[0];

  const Identifier id=cabling->cnvToIdentifier(chid);

  const float covTolerance=m_covTolerance.valuesForCell(id)[gain];

  
  HWIdentifier febid=m_onlineHelper->feb_Id(chid);
  DataPerFEB* dataPerFEB = m_vDataPerFEB.empty() ? nullptr : &(m_vDataPerFEB.back());
  if (!dataPerFEB || dataPerFEB->febid!=febid) {//Got to new FEB
    m_vDataPerFEB.push_back(DataPerFEB(chid,febid,gain));
    dataPerFEB=&(m_vDataPerFEB.back());
  }
  dataPerFEB->covVal+=covVal;
  dataPerFEB->covRef+=covRef;
  ++(dataPerFEB->nEntries);

  m_covGlobalVal+=covVal;
  m_covGlobalRef+=covRef;
  ++m_nEntriesGlobal;
  
  const size_t s=val.m_vAutoCorr.size();
  const size_t sr=ref.m_vAutoCorr.size();
  for (size_t i=0;i<s;++i) {
    const float covVal_i=val.m_vAutoCorr[i];
    if (fabs(covVal_i)>1.0) {
      msg() <<  this->m_myMsgLvl << "Unphysical! " << channelDescription(chid, cabling, bcCont, gain) << " AutoCorr[" << i << "]: " 
	       <<  std::setprecision(4) << covVal_i << endmsg;
      return false;
    }
    if (m_checkFifthSample and i==5 and fabs(covVal_i)>0.13) {
      msg() <<  this->m_myMsgLvl << "LARGE Autocorr sample 5 " << channelDescription(chid, cabling, bcCont, gain) << " AutoCorr[" << i << "]: " << covVal_i << endmsg;
      return false;
    }
    if (i<m_nSamplesToCheck && i<sr) {
      const float covRef_i=ref.m_vAutoCorr[i];
      if (fabs(covVal_i-covRef_i)> covTolerance){
	if (m_nFailedValidation<m_maxmessages) {
	  std::stringstream devMsg;
	  devMsg.setf(std::ios::fixed,std::ios::floatfield);
	  devMsg <<  "Deviating! " << channelDescription(chid, cabling, bcCont,gain) << " AutoCorr[" << i << "]: " << std::setprecision(4) << covVal_i
		 <<" (" << covRef_i << ", " << std::setprecision(2) << ((covVal_i-covRef_i)/covRef_i)*100 << "%)";
	  msg() << this->m_myMsgLvl << devMsg.str() << endmsg;
	  ATH_MSG_DEBUG ( "Covariance Tolerance: " <<  covTolerance ) ;
	}
	if (m_nFailedValidation==m_maxmessages)
	  msg() <<  this->m_myMsgLvl << "Channel deviation message has already been printed " << m_maxmessages << " times. Now silent..." << endmsg;
	return false;
      }//end if > tolerance
    }//end if nSamplesToCheck
  }// End loop over all samples
  return true;
}


bool LArAutoCorrValidationAlg::febSummary(const LArOnOffIdMapping *cabling,const LArBadChannelCont * bcCont) {
  unsigned nBadFebs=0;
  msg().precision(3);
  msg().setf(std::ios::fixed,std::ios::floatfield); 
  for (DataPerFEB& dataPerFeb : m_vDataPerFEB) {
    dataPerFeb.covVal/=dataPerFeb.nEntries;
    dataPerFeb.covRef/=dataPerFeb.nEntries;

    const Identifier id=cabling->cnvToIdentifier(dataPerFeb.chid);
    const float& covToleranceFEB=m_covToleranceFEB.valuesForCell(id)[dataPerFeb.gain];

    if (fabs(dataPerFeb.covVal-dataPerFeb.covRef)>covToleranceFEB){
      msg() << m_myMsgLvl << "Deviating!" << channelDescription(dataPerFeb.febid, cabling, bcCont, dataPerFeb.gain,true) << "Average AutoCorr: " 
            << dataPerFeb.covVal << " (" << dataPerFeb.covRef << ")" << endmsg;
      ++nBadFebs;
    }
  }

  if (nBadFebs) {
    msg() << m_myMsgLvl << "Found " << nBadFebs << " out of " << m_vDataPerFEB.size() << " FEBs deviating from reference" << endmsg;
    return false;
  }
  else {
    ATH_MSG_INFO ( "All" << m_vDataPerFEB.size() << " FEBs withing given tolerance." ) ;
    return true;
  }
}
StatusCode LArAutoCorrValidationAlg::summary(const LArOnOffIdMapping *cabling,const LArBadChannelCont *bcCont) {
  StatusCode sc=StatusCode::SUCCESS;
  //1nd step: Check the FEB-averages:
  if (m_doFebAverages && !febSummary(cabling, bcCont))
    sc=StatusCode::RECOVERABLE;
  //2nd step: Call the summary method from base-class (single-channel summary)
  if (!LArAutoCorrValidationBase::summary(cabling, bcCont).isSuccess())
    sc=StatusCode::RECOVERABLE;
  //3rd step: Check the gobal averages:
  if (m_nEntriesGlobal) {
    m_covGlobalVal/=m_nEntriesGlobal;
    m_covGlobalRef/=m_nEntriesGlobal;
  }
  ATH_MSG_INFO ( "Global autocorr average: " << m_covGlobalVal << " Referecence:" << m_covGlobalRef
                 << " Deviation:" << m_covGlobalVal-m_covGlobalRef ) ;
  return sc;
}
