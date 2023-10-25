/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//
// NewVrtSecInclusiveTool.h - Description
//
/*
   Tool for inclusive secondary vertex reconstruction
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Tool creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _VKalVrt_NewVrtSecInclusiveTool_H
#define _VKalVrt_NewVrtSecInclusiveTool_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "boost/graph/adjacency_list.hpp"
//
#include "xAODTruth/TruthEventContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "NewVrtSecInclusiveTool/IVrtInclusive.h"

class TH1D;
class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

namespace Trk{
  class TrkVKalVrtFitter;
  class IVertexFitter;
  class IVKalState;
}
 
namespace MVAUtils { class BDT; }

 
//------------------------------------------------------------------------
namespace Rec {

  struct workVectorArrxAOD{
        std::vector<const xAOD::TrackParticle*> listSelTracks;  // Selected tracks after quality cuts
        std::vector<const xAOD::TrackParticle*> tmpListTracks;
        std::vector<const xAOD::TrackParticle*> inpTrk;         // All tracks provided to tool
        double beamX=0.;
        double beamY=0.;
        double beamZ=0.;
        double tanBeamTiltX=0.;
        double tanBeamTiltY=0.;
  };

  class NewVrtSecInclusiveTool : public AthAlgTool, virtual public IVrtInclusive
  {


  public:
       /* Constructor */
      NewVrtSecInclusiveTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~NewVrtSecInclusiveTool();


      StatusCode initialize();
      StatusCode finalize();



      std::unique_ptr<Trk::VxSecVertexInfo> findAllVertices(const std::vector<const xAOD::TrackParticle*> & inputTracks,
                                                                                     const xAOD::Vertex & primaryVertex) const final;
//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

    private:

      double m_w_1{};
      struct DevTuple;
      struct Hists {
        StatusCode book (ITHistSvc& histSvc, const std::string& histDir);
        TTree* m_tuple{};
        DevTuple*  m_curTup;
        TH1D* m_hb_massPiPi{};
        TH1D* m_hb_massPiPi1{};
        TH1D* m_hb_massPPi{};
        TH1D* m_hb_massEE{};
        TH1D* m_hb_nvrt2{};
        TH1D* m_hb_ratio{};
        TH1D* m_hb_totmass{};
        TH1D* m_hb_impact{};
        TH1D* m_hb_impactR{};
        TH2D* m_hb_impactRZ{};
        TH1D* m_hb_trkD0{};
        TH1D* m_hb_trkZ{};
        TH1F* m_hb_ntrksel{};
        TH1F* m_hb_ntrkInput{};
        TH1F* m_hb_trkSelect{};
        TH1D* m_hb_impactZ{};
        TH1D* m_hb_r2d{};
        TH1D* m_hb_signif3D{};
        TH1D* m_hb_impV0{};
        TH1D* m_hb_sig3DTot{};
        TH1F* m_hb_goodvrtN{};
        TH1F* m_hb_goodvrt1N{};
        TH1D* m_hb_distVV{};
        TH1D* m_hb_diffPS{};
        TH1D* m_hb_sig3D1tr{};
        TH1D* m_hb_sig3D2tr{};
        TH1D* m_hb_sig3DNtr{};
        TH1F* m_hb_rawVrtN{};
        TH1F* m_hb_cosSVMom{};
        TH1F* m_hb_etaSV{};
        TH1F* m_hb_fakeSVBDT{};
      };
      std::unique_ptr<Hists> m_h;
