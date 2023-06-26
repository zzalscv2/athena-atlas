// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


#include "EFTrackingSmearingAlg.h"
#include "xAODTracking/VertexContainer.h" 

#include "TH1.h"

#ifndef XAOD_ANALYSIS
#include "TrkEventTPCnv/helpers/EigenHelpers.h"
#include "TrkEventPrimitives/ParamDefs.h"
#else

namespace Amg
{
  // expression template to evaluate the required size of the compressed matrix at compile time
  template<int N>
  struct CalculateCompressedSize {
      static const int value = N + CalculateCompressedSize<N - 1>::value;
  }; 
  
  template<>
  struct CalculateCompressedSize<1> {
      static const int value = 1;
  };

  template<int N>
   inline void compress(const AmgSymMatrix(N)& covMatrix, std::vector<float>& vec ) {
       vec.reserve(CalculateCompressedSize<N>::value);
       for (unsigned int i = 0; i < N ; ++i)
         for (unsigned int j = 0; j <= i; ++j)
             vec.push_back(covMatrix(i,j));
     }
   inline void compress(const MatrixX& covMatrix, std::vector<float>& vec) {
       int rows = covMatrix.rows();
       for (int i = 0; i < rows; ++i) {
           for (int j = 0; j <= i; ++j) {
               vec.push_back(covMatrix(i, j));
           }
       }
   }
}

// from Tracking/TrkEventCnv/TrkEventTPCnv/TrkEventTPCnv/helpers/EigenHelpers.h
namespace EigenHelpers 
{
  template <class VECTOR, class COVARIANCE> 
   inline static void eigenMatrixToVector(VECTOR& vec, COVARIANCE& cov, const char* ) {                                                    
       Amg::compress(cov, vec);                                          
     }
}

namespace Trk
{
  enum ParamDefs        {
    // Enums for LocalParameters - LocalPosition/
                             loc1=0, loc2=1,                     //!< generic first and second local coordinate
                             locX=0, locY=1 ,                    //!< local cartesian
                             locRPhi=0, locPhiR=0, locZ=1,       //!< local cylindrical
                             locR=0, locPhi=1,                   //!< local polar
                             iPhi=0, iEta=1,                     //!< (old readout) will be skipped
                             distPhi=0, distEta=1,               //!< readout for silicon
                             driftRadius=0,                      //!< trt, straws
    // Enums for const Amg::Vector3D & GlobalMomentum /
                             x=0,   y=1,  z=2,                   //!< global position (cartesian)
                             px=0, py=1, pz=2,                   //!< global momentum (cartesian)
    // Enums for Perigee //
                             d0=0, z0=1, phi0=2, theta=3, qOverP=4, //!< perigee
 /* Enums for TrackState on Surfaces
  The first two enums in the TrackParameters refer to the local Frame, i.e.
   - LocalCartesian for AtanArbitraryPlane
   - LocalCylindrical for AtaCylinder (includes line)
   - LocalPolar for AtaDisc
   The other three enums are standard \f$(\phi, \eta, \frac{q}{p_{T}})\f$
  */
                             phi=2,
    /**Enums for curvilinear frames*/
                             u=0,  v=1,        
             
   /** Extended perigee: mass*/              
             trkMass=5
             
                             };                            //!< parameters on surface

}
#endif


EFTrackingSmearingAlg::EFTrackingSmearingAlg( const std::string& name, ISvcLocator* pSvcLocator ) 
: AthHistogramAlgorithm( name, pSvcLocator ){}


EFTrackingSmearingAlg::~EFTrackingSmearingAlg() {}


