/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"

// Local include(s):
#include "TrigT1EventTPCnv/MuCTPIL1TopoCandidateCnv_p1.h"

/**
 * Function transferring the information from a persistent MuCTPIL1TopoCandidate_p1 object
 * to a transient MuCTPIL1TopoCandidate object.
 */
void MuCTPIL1TopoCandidateCnv_p1::persToTrans( const MuCTPIL1TopoCandidate_p1* persObj, LVL1::MuCTPIL1TopoCandidate* transObj,
				      MsgStream& log ) {

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting MuCTPIL1TopoCandidate from persistent state..." << endmsg;

  transObj->setCandidateData(persObj->m_sectorName,
			     persObj->m_roiID,
			     persObj->m_bcid,
			     persObj->m_ptThresholdID,
			     persObj->m_ptL1TopoCode,
			     persObj->m_ptValue,
			     persObj->m_eta,
			     persObj->m_phi,
			     persObj->m_etacode,
			     persObj->m_phicode,
			     persObj->m_etamin,
			     persObj->m_etamax,
			     persObj->m_phimin,
			     persObj->m_phimax,
			     persObj->m_mioctID,
			     persObj->m_ieta,
			     persObj->m_iphi);

  transObj->setTGCFlags(persObj->m_bw2or3,
			persObj->m_innerCoin,
			persObj->m_goodMF,
			persObj->m_charge);
  
  transObj->setRPCFlags(persObj->m_is2cand,
			persObj->m_phiOvl);
			
  transObj->setRoiWord(persObj->m_roiWord);
  

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting MuCTPIL1TopoCandidate from persistent state [OK]" << endmsg;

  return;
}

/**
 * Function transferring the information from a transient MuCTPIL1TopoCandidate object
 * to a persistent MuCTPIL1TopoCandidate_p1 object.
 */
void MuCTPIL1TopoCandidateCnv_p1::transToPers( const LVL1::MuCTPIL1TopoCandidate* transObj, MuCTPIL1TopoCandidate_p1* persObj,
				      MsgStream& log ) {

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating persistent state of MuCTPIL1TopoCandidate..." << endmsg;

  persObj->m_sectorName       = transObj->getSectorName();
  persObj->m_roiID            = transObj->getRoiID();
  persObj->m_bcid 	      = transObj->getbcid();
  persObj->m_ptThresholdID    = transObj->getptThresholdID();
  persObj->m_ptL1TopoCode     = transObj->getptL1TopoCode();
  persObj->m_ptValue	      = transObj->getptValue();
  persObj->m_eta	      = transObj->geteta();
  persObj->m_phi	      = transObj->getphi();
  persObj->m_etacode	      = transObj->getetacode();
  persObj->m_phicode	      = transObj->getphicode();
  persObj->m_etamin	      = transObj->getetamin();
  persObj->m_etamax	      = transObj->getetamax();
  persObj->m_phimin	      = transObj->getphimin();
  persObj->m_phimax	      = transObj->getphimax();
  persObj->m_roiWord	      = transObj->getRoiWord();
  persObj->m_mioctID	      = transObj->getMioctID();
  persObj->m_ieta	      = transObj->getieta();
  persObj->m_iphi	      = transObj->getiphi();
  persObj->m_phiOvl	      = transObj->getphiOvl();
  persObj->m_is2cand	      = transObj->getis2cand();
  persObj->m_charge	      = transObj->getcharge();
  persObj->m_bw2or3	      = transObj->getbw2or3();
  persObj->m_innerCoin	      = transObj->getinnerCoin();
  persObj->m_goodMF           = transObj->getgoodMF();   

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating persistent state of MuCTPIL1TopoCandidate [OK]" << endmsg;

  return;
}
