/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MUFASTSTATIONFITTER_H
#define  TRIGL2MUONSA_MUFASTSTATIONFITTER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigT1Interfaces/RecMuonRoI.h"

#include "TrigL2MuonSA/MdtData.h"
#include "TrigL2MuonSA/TgcFitResult.h"
#include "TrigL2MuonSA/RpcFitResult.h"
#include "TrigL2MuonSA/SuperPointData.h"
#include "TrigL2MuonSA/TrackData.h"
#include "TrigL2MuonSA/MuonRoad.h"
#include "TrigL2MuonSA/PtEndcapLUT.h"
#include "TrigMuonBackExtrapolator/ITrigMuonBackExtrapolator.h"
#include "TrigL2MuonSA/AlphaBetaEstimate.h"
#include "TrigL2MuonSA/PtFromAlphaBeta.h"

namespace TrigL2MuonSA {

class MuFastStationFitter: public AthAlgTool
{
   public:
      
      static const InterfaceID& interfaceID();

   public:

      MuFastStationFitter(const std::string& type, 
                          const std::string& name,
                          const IInterface*  parent);
    
      ~MuFastStationFitter();
    
      virtual StatusCode initialize();
      virtual StatusCode finalize  ();
    
   public:
      StatusCode findSuperPoints(const LVL1::RecMuonRoI*    p_roi,
      TrigL2MuonSA::RpcFitResult& rpcFitResult,
      std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns);
      StatusCode findSuperPoints(const LVL1::RecMuonRoI*    p_roi,
      TrigL2MuonSA::TgcFitResult& tgcFitResult,
      std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns);

      StatusCode findSuperPoints(const LVL1::RecMuonRoI*    p_roi,
                                 const TrigL2MuonSA::MuonRoad& muonRoad,
                                 TrigL2MuonSA::TgcFitResult& tgcFitResult,
                                 std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns);

      StatusCode superPointFitter(TrigL2MuonSA::TrackPattern& trackPattern);

      StatusCode superPointFitter(TrigL2MuonSA::TrackPattern& trackPattern,
                                  const TrigL2MuonSA::MuonRoad&  muonRoad);

      StatusCode setMCFlag(BooleanProperty use_mcLUT);

   private:

      BooleanProperty m_use_mcLUT;
		
      Gaudi::Property< double > m_endcapinn_mdt_chi2_limit {
	this, "ENDCAPINN_MDT_CHI2_LIMIT", 20, ""};    
      Gaudi::Property< double > m_endcapmid_mdt_chi2_limit {
	this, "ENDCAPMID_MDT_CHI2_LIMIT", 20, ""};   
      Gaudi::Property< double > m_endcapout_mdt_chi2_limit {
	this, "ENDCAPOUT_MDT_CHI2_LIMIT", 20, ""};   
      Gaudi::Property< double > m_endcapee_mdt_chi2_limit {
	this, "ENDCAPEE_MDT_CHI2_LIMIT",  20, ""};    

      Gaudi::Property< double > m_rwidth_Endcapinn_first {
	this, "RWIDTH_EndcapINN_FIRST",  150, ""};  
      Gaudi::Property< double > m_rwidth_Endcapinn_second {
	this, "RWIDTH_EndcapINN_SECOND", 80, ""};  
      Gaudi::Property< double > m_rwidth_Endcapmid_first {
	this, "RWIDTH_EndcapMID_FIRST", 150, ""}; 
      Gaudi::Property< double > m_rwidth_Endcapmid_second {
	this, "RWIDTH_EndcapMID_SECOND", 100, ""}; 
      Gaudi::Property< double > m_rwidth_Endcapout_first {
	this, "RWIDTH_EndcapOUT_FIRST", 120, ""}; 
      Gaudi::Property< double > m_rwidth_Endcapout_second {
	this, "RWIDTH_EndcapOUT_SECOND", 60, ""}; 
      Gaudi::Property< double > m_rwidth_Endcapee_first {
	this, "RWIDTH_EndcapEE_FIRST", 150, ""};   
      Gaudi::Property< double > m_rwidth_Endcapee_second {
	this, "RWIDTH_EndcapEE_SECOND", 100, ""};  

      Gaudi::Property< double > m_mdt_driftspace_uplimit {
	this, "MDT_DRFITSPACE_UPLIMIT", 14.8, ""};
      Gaudi::Property< double > m_mdt_driftspace_downlimit {
	this, "MDT_DRFITSPACE_DOWNLIMIT", 0.1, ""};
      Gaudi::Property< double > m_mdt_drifttime_limit {
	this, "MDT_DRFITTIME_LIMIT", 1700, ""};

      ToolHandle<ITrigMuonBackExtrapolator> m_backExtrapolator {
	this, "BackExtrapolator", "TrigMuonBackExtrapolator", "public tool for back extrapolating the muon tracks to the IV"};

   private:
      float SetDriftSpace(float tdr, float rad, float zeta, float phim, float phiDir);
      void  Xline(float *, float *, float *, int *, int ,
                  float *, float *, float *, float *, float *, float *);
      void  Circfit (int, float *, float *, float *, float *, int *,
                     float *, float *, float DAB[2][2], float *);
      void  Circles (int, float *, float *, float *, float *, int *,
                     float *, float *, float DAB[2][2], float *, float *);
      int   Evlfit (int, TrigL2MuonSA::PBFitResult& fitres);

      StringProperty                         m_lut_fileName;
      ToolHandle<AlphaBetaEstimate>          m_alphaBetaEstimate;
      ToolHandle<PtFromAlphaBeta>            m_ptFromAlphaBeta {
	this, "PtFromAlphaBeta", "TrigL2MuonSA::PtFromAlphaBeta", ""};
      //const TrigL2MuonSA::PtEndcapLUTSvc*    m_ptEndcapLUTSvc;

      void findLayerCombination(std::vector<unsigned int> &a, int n, int r,std::vector<std::vector<unsigned int> > &c, int &nr);
      void findSubLayerCombination(std::vector<unsigned int> &a, int n,int r, std::vector<unsigned int> &b, int index ,int num,
                                   std::vector<std::vector<unsigned int> > &c, int &nr);
      void makeReferenceLine(TrigL2MuonSA::TrackPattern& trackPattern,const TrigL2MuonSA::MuonRoad&    muonRoad);
      void Circles (int, float *, float *, float *, float *, int *,
                     float *, float *, float DAB[2][2], float *, float *, float *, float *, float *);

      double fromAlphaPtToInn(TrigL2MuonSA::TgcFitResult& tgcFitResult,TrigL2MuonSA::TrackPattern& trackPattern);
      void updateInnSP(TrigL2MuonSA::TrackPattern& trackPattern, double &aw,double &tgc_aw, double &bw);
      void stationSPFit(TrigL2MuonSA::MdtHits*    mdtSegment, TrigL2MuonSA::SuperPoint* superPoint,
                        TrigL2MuonSA::PBFitResult& pbFitResult, int s_address, int i_station,double aw, float phiDir);


};

} // namespace TrigL2MuonSA

#endif  // MUFASTSTATIONFITTER_H