StatusCode EFTrackingSmearingAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");

  ATH_CHECK( m_inputTrackParticleKey.initialize() );
  ATH_CHECK( m_outputTrackParticleKey.initialize() );

  ATH_MSG_INFO("########## EFTrackingSmearingAlg Configurations are ########## ");
  ATH_MSG_INFO("------- InputTrackParticleKey: "<<m_inputTrackParticleKey.key());
  ATH_MSG_INFO("------- OutputTrackParticleKey: "<<m_outputTrackParticleKey.key());
  ATH_MSG_INFO("------- inputTracksPtCut [GeV]: "<<m_inputTracksPtCut);
  ATH_MSG_INFO("------- outputTracksPtCut [GeV]: "<<m_outputTracksPtCut);
  ATH_MSG_INFO("------- SmearingSigma: "<<m_SigmaScaleFactor);  
  ATH_MSG_INFO("------- trackEfficiency: "<<m_smearedTrackEfficiency);  
  ATH_MSG_INFO("------- parameterizedEfficiency: "<<m_parameterizedTrackEfficiency);  
  ATH_MSG_INFO("------- UseResolutionPtCutOff: "<<m_UseResolutionPtCutOff);
  ATH_MSG_INFO("------- SetResolutionPtCutOff: "<<m_SetResolutionPtCutOff);
  ATH_MSG_INFO("------- EnableMonitoring:" <<m_enableMonitoring);
  
  ATH_MSG_INFO("------- IncludeDuplicatesAndFakes: "<<m_EnableFakes);
  ATH_MSG_INFO("------- RandomSeed: "<<m_RandomSeed);
  ATH_MSG_INFO("------- UseCoinToss: "<<m_UseCoinToss);
  ATH_MSG_INFO("------- FakeKillerEnable: "<<m_FakeKillerEnable);
  ATH_MSG_INFO("------- IncludeFakesInResolutionCalculation: "<<m_IncludeFakesInResolutionCalculation);
  ATH_MSG_INFO("########## EFTrackingSmearingAlg Configurations: That's it. ########## ");

  ATH_MSG_INFO("Will output new track container with name "<<m_outputTrackParticleKey.key());

  // configure the Smearer
  std::string smearerName = m_outputTrackParticleKey.key()+"_smearer";
  m_mySmearer = (void *) new FakeTrackSmearer(smearerName.c_str(), m_RandomSeed,msgLvl (MSG::DEBUG));
  ((FakeTrackSmearer *) m_mySmearer)->SetInputTracksPtCut(m_inputTracksPtCut);
  ((FakeTrackSmearer *) m_mySmearer)->SetOutputTracksPtCut(m_outputTracksPtCut);
  ((FakeTrackSmearer *) m_mySmearer)->SetTrackingEfficiency(m_smearedTrackEfficiency);
  ((FakeTrackSmearer *) m_mySmearer)->SetParameterizedEfficiency(m_parameterizedTrackEfficiency);
  ((FakeTrackSmearer *) m_mySmearer)->SetSigmaScaleFactor(m_SigmaScaleFactor.value());
  ((FakeTrackSmearer *) m_mySmearer)->UseResolutionPtCutOff(m_UseResolutionPtCutOff.value());
  ((FakeTrackSmearer *) m_mySmearer)->SetResolutionPtCutOff(m_SetResolutionPtCutOff.value());

  ((FakeTrackSmearer *) m_mySmearer)->EnableFakes(m_EnableFakes.value());
  ((FakeTrackSmearer *) m_mySmearer)->UseCoinToss(m_UseCoinToss.value());
  ((FakeTrackSmearer *) m_mySmearer)->FakeKillerEnable(m_FakeKillerEnable.value());
  ((FakeTrackSmearer *) m_mySmearer)->IncludeFakesInResolutionCalculation(m_IncludeFakesInResolutionCalculation.value());
  

  

  if (m_enableMonitoring) {
    // store the smearing functions
    TF1 *d0res_eta = ((FakeTrackSmearer *) m_mySmearer)->d0res_eta;
    TF1 *z0res_eta = ((FakeTrackSmearer *) m_mySmearer)->z0res_eta;
    TF1 *d0res_pt  = ((FakeTrackSmearer *) m_mySmearer)->d0res_pt;
    TF1 *z0res_pt  = ((FakeTrackSmearer *) m_mySmearer)->z0res_pt;
    CHECK(book(new TH1F("d0res_function_vs_eta","#eta of track",100, 0.0,4.0)));
    CHECK(book(new TH1F("z0res_function_vs_eta","#eta of track",100, 0.0,4.0)));
    CHECK(book(new TH1F("d0res_function_vs_pt","#pt of track",100, 4.0,200.0)));
    CHECK(book(new TH1F("z0res_function_vs_pt","#pt of track",100, 4.0,200.0)));
    hist("d0res_function_vs_eta")->Add(d0res_eta);
    hist("z0res_function_vs_eta")->Add(z0res_eta);
    hist("d0res_function_vs_pt")->Add(d0res_pt);
    hist("z0res_function_vs_pt")->Add(z0res_pt);
    // book historgams
    CHECK(book_histograms());
  }
  return StatusCode::SUCCESS;
}