//--

      long int m_cutSctHits{};
      long int m_cutPixelHits{};
      long int m_cutTRTHits{};
      long int m_cutSiHits{};
      long int m_cutBLayHits{};
      long int m_cutSharedHits{};
      double m_cutPt{};
      double m_cutZVrt{};
      double m_cutD0Max{};
      double m_cutD0Min{};
      double m_cutChi2{};
      double m_sel2VrtProbCut{};
      double m_globVrtProbCut{};
      double m_maxSVRadiusCut{};
      double m_selVrtSigCut{};
      double m_trkSigCut{};
      float m_vrtMassLimit{};
      float m_vrt2TrMassLimit{};
      float m_vrt2TrPtLimit{};
      float m_antiPileupSigRCut{};
      float m_dRdZRatioCut{};
      float m_v2tIniBDTCut{};
      float m_v2tFinBDTCut{};
      float m_vertexMergeCut{};
      float m_beampipeR{};
      float m_firstPixelLayerR{};
      float m_removeTrkMatSignif{};
      float m_fastZSVCut{};
      float m_cosSVPVCut{};

      bool m_fillHist{};
      bool m_useVertexCleaning{};
      bool m_multiWithOneTrkVrt{};
      std::string m_calibFileName;

      std::unique_ptr<MVAUtils::BDT> m_SV2T_BDT;

      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

      ToolHandle<Trk::IExtrapolator>  m_extrapolator{this,"ExtrapolatorName","Trk::Extrapolator/Extrapolator"};
      ToolHandle<Trk::TrkVKalVrtFitter>  m_fitSvc;
      //Trk::TrkVKalVrtFitter*   m_fitSvc{};

      double m_massPi {};
      double m_massP {};
      double m_massE{};
      double m_massK0{};
      double m_massLam{};
      std::string m_instanceName;

//=======================================================================================
// Functions and structure below are for algorithm development, debugging and calibration
// NOT USED IN PRODUCTION!

     int notFromBC(int PDGID) const;
     const xAOD::TruthParticle * getPreviousParent(const xAOD::TruthParticle * child, int & ParentPDG) const;
     int getIdHF(const xAOD::TrackParticle* TP ) const;
     int getG4Inter( const xAOD::TrackParticle* TP ) const;
     int getMCPileup(const xAOD::TrackParticle* TP ) const;

     struct DevTuple 
     { 
       static constexpr int maxNTrk=100;
       static constexpr int maxNVrt=100;
       int   nTrk;
       float pttrk[maxNTrk];
       float d0trk[maxNTrk];
       float etatrk[maxNTrk];
       float Sig3D[maxNTrk];
       float dRdZrat[maxNTrk];
       int   idHF[maxNTrk];
       int   trkTRT[maxNTrk];
       int   n2Vrt;
       int   VrtTrkHF[maxNVrt];
       int   VrtTrkI[maxNVrt];
       int   VrtCh[maxNVrt];
       int   VrtIBL[maxNVrt];
       int   VrtBL[maxNVrt];
       int   VrtDisk[maxNVrt];
       float VrtDist2D[maxNVrt];
       float VrtSig3D[maxNVrt];
       float VrtSig2D[maxNVrt];
       float VrtM[maxNVrt];
       float VrtZ[maxNVrt];
       float VrtPt[maxNVrt];
       float VrtEta[maxNVrt];
       float VrtBDT[maxNVrt];
       float VrtProb[maxNVrt];
       float VrtHR1[maxNVrt];
       float VrtHR2[maxNVrt];
       float VrtDZ[maxNVrt];
       float VrtCosSPM[maxNVrt];
       float VMinPtT[maxNVrt];
       float VMinS3DT[maxNVrt];
       float VMaxS3DT[maxNVrt];
       float VSigMat[maxNVrt];
       //---
       int   nNVrt;
       int   NVrtTrk[maxNVrt];
       int   NVrtTrkHF[maxNVrt];
       int   NVrtTrkI[maxNVrt];
       int   NVrtCh[maxNVrt];
       int   NVrtIBL[maxNVrt];
       int   NVrtBL[maxNVrt];
       float NVrtM[maxNVrt];
       float NVrtPt[maxNVrt];
       float NVrtEta[maxNVrt];
       float NVrtCosSPM[maxNVrt];
       float NVMinPtT[maxNVrt];
       float NVMinS3DT[maxNVrt];
       float NVMaxS3DT[maxNVrt];
       float NVrtDist2D[maxNVrt];
       float NVrtSig3D[maxNVrt];
       float NVrtSig2D[maxNVrt];
       float NVrtProb[maxNVrt];
       float NVrtBDT[maxNVrt];
       float NVrtHR1[maxNVrt];
       float NVrtHR2[maxNVrt];
     };
//
// End of development stuff
//============================================================


     struct Vrt2Tr 
     {
         Amg::Vector3D     fitVertex;
         TLorentzVector    momentum;
         long int   vertexCharge;
         std::vector<double> errorMatrix;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2=0.;
     };


