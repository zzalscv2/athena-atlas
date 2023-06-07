/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ITHistSvc.h"

class ISurveyConstraint;
class PixelID;
class SCT_ID;
class TH1;


/////////////////////////////////////////////////////////////////////////////

class ATLAS_NOT_THREAD_SAFE SurveyConstraintTestAlg : public AthAlgorithm {
 public:
  SurveyConstraintTestAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  StatusCode BookHist(); 
  void CreateMisAlignNtuple();
  
 private :
  IToolSvc*                              m_toolsvc;            //!< Pointer to tool service
  ISurveyConstraint*                     m_SurvConstr;
  const PixelID*                         m_pixid;
  const SCT_ID*                          m_sctid;
  TH1*                                   m_h_PixEC_Align_Disk[6];
  TH1*                                   m_h_PixEC_Align_first[6];
  TH1*                                   m_h_PixEC_Align[6];

  int                  m_AlignResults_nModules;
  NTuple::Item<double> m_AlignResults_x;
  NTuple::Item<double> m_AlignResults_y;
  NTuple::Item<double> m_AlignResults_z;
  NTuple::Item<double> m_AlignResults_alpha;
  NTuple::Item<double> m_AlignResults_beta;
  NTuple::Item<double> m_AlignResults_gamma;
  NTuple::Item<long>   m_AlignResults_Identifier_ID;
  NTuple::Item<long>   m_AlignResults_Identifier_SCT;
  NTuple::Item<long>   m_AlignResults_Identifier_BarrelEC;
  NTuple::Item<long>   m_AlignResults_Identifier_LayerDisc;
  NTuple::Item<long>   m_AlignResults_Identifier_Phi;
  NTuple::Item<long>   m_AlignResults_Identifier_Eta;

  // algorithm parameters, possible to declare at runtime
  int                                    m_NLoop;                       //!< 
};
 