StatusCode EFTrackingSmearingAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  delete((FakeTrackSmearer *)m_mySmearer);
  return StatusCode::SUCCESS;
}

StatusCode EFTrackingSmearingAlg::execute() {  
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  
  SG::ReadHandle<xAOD::TrackParticleContainer> inputTracks_handle( m_inputTrackParticleKey, getContext() );
  const xAOD::TrackParticleContainer* inputTracks = inputTracks_handle.cptr();
  if (not inputTracks) {
     ATH_MSG_FATAL("Unable to retrieve input ID tacks");
     return StatusCode::FAILURE;
  }

  SG::WriteHandle<xAOD::TrackParticleContainer> outputTracks_handle( m_outputTrackParticleKey, getContext() );
  ATH_CHECK( outputTracks_handle.record( std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>() ) );
  auto outputTracks = outputTracks_handle.ptr();
  
  // clear the smearear
  FakeTrackSmearer *mySmearer=(FakeTrackSmearer *) m_mySmearer;
  mySmearer->Clear();

  int trackno=0;
  int n_input_tracks=0;
  int n_output_tracks=0;
  int n_output_broad_tracks=0;
  int n_output_narrow_tracks=0;
  ATH_MSG_DEBUG ("Found "<<inputTracks->size()<< " input tracks");
  for ( const auto* trk : *inputTracks ) 
    {            
      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();
      double theta=trk->theta();
      double pt = TMath::Sin(theta)/(1000.*trk->qOverP());// GeV
      float qoverPt = trk->qOverP()*1000./TMath::Sin(theta); // GeV
      ATH_MSG_DEBUG ("===> New Track: parameters: d0=" << trk->d0()
                      <<" z0="     << trk->z0()
                      <<" qoverP=" << trk->qOverP()<<" pt(GeV)="<<pt << " curvT="<<qoverPt
                      <<" eta="    << trk->eta()
                      <<" phi="    << trk->phi0()
                      <<" cov d0=" << trkcov(Trk::d0,Trk::d0)
                      <<" cov z0=" << trkcov(Trk::z0,Trk::z0)
                      <<" sigma d0="  << TMath::Sqrt(TMath::Abs(trkcov(Trk::d0,Trk::d0)))
                      <<" sigma z0="  << TMath::Sqrt(TMath::Abs(trkcov(Trk::z0,Trk::z0))) );
      
      
      if (TMath::Abs(pt) > m_inputTracksPtCut)
      	{
      	  n_input_tracks++;
          if (m_enableMonitoring) {
      	    hist("track_input_eta")->Fill(trk->eta());
      	    hist("track_input_theta")->Fill(trk->theta());
      	    hist("track_input_pt" )->Fill(pt);
      	    hist("track_input_phi")->Fill(trk->phi0());
      	    hist("track_input_z0" )->Fill(trk->z0());
      	    hist("track_input_d0" )->Fill(trk->d0());
      	
      	    hist("track_input_sigma_theta") ->Fill(TMath::Sqrt(trkcov(Trk::theta,Trk::theta)));
      	    hist("track_input_sigma_qOverP")->Fill(TMath::Sqrt(trkcov(Trk::qOverP,Trk::qOverP)));
      	    hist("track_input_sigma_phi")   ->Fill(TMath::Sqrt(trkcov(Trk::phi0,Trk::phi0)));
      	    hist("track_input_sigma_z0")    ->Fill(TMath::Sqrt(trkcov(Trk::z0,Trk::z0)));
      	    hist("track_input_sigma_d0")    ->Fill(TMath::Sqrt(trkcov(Trk::d0,Trk::d0)));
          }
      	  
          
          mySmearer->AddTrack(trk->d0(),trk->z0(),qoverPt,trk->eta(),trk->phi0());                    
      	  n_output_tracks += mySmearer->GetNTracks();
      	  
      	  ATH_MSG_DEBUG ("Looping on output track #"<< n_output_tracks);
      	  for (int i=0;i<mySmearer->GetNTracks();i++)
      	    {
      	      xAOD::TrackParticle * newtrk = new xAOD::TrackParticle(*trk);
      	      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();
      
      	      double sintheta=TMath::Sin(mySmearer->Tracks[i].theta());
      	      for (int ii=0;ii<5;ii++) for (int jj=0;jj<5;jj++) if (ii!=jj) trkcov(ii,jj)=0.0;
      	      // not sure about the position of the elements below: need to cross check 
      	      trkcov(Trk::d0    ,Trk::d0    ) = mySmearer->Tracks[i].sigma_d0()   * mySmearer->Tracks[i].sigma_d0()   ; 
      	      trkcov(Trk::z0    ,Trk::z0    ) = mySmearer->Tracks[i].sigma_z0()   * mySmearer->Tracks[i].sigma_z0()   ; 
      	      trkcov(Trk::theta ,Trk::theta ) = mySmearer->Tracks[i].sigma_theta()* mySmearer->Tracks[i].sigma_theta(); 
      	      trkcov(Trk::phi0  ,Trk::phi0  ) = mySmearer->Tracks[i].sigma_phi()  * mySmearer->Tracks[i].sigma_phi()  ; 
      	      trkcov(Trk::qOverP,Trk::qOverP) = mySmearer->Tracks[i].sigma_curv() * mySmearer->Tracks[i].sigma_curv() *sintheta*sintheta; 
              if (m_enableMonitoring) {
      	        hist("track_outputcoll_sigma_theta") ->Fill(TMath::Sqrt(trkcov(Trk::theta,Trk::theta)));
      	        hist("track_outputcoll_sigma_qOverP")->Fill(TMath::Sqrt(trkcov(Trk::qOverP,Trk::qOverP)));
      	        hist("track_outputcoll_sigma_phi")   ->Fill(TMath::Sqrt(trkcov(Trk::phi0,Trk::phi0)));
      	        hist("track_outputcoll_sigma_z0")    ->Fill(TMath::Sqrt(trkcov(Trk::z0,Trk::z0)));
      	        hist("track_outputcoll_sigma_d0")    ->Fill(TMath::Sqrt(trkcov(Trk::d0,Trk::d0)));
              }

      	      ATH_MSG_DEBUG ("Setting parameters covariance vector"); 
              auto trkcovvec = trk->definingParametersCovMatrixVec();
      	      trkcovvec.clear();
      	      EigenHelpers::eigenMatrixToVector(trkcovvec,trkcov,"");
      	      newtrk->setDefiningParametersCovMatrixVec(trkcovvec);
      
      	      ATH_MSG_DEBUG ("Setting parameters covariance");
      	      newtrk->setDefiningParametersCovMatrix(trkcov);                             


      	      ATH_MSG_DEBUG ("Smeared track: d0="  << mySmearer->Tracks[i].d0()
                      <<" z0="     << mySmearer->Tracks[i].z0()
                      <<" qoverPt (curv)=" << mySmearer->Tracks[i].curv()
                      <<" pT="     << mySmearer->Tracks[i].pt()
                      <<" pT (inverse curv)=" << 1./mySmearer->Tracks[i].curv()  
                      <<" qoverP="  << mySmearer->Tracks[i].curv()*sintheta
                      <<" eta="    << mySmearer->Tracks[i].eta()
                      <<" phi="    << mySmearer->Tracks[i].phi() );
                      
              //(float d0, float z0, float phi0, float theta, float qOverP)
              newtrk->setDefiningParameters(
      					    mySmearer->Tracks[i].d0(),
      					    mySmearer->Tracks[i].z0(),
      					    mySmearer->Tracks[i].phi(),
      					    mySmearer->Tracks[i].theta(),
      					    (mySmearer->Tracks[i].curv()*sintheta)
      					    );
              
      	      if (m_enableMonitoring) {
      	        hist("track_output_eta")->Fill(mySmearer->Tracks[i].eta());
      	        hist("track_output_theta")->Fill(mySmearer->Tracks[i].theta());
      	        hist("track_output_pt" )->Fill(mySmearer->Tracks[i].pt()/1000.0 );
      	        hist("track_output_phi")->Fill(mySmearer->Tracks[i].phi());
      	        hist("track_output_z0" )->Fill(mySmearer->Tracks[i].z0() );
      	        hist("track_output_d0" )->Fill(mySmearer->Tracks[i].d0() );      
      
      	        hist("track_outputcoll_eta")->Fill(newtrk->eta());
      	        hist("track_outputcoll_theta")->Fill(newtrk->theta());
      	        hist("track_outputcoll_pt" )->Fill(TMath::Sin(newtrk->theta())/(1000.0*newtrk->qOverP()));
      	        hist("track_outputcoll_phi")->Fill(newtrk->phi0());
      	        hist("track_outputcoll_z0" )->Fill(newtrk->z0());
      	        hist("track_outputcoll_d0" )->Fill(newtrk->d0());
      	      
      	        hist("track_delta_eta")->Fill(mySmearer->Tracks[i].eta() - trk->eta());      	      
      	        hist("track_delta_crv")->Fill(1000.0*mySmearer->Tracks[i].curv()-((1000.0*trk->qOverP())/TMath::Sin(theta)));
      	        hist("track_delta_phi")->Fill(mySmearer->Tracks[i].phi() - trk->phi0());
      	        hist("track_delta_z0" )->Fill(mySmearer->Tracks[i].z0()  - trk->z0());
      	        hist("track_delta_d0" )->Fill(mySmearer->Tracks[i].d0()  - trk->d0());
              
      	        xAOD::ParametersCovMatrix_t trkcov_out = newtrk->definingParametersCovMatrix();
      	        hist("track_outputcoll_check_sigma_theta") ->Fill(TMath::Sqrt(trkcov_out(Trk::theta,Trk::theta)));
      	        hist("track_outputcoll_check_sigma_qOverP")->Fill(TMath::Sqrt(trkcov_out(Trk::qOverP,Trk::qOverP)));
      	        hist("track_outputcoll_check_sigma_phi")   ->Fill(TMath::Sqrt(trkcov_out(Trk::phi0,Trk::phi0)));
      	        hist("track_outputcoll_check_sigma_z0")    ->Fill(TMath::Sqrt(trkcov_out(Trk::z0,Trk::z0)));
      	        hist("track_outputcoll_check_sigma_d0")    ->Fill(TMath::Sqrt(trkcov_out(Trk::d0,Trk::d0)));
              }
              // do we need this hack?
      	      // hack!!!
      	      // Chi2: 1 -> "Broad Fake"
      	      //       0 -> "core tracks"
      	      //
      	      // NDF: index of parent track in input track collection 
      	      newtrk->setFitQuality(mySmearer->Tracks[i].FakeFlags(),trackno);
      
      	      outputTracks->push_back(newtrk);              
      	    }
            
	      }
        mySmearer->Clear(); // clear teh smearer after each input track
        trackno++;
      }

  ATH_MSG_DEBUG ("End of loop track #"<<n_input_tracks<<" ---> "<<" "<< n_output_tracks
              <<" "<<n_output_narrow_tracks<<" "<<n_output_broad_tracks);
  if (m_enableMonitoring) {
    hist("n_input_tracks")->Fill(n_input_tracks);
    hist("n_output_tracks")->Fill(n_output_tracks);
    hist("n_output_narrow_tracks")->Fill(n_output_narrow_tracks);
    hist("n_output_broad_tracks")->Fill(n_output_broad_tracks);
  }
  return StatusCode::SUCCESS;
}




