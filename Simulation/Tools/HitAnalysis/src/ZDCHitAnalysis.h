/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_HIT_ANALYSIS_H
#define ZDC_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>
#include "TH1.h"
#include "TTree.h"
#include "ZdcIdentifier/ZdcID.h"

class TH1;
class TTree;

 
class ZDCHitAnalysis : public AthAlgorithm {

 public:

   ZDCHitAnalysis(const std::string& name, ISvcLocator* pSvcLocator);
   ~ZDCHitAnalysis(){}

   virtual StatusCode initialize();
   virtual StatusCode execute();

 private:

   /** Some variables**/
   TH1*  m_h_zdc_photons[2][5] = {{nullptr,nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr,nullptr}};
   TH1*  m_h_zdc_calibTot[2][5]= {{nullptr,nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr,nullptr}};
   TH1*  m_h_zdc_calibEM[2][5]= {{nullptr,nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr,nullptr}};
   TH1*  m_h_zdc_calibNonEM[2][5]= {{nullptr,nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr,nullptr}};
   
   std::vector<int>* m_zdc_fiber_side;
   std::vector<int>* m_zdc_fiber_mod;
   std::vector<int>* m_zdc_fiber_channel;
   std::vector<int>* m_zdc_fiber_photons;
   
   std::vector<int>* m_zdc_calib_side;
   std::vector<int>* m_zdc_calib_mod;
   std::vector<int>* m_zdc_calib_channel;
   std::vector<float>* m_zdc_calib_Total;
   std::vector<float>* m_zdc_calib_EM;
   std::vector<float>* m_zdc_calib_NonEM;

   TTree * m_tree;
   std::string m_ntupleFileName; 
   std::string m_path;
   ServiceHandle<ITHistSvc>  m_thistSvc;
   ZdcID *m_ZdcID;

};

#endif // ZDC_HIT_ANALYSIS_H
