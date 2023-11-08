
//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBUTILS_LAROFCALGORITHM_H
#define LARCALIBUTILS_LAROFCALGORITHM_H
 
#include <vector>
#include <string>
 
#include "LArRawConditions/LArWaveCumul.h"

#include "GaudiKernel/ToolHandle.h"
#include "LArElecCalib/ILArAutoCorrDecoderTool.h"

#include "CaloIdentifier/CaloGain.h"
#include "LArRawConditions/LArCaliWaveContainer.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

#include "LArRawConditions/LArOFCComplete.h"
#include "LArRawConditions/LArOFCBinComplete.h"
#include "LArRawConditions/LArShapeComplete.h"
#include "LArCOOLConditions/LArDSPConfig.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include <Eigen/Dense>

#include "tbb/blocked_range.h"
#include "tbb/global_control.h"

#include <memory>

#include "CxxUtils/checker_macros.h"

class LArOnlineID_Base; 
class CaloDetDescrManager_Base; 


class ATLAS_NOT_THREAD_SAFE LArOFCAlg:public AthAlgorithm {
  //Acutally this algo can do internal multi-threading at finalize 
  //but not the way regular athenaMT works, so the thread-safety checker complains 
public:
 
  LArOFCAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute() {return StatusCode::SUCCESS;}
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}

private:

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"ScCablingKey","LArOnOffIdMapSC","SG Key of SC LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey { this
      , "CaloSuperCellDetDescrManager"
      , "CaloSuperCellDetDescrManager"
      , "SG Key for CaloSuperCellDetDescrManager in the Condition Store" };

  struct perChannelData_t {
    //Input:
    const LArWaveCumul* inputWave;
    HWIdentifier chid;
    unsigned gain;

    //Output:
    std::vector<std::vector<float> > ofc_a;
    std::vector<std::vector<float> > ofc_b;

    std::vector<std::vector<float> > ofcV2_a;
    std::vector<std::vector<float> > ofcV2_b;

    std::vector<std::vector<float> >shape;
    std::vector<std::vector<float> >shapeDer;
    
    float tstart;
    float timeBinWidthOFC;
    unsigned phasewMaxAt3;
    bool faultyOFC;
    bool shortWave;


    perChannelData_t(const LArWaveCumul* wave, const HWIdentifier hi, const unsigned g) : 
      inputWave(wave), chid(hi), gain(g),tstart(0), timeBinWidthOFC(25./24), phasewMaxAt3(0), faultyOFC(false), shortWave(false) {};

  };


  std::vector<perChannelData_t> m_allChannelData;

  static void           optFilt(const std::vector<float> &gWave_in, const std::vector<float>  &gDerivWave_in, const Eigen::MatrixXd& autoCorrInv, //input variables
			 std::vector<float>& OFCa, std::vector<float>& OFCb // Output variables;
			 ) ; 

  static void           optFiltDelta(const std::vector<float> &gWave_in, const std::vector<float>  &gDerivWave_in, const Eigen::MatrixXd& autoCorrInv, 
			      const Eigen::VectorXd& delta, //input variables
			      std::vector<float>& vecOFCa, std::vector<float>& vecOFCb // Output variables;
			      ) ; 

  void process(perChannelData_t&, const LArOnOffIdMapping* cabling) const;


  bool verify(const HWIdentifier chid, const std::vector<float>& OFCa, const std::vector<float>& OFCb, 
	      const std::vector<float>& Shape, const char* ofcversion, const unsigned phase) const;

  static void printOFCVec(const std::vector<float>& vec, MsgStream& mLog) ;

  
  StatusCode     initPhysWaveContainer(const LArOnOffIdMapping* cabling);
  StatusCode     initCaliWaveContainer();

    
  std::string              m_dumpOFCfile ;
  std::vector<std::string> m_keylist;
  bool                     m_verify; 
  bool                     m_normalize;
  bool                     m_timeShift;
  int                      m_timeShiftByIndex ;


  LArCaliWaveContainer*    m_waveCnt_nc=nullptr;

  unsigned int             m_nSamples;
  unsigned int             m_nPhases;
  unsigned int             m_dPhases; // number of samples between two neighboring phases (OFC sets)
  unsigned int             m_nDelays ;
  unsigned int             m_nPoints;
  float                    m_addOffset;

  ToolHandle<ILArAutoCorrDecoderTool> m_AutoCorrDecoder{this,"DecoderTool",{} };
  ToolHandle<ILArAutoCorrDecoderTool> m_AutoCorrDecoderV2{this,"DecoderToolV2", {} };

  const CaloDetDescrManager_Base* m_calo_dd_man;
  const LArOnlineID_Base*  m_onlineID; 
  const LArOFCBinComplete* m_larPhysWaveBin;

  double m_errAmpl;
  double m_errTime;

  bool                     m_readCaliWave ;
  bool                     m_fillShape ;
  std::string              m_ofcKey; 
  std::string              m_ofcKeyV2; 
  std::string              m_shapeKey; 
  bool                     m_storeMaxPhase;
  std::string              m_ofcBinKey;

  // Grouping type
  std::string              m_groupingType;
  std::string              m_larPhysWaveBinKey;

  int                      m_useDelta;
  int                      m_useDeltaV2;
  bool                     m_computeV2;
  int                      m_nThreads;

  bool                     m_readDSPConfig;
  std::string              m_DSPConfigFolder;
  std::unique_ptr<LArDSPConfig>  m_DSPConfig;

  bool                     m_forceShift;

  Eigen::VectorXd getDelta(std::vector<float>& samples, const HWIdentifier chid, unsigned nSamples) const;
 


  bool useDelta(const HWIdentifier chid, const int jobOFlag, const LArOnOffIdMapping* cabling) const;

  static const float m_fcal3Delta[5];
  static const float m_fcal2Delta[5];
  static const float m_fcal1Delta[5];


  //Functor for processing with TBB
  class  ATLAS_NOT_THREAD_SAFE Looper {
    //The way this class gets used is actually thread-safe
  public:
    Looper(std::vector<perChannelData_t>* p, const LArOnOffIdMapping* cabling, const LArOFCAlg* a) : m_perChanData(p), m_cabling(cabling), m_ofcAlg(a) {};
    void operator() (tbb::blocked_range<size_t>& r) const {
      for (size_t i=r.begin();i!=r.end();++i) {
	m_ofcAlg->process(m_perChanData->at(i),m_cabling);
      }
    }
  private:
    std::vector<perChannelData_t>* m_perChanData;
    const LArOnOffIdMapping* m_cabling;
    const LArOFCAlg* m_ofcAlg;
  };
  
  // Running on cells or supercells?
  bool m_isSC;
};


#endif