StatusCode EFTrackingSmearingAlg::book_histograms()
{
  double b_eta[3]={50,-5,5};
  CHECK(book(new TH1F("track_input_eta","#eta of input tracks",b_eta[0],b_eta[1],b_eta[2])));
  CHECK(book(new TH1F("track_input_theta","#Theta of input tracks",50,0.0,4.0)));
  CHECK(book(new TH1F("track_input_pt" ,"p_T  of input tracks",50.0,-10.0,10.0)));
  CHECK(book(new TH1F("track_input_phi","#phi of input tracks",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_input_z0" ,"z_0  of input tracks",100,-50,50)));
  CHECK(book(new TH1F("track_input_d0" ,"d_0  of input tracks",50,-5,5)));

  CHECK(book(new TH1F("track_input_sigma_theta","#sigma_{#Theta} of input tracks"   ,50,0.0,0.001)));
  CHECK(book(new TH1F("track_input_sigma_qOverP"   ,"#sigma_{q/P}  of input tracks" ,50.0,0.0,0.0001)));
  CHECK(book(new TH1F("track_input_sigma_phi"  ,"#sigma_{#phi} of input tracks"     ,50,0.0,0.002)));
  CHECK(book(new TH1F("track_input_sigma_z0"   ,"#sigma_{z_0}  of input tracks"     ,100,0.0,5   )));
  CHECK(book(new TH1F("track_input_sigma_d0"   ,"#sigma_{d_0}  of input tracks"     ,50,0.0,5    )));
  
  int maxtracks=500;
  CHECK(book(new TH1F("n_input_tracks"       ,"Number of input tracks"            ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_tracks"      ,"Number of output tracks"           ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_narrow_tracks","Number of output tracks (narrow)" ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_broad_tracks"  ,"Number of output tracks (broad)" ,maxtracks,0,maxtracks)));

  CHECK(book(new TH1F("track_output_eta","#eta of output tracks",50,-5,5)));
  CHECK(book(new TH1F("track_output_theta","#Theta of output tracks",50,0.0,4.0)));
  CHECK(book(new TH1F("track_output_pt" ,"p_T  of output tracks",50,-10,10)));
  CHECK(book(new TH1F("track_output_phi","#phi of output tracks",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_output_z0" ,"z_0  of output tracks",100,-50,50)));
  CHECK(book(new TH1F("track_output_d0" ,"d_0  of output tracks",50,-5,5)));

  CHECK(book(new TH1F("track_outputcoll_eta","#eta of output tracks collection",50,-5,5)));
  CHECK(book(new TH1F("track_outputcoll_theta","#Theta of output tracks collection",50,0.0,4.0)));
  CHECK(book(new TH1F("track_outputcoll_pt" ,"p_T  of output tracks collection",50,-10,10)));
  CHECK(book(new TH1F("track_outputcoll_phi","#phi of output tracks collection",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_outputcoll_z0" ,"z_0  of output tracks collection",100,-50,50)));
  CHECK(book(new TH1F("track_outputcoll_d0" ,"d_0  of output tracks collection",50,-5,5)));

  CHECK(book(new TH1F("track_outputcoll_sigma_theta","#sigma_{#Theta} of output tracks collection"  ,50,0.0,0.001)));
  CHECK(book(new TH1F("track_outputcoll_sigma_qOverP"   ,"#sigma_{q/P}  of output tracks collection",50.0,0.0,0.0001)));
  CHECK(book(new TH1F("track_outputcoll_sigma_phi"  ,"#sigma_{#phi} of output tracks collection"    ,50,0.0,0.002)));
  CHECK(book(new TH1F("track_outputcoll_sigma_z0"   ,"#sigma_{z_0}  of output tracks collection"    ,100,0.0,5   )));
  CHECK(book(new TH1F("track_outputcoll_sigma_d0"   ,"#sigma_{d_0}  of output tracks collection"    ,50,0.0,5    )));

  CHECK(book(new TH1F("track_outputcoll_check_sigma_theta","#sigma_{#Theta} of output tracks collection"  ,50,0.0,0.001)));
  CHECK(book(new TH1F("track_outputcoll_check_sigma_qOverP"   ,"#sigma_{q/P}  of output tracks collection",50.0,0.0,0.0001)));
  CHECK(book(new TH1F("track_outputcoll_check_sigma_phi"  ,"#sigma_{#phi} of output tracks collection"    ,50,0.0,0.002)));
  CHECK(book(new TH1F("track_outputcoll_check_sigma_z0"   ,"#sigma_{z_0}  of output tracks collection"    ,100,0.0,5   )));
  CHECK(book(new TH1F("track_outputcoll_check_sigma_d0"   ,"#sigma_{d_0}  of output tracks collection"    ,50,0.0,5    )));

  CHECK(book(new TH1F("track_delta_eta","tracks #Delta #eta",50,-1,1)));
  CHECK(book(new TH1F("track_delta_crv" ,"tracks #Delta crv",50,-1,1)));
  CHECK(book(new TH1F("track_delta_phi","tracks #Delta #phi",50,-0.5,0.5)));
  CHECK(book(new TH1F("track_delta_z0" ,"tracks #Delta z_0 ",100,-50,50)));
  CHECK(book(new TH1F("track_delta_d0" ,"tracks #Delta d_0 ",50,-10,10)));
  return StatusCode::SUCCESS;
}