// For multivertex version only

      using compatibilityGraph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>;
      float m_chiScale[11]{};
      struct WrkVrt 
      {  bool Good=true;
         std::deque<long int> selTrk;
         Amg::Vector3D     vertex;
         TLorentzVector    vertexMom;
         long int   vertexCharge{};
         std::vector<double> vertexCov;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2{};
         double projectedVrt=0.;
         int detachedTrack=-1;
         double BDT=1.1;
      };


//   Private technical functions
//
//
      std::vector<xAOD::Vertex*> getVrtSecMulti(  workVectorArrxAOD * inpParticlesxAOD, const xAOD::Vertex  & primVrt,
                                                  compatibilityGraph_t& compatibilityGraph ) const;


      void printWrkSet(const std::vector<WrkVrt> * WrkSet, const std::string &name ) const;

//
// Gives correct mass assignment in case of nonequal masses
      double massV0(const std::vector< std::vector<double> >& TrkAtVrt, double massP, double massPi ) const;


      TLorentzVector momAtVrt(const std::vector<double>& inpTrk) const; 
      double  vrtRadiusError(const Amg::Vector3D & secVrt, const std::vector<double>  & vrtErr) const;

      int   nTrkCommon( std::vector<WrkVrt> *WrkVrtSet, int indexV1, int indexV2) const;
      double minVrtVrtDist( std::vector<WrkVrt> *WrkVrtSet, int & indexV1, int & indexV2, std::vector<double> & check) const;
      bool isPart( const std::deque<long int>& test, std::deque<long int> base) const;
      std::vector<double> estimVrtPos( int nTrk, std::deque<long int> &selTrk, std::map<long int,std::vector<double>> & vrt) const;

      double vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                  const std::vector<double>& vrtErr,double& signif ) const;
      double vrtVrtDist2D(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                  const std::vector<double>& vrtErr,double& signif ) const;
      double vrtVrtDist(const Amg::Vector3D & vrt1, const std::vector<double>& vrtErr1,
                        const Amg::Vector3D & vrt2, const std::vector<double>& vrtErr2) const;
      double PntPntDist(const Amg::Vector3D & Vrt1, const Amg::Vector3D & Vrt2) const;


      double projSV_PV(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Direction) const;
      double MomProjDist(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Direction) const;

      double distToMatLayerSignificance(Vrt2Tr & Vrt) const;

      double refitVertex( WrkVrt &Vrt,std::vector<const xAOD::TrackParticle*> & SelectedTracks,
                          Trk::IVKalState& istate,
                          bool ifCovV0) const;

      int mostHeavyTrk(WrkVrt V, std::vector<const xAOD::TrackParticle*> AllTracks) const;
      double refineVerticesWithCommonTracks( WrkVrt &v1, WrkVrt &v2, std::vector<const xAOD::TrackParticle*> & allTrackList,
                                                        Trk::IVKalState& istate) const;
      double mergeAndRefitVertices( WrkVrt & v1, WrkVrt & v2, WrkVrt & newvrt,
                                    std::vector<const xAOD::TrackParticle*> & AllTrackList,
                                    Trk::IVKalState& istate, int robKey =0) const;

      double  improveVertexChi2( WrkVrt &vertex, std::vector<const xAOD::TrackParticle*> & allTracks,
                                 Trk::IVKalState& istate,
                                 bool ifCovV0) const;

      void selGoodTrkParticle( workVectorArrxAOD * xAODwrk,
                               const xAOD::Vertex  & primVrt) const;



      void select2TrVrt(std::vector<const xAOD::TrackParticle*> & SelectedTracks, const xAOD::Vertex  & primVrt,
                        std::map<long int,std::vector<double>> & vrt,
                        compatibilityGraph_t& compatibilityGraph) const;


     void  getPixelDiscs   (const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) const;
     int   getIBLHit(const xAOD::TrackParticle* Part) const;
     int   getBLHit(const xAOD::TrackParticle* Part) const;

     Hists& getHists() const;
   };


  struct clique_visitor
  {
    clique_visitor(std::vector< std::vector<int> > & input): m_allCliques(input){ input.clear();}
    
    template <typename Clique, typename Graph>
    void clique(const Clique& clq, Graph& )
    { 
      std::vector<int> new_clique(0);
      for(auto i = clq.begin(); i != clq.end(); ++i) new_clique.push_back(*i);
      m_allCliques.push_back(new_clique);
    }

    std::vector< std::vector<int> > & m_allCliques;

  };


}  //end namespace

#endif
