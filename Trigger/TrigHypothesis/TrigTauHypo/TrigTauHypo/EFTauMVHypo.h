/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:     EFTauMVHypo.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigTauHypo
 *
 * AUTHOR:   m. Morgenstern based on EFTauInvHypo
 * CREATED:  Jun 20, 2011
 *
  *********************************************************************/
#ifndef TRIGEFTAUMVHYPO_H
#define TRIGEFTAUMVHYPO_H

#include "TrigInterfaces/HypoAlgo.h"
#include "TGraph.h"

class StoreGateSvc;
namespace HLT {
  class TriggerElement;
}

class EFTauMVHypo : public HLT::HypoAlgo {

 public:

  /** constructor of EF tauRec hypo algo */
  EFTauMVHypo(const std::string& name, ISvcLocator* pSvcLocator);
  /** destructor */
  ~EFTauMVHypo();

  /** HLT method for initialize */
  HLT::ErrorCode hltInitialize();

  /** HLT method for finalize */
  HLT::ErrorCode hltFinalize();

  /** HLT method for execute Hypo on  a TE.
      input is TE, output is pass=True or False */
  HLT::ErrorCode hltExecute(const HLT::TriggerElement* outputTE, bool& pass);

 private:

  /** Cuts to be applied */

  /** min numTrack cut **/
  int m_numTrackMin;
  int m_numTrackMax;

  double m_EtCalibMin;

  int m_level;
  int m_method;

  /** min BDTScore cut **/
  //double m_BDTScoreMin;

  /** variables used for cuts in hypo algorithm */   
  int m_numTrack;

  double m_LLHScore;
  double m_BDTScore;

  /** for monitoring */
  int  m_cutCounter;
  std::string s_cut_level;
  std::map<std::string,TGraph> m_cuts;
  TGraph *OneProngGraph,*MultiProngGraph;
};
#endif

