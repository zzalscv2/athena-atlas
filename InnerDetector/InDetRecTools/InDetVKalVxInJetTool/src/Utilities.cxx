/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)

// Header include
#include "InDetVKalVxInJetTool/InDetVKalVxInJetTool.h"
#include "TrkNeutralParameters/NeutralParameters.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "CxxUtils/sincos.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "AtlasHepMC/MagicNumbers.h"
//-------------------------------------------------
// Other stuff
#include <cmath>


namespace InDet{  

  double InDetVKalVxInJetTool::rankBTrk(double TrkPt, double JetPt, double Signif)  const
  {
    double p_prob=0.;
    double s_prob=0.;
    if(TrkPt > 0. &&  JetPt > 0.){
      double coeffPt=10.;
      double pfrac=(TrkPt-m_cutPt)/std::sqrt(JetPt);
      p_prob= pfrac/(coeffPt+pfrac);    // Old probability to be b-track
      if (Signif == 0.) return p_prob; //should be less than some epsilon?
    } 
    if( Signif != 0.) {
      double coeffSig=1.0;
      s_prob=(Signif-coeffSig)/Signif;   // Old probability to be b-track
      if(TrkPt + JetPt == 0.) return s_prob;
    }
    //----------------------------------Initial definition of selective variable
    double contrib=0.4;
    return (1.+contrib)*std::max(s_prob,0.)+(1.-contrib)*p_prob;
  }


  TLorentzVector  InDetVKalVxInJetTool::getBDir( const xAOD::TrackParticle* trk1,
                                                 const xAOD::TrackParticle* trk2,
                                                 const xAOD::Vertex    & PrimVrt,
						 Amg::Vector3D &V1, Amg::Vector3D &V2)
  const
  { // B hadron flight direction based on 2 separate tracks and PV. Calculated via plane-plane crossing
    Amg::Vector3D PVRT(PrimVrt.x(),PrimVrt.y(),PrimVrt.z());
//----------------------------------------------------------------------------
    Amg::Vector3D   pnt1=trk1->perigeeParameters().position()-PVRT;
    Amg::Vector3D   mom1((trk1->p4()).Px(),(trk1->p4()).Py(),(trk1->p4()).Pz());
    Amg::Vector3D   pnt2=trk2->perigeeParameters().position()-PVRT;
    Amg::Vector3D   mom2((trk2->p4()).Px(),(trk2->p4()).Py(),(trk2->p4()).Pz());
    pnt1.normalize(); pnt2.normalize(); mom1.normalize(); mom2.normalize();
//------------------------------------------------------------------------
    const double dRLim=m_coneForTag;
    Amg::Vector3D norm1=pnt1.cross(mom1);
    Amg::Vector3D norm2=pnt2.cross(mom2);
    Amg::Vector3D t=norm1.cross(norm2); t.normalize(); if(t.dot(mom1+mom2)<0.) t*=-1.;
    double aveP=(trk1->p4()+trk2->p4()).P()/2.;
    TLorentzVector tl;  tl.SetXYZM(t.x()*aveP,t.y()*aveP,t.z()*aveP,139.57); //Crossing line of 2 planes
    if( tl.DeltaR(trk1->p4()) >dRLim || tl.DeltaR(trk2->p4()) >dRLim ) {V1*=0.; V2*=0.; return tl;}//Too big dR between tracks and found "B line"
//------------------------------------------------------------------------
    double X;
    pnt1=trk1->perigeeParameters().position()-PVRT;
    pnt2=trk2->perigeeParameters().position()-PVRT;
    std::abs(mom1[1]*t[2]-mom1[2]*t[1])>std::abs(mom1[0]*t[2]-mom1[2]*t[0]) ? X=(t[1]*pnt1[2]-t[2]*pnt1[1])/(mom1[1]*t[2]-mom1[2]*t[1])
                                                                            : X=(t[0]*pnt1[2]-t[2]*pnt1[0])/(mom1[0]*t[2]-mom1[2]*t[0]);
    V1=pnt1+mom1*X;   // First particle vertex
    std::abs(mom2[1]*t[2]-mom2[2]*t[1])>std::abs(mom2[0]*t[2]-mom2[2]*t[0]) ? X=(t[1]*pnt2[2]-t[2]*pnt2[1])/(mom2[1]*t[2]-mom2[2]*t[1])
                                                                            : X=(t[0]*pnt2[2]-t[2]*pnt2[0])/(mom2[0]*t[2]-mom2[2]*t[0]);
    V2=pnt2+mom2*X;   // Second particle vertex
//------------------------------------------------------------------------
    if(V1.dot(t)<0. && V2.dot(t)<0.) {V1*=0.;V2*=0.;}        // Check correctness of topology
    else                             {V1+=PVRT; V2+=PVRT;}   // Transform to detector frame
//------------------------------------------------------------------------
    return tl;
  }

  void InDetVKalVxInJetTool::printWrkSet(const std::vector<WrkVrt> *wrkVrtSet, const std::string& name) const {
   int nGoodV=0;
   for(const auto & iv : *wrkVrtSet) {
      std::ostringstream ostr1,ostr2;
      for(long kk : iv.selTrk) {ostr1<<kk<<", ";}
      for(int kk=0; kk<(int)iv.selTrk.size(); kk++) {ostr2<<momAtVrt(iv.trkAtVrt[kk]).Perp()<<", ";}
      ATH_MSG_DEBUG(name
      <<"= "<<iv.vertex[0]
      <<", "<<iv.vertex[1]
      <<", "<<iv.vertex[2]
      <<" NTrk="<<iv.selTrk.size()
      <<" is good="<<std::boolalpha<<iv.Good<<std::noboolalpha
      <<"  Chi2="<<iv.chi2
      <<"  Mass="<<iv.vertexMom.M()
      <<"  detached="<<iv.detachedTrack
      <<"  proj.dist="<<iv.projectedVrt
      <<" trk="<<ostr1.str()<<" trk Pt="<<ostr2.str());
      if(iv.Good)nGoodV++;
    }
   ATH_MSG_DEBUG(name<<" N="<<nGoodV);
  }

               /*  Technicalities */
  double InDetVKalVxInJetTool::projSV_PV(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Jet) 
  {  
     TVector3 SV_PV( SV.x()-PV.x(), SV.y()-PV.y(), SV.z()-PV.z() );
     return Jet.Vect().Unit()*SV_PV.Unit();
  }

  bool InDetVKalVxInJetTool::insideMatLayer(float xvt,float yvt) const
  {
     float R = std::hypot(xvt, yvt);
     if(m_existIBL){              // 4-layer pixel detector
       if( std::abs(R-m_beampipeR)< 1.0) return true; // Beam Pipe removal
       if( std::abs(R-m_rLayerB)  < 2.5) return true;
       if( std::abs(R-m_rLayer1)  < 3.0) return true;
       if( std::abs(R-m_rLayer2)  < 3.0) return true;
     }else{                       // 3-layer pixel detector
       if( std::abs(R-m_beampipeR)< 1.5) return true; // Beam Pipe removal
       if( std::abs(R-m_rLayerB)  < 3.5) return true;
       if( std::abs(R-m_rLayer1)  < 4.0) return true;
       if( std::abs(R-m_rLayer2)  < 5.0) return true;
     }
     return false;
  }
  
  double InDetVKalVxInJetTool::vrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                          const std::vector<double>& SecVrtErr, double& Signif)
  const
  {

    Amg::Vector3D SVPV(PrimVrt.x()- SecVrt.x(),PrimVrt.y()- SecVrt.y(),PrimVrt.z()- SecVrt.z());

    AmgSymMatrix(3)  PrimCovMtx=PrimVrt.covariancePosition();  //Create
    PrimCovMtx(0,0) += SecVrtErr[0];
    PrimCovMtx(0,1) += SecVrtErr[1];
    PrimCovMtx(1,0) += SecVrtErr[1];
    PrimCovMtx(1,1) += SecVrtErr[2];
    PrimCovMtx(0,2) += SecVrtErr[3];
    PrimCovMtx(2,0) += SecVrtErr[3];
    PrimCovMtx(1,2) += SecVrtErr[4];
    PrimCovMtx(2,1) += SecVrtErr[4];
    PrimCovMtx(2,2) += SecVrtErr[5];

    bool success=true;
    AmgSymMatrix(3)  WgtMtx;
    PrimCovMtx.computeInverseWithCheck(WgtMtx, success);
    if( !success || WgtMtx(0,0)<=0. || WgtMtx(1,1)<=0. || WgtMtx(2,2)<=0. ){
       ATH_MSG_DEBUG(" Cov.matrix inversion failure in vertex distance significane");
       return 1.e10;
    }

    Signif=SVPV.transpose()*WgtMtx*SVPV;

    if(Signif<=0.)return 1.e10;        //Something is wrong in distance significance. 
    Signif=std::sqrt(Signif);
    if( Signif!=Signif ) Signif = 0.;
    return SVPV.norm();
  }

  double InDetVKalVxInJetTool::vrtVrtDist2D(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                          const std::vector<double>& SecVrtErr, double& Signif)
  const
  {
    Amg::Vector2D SVPV(PrimVrt.x()- SecVrt.x(),PrimVrt.y()- SecVrt.y());

    AmgSymMatrix(3)  PrimCovMtx=PrimVrt.covariancePosition();  //Create
    AmgSymMatrix(2)  CovMtx;
    CovMtx(0,0) = PrimCovMtx(0,0) + SecVrtErr[0];
    CovMtx(0,1) = PrimCovMtx(0,1) + SecVrtErr[1];
    CovMtx(1,0) = PrimCovMtx(1,0) + SecVrtErr[1];
    CovMtx(1,1) = PrimCovMtx(1,1) + SecVrtErr[2];

    bool success=true;
    AmgSymMatrix(2)  WgtMtx;
    CovMtx.computeInverseWithCheck(WgtMtx, success);
    if( !success || WgtMtx(0,0)<=0. || WgtMtx(1,1)<=0. ){
       ATH_MSG_DEBUG(" Cov.matrix inversion failure in vertex distance significane");
       return 1.e10;
    }

    Signif=SVPV.transpose()*WgtMtx*SVPV;

    if(Signif<=0.)return 1.e10;        //Something is wrong in distance significance. 
    Signif=std::sqrt(Signif);
    if( Signif!=Signif ) Signif = 0.;
    return SVPV.norm();
  }

//--------------------------------------------------
// Significance along jet direction
//--------------------------------------------------
  double InDetVKalVxInJetTool::vrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                          const std::vector<double>& SecVrtErr, const TLorentzVector & JetDir)
  const
  {
    Amg::Vector3D jetDir(JetDir.Vect().Unit().X(), JetDir.Vect().Unit().Y(), JetDir.Vect().Unit().Z());
    double projDist=(SecVrt-PrimVrt.position()).dot(jetDir);
    Amg::Vector3D SVPV=jetDir*projDist;

    AmgSymMatrix(3)  PrimCovMtx=PrimVrt.covariancePosition();  //Create
    PrimCovMtx(0,0) += SecVrtErr[0];
    PrimCovMtx(0,1) += SecVrtErr[1];
    PrimCovMtx(1,0) += SecVrtErr[1];
    PrimCovMtx(1,1) += SecVrtErr[2];
    PrimCovMtx(0,2) += SecVrtErr[3];
    PrimCovMtx(2,0) += SecVrtErr[3];
    PrimCovMtx(1,2) += SecVrtErr[4];
    PrimCovMtx(2,1) += SecVrtErr[4];
    PrimCovMtx(2,2) += SecVrtErr[5];

    bool success=true;
    AmgSymMatrix(3)  WgtMtx;
    PrimCovMtx.computeInverseWithCheck(WgtMtx, success);
    if( !success || WgtMtx(0,0)<=0. || WgtMtx(1,1)<=0. || WgtMtx(2,2)<=0. ){
       ATH_MSG_DEBUG(" Cov.matrix inversion failure in vertex distance significane");
       return 1.e10;
    }

    double Signif=SVPV.transpose()*WgtMtx*SVPV;
    if(Signif<=0.)return 1.e10;        //Something is wrong in distance significance. 
    Signif=std::sqrt(Signif);
    if( Signif!=Signif ) Signif = 0.;
    if(projDist<0)Signif=-Signif;
    return Signif;
  }

  double InDetVKalVxInJetTool::vrtVrtDist(const Amg::Vector3D & Vrt1, const std::vector<double>  & VrtErr1,
                                          const Amg::Vector3D & Vrt2, const std::vector<double>  & VrtErr2)
  const
  {
    double Signif;
    Amg::Vector3D SVPV(Vrt1.x()- Vrt2.x(),Vrt1.y()- Vrt2.y(),Vrt1.z()- Vrt2.z());

    AmgSymMatrix(3)  PrimCovMtx;  //Create
    PrimCovMtx(0,0) =                   VrtErr1[0]+VrtErr2[0];
    PrimCovMtx(0,1) = PrimCovMtx(1,0) = VrtErr1[1]+VrtErr2[1];
    PrimCovMtx(1,1) =                   VrtErr1[2]+VrtErr2[2];
    PrimCovMtx(0,2) = PrimCovMtx(2,0) = VrtErr1[3]+VrtErr2[3];
    PrimCovMtx(1,2) = PrimCovMtx(2,1) = VrtErr1[4]+VrtErr2[4];
    PrimCovMtx(2,2) =                   VrtErr1[5]+VrtErr2[5];

    bool success=true;
    AmgSymMatrix(3)  WgtMtx;
    PrimCovMtx.computeInverseWithCheck(WgtMtx, success);
    if( !success || WgtMtx(0,0)<=0. || WgtMtx(1,1)<=0. || WgtMtx(2,2)<=0. ){
       ATH_MSG_DEBUG(" Cov.matrix inversion failure in vertex distance significane");
       return 1.e10;
    }

    Signif=SVPV.transpose()*WgtMtx*SVPV;
    if(Signif<=0.)return 1.e10;        //Something is wrong in distance significance. 
    Signif=std::sqrt(Signif);
    if(Signif != Signif)  Signif = 0.;
    return Signif;
  }
//


//----------------------------
//   Vertex error along radius
//----------------------------
  double InDetVKalVxInJetTool::vrtRadiusError(const Amg::Vector3D & SecVrt, const std::vector<double>  & VrtErr) 
  {
    double DirX=SecVrt.x(), DirY=SecVrt.y(); 
    double Covar =    DirX*VrtErr[0]*DirX
                  +2.*DirX*VrtErr[1]*DirY
                     +DirY*VrtErr[2]*DirY;
    Covar /= DirX*DirX + DirY*DirY;
    Covar=std::sqrt(Covar);
    if(Covar != Covar)  Covar = 0.;
    return Covar;
  }



  double InDetVKalVxInJetTool::coneDist(const AmgVector(5) & vectPerig, const TLorentzVector & jetDir)
  
  {
  
  	  double  etaTr = -std::log(std::tan(vectPerig[3]/2.));
	  double  etaJet = jetDir.PseudoRapidity();
	  double  adphi = std::abs(jetDir.Phi()-vectPerig[2]);
	  while(adphi> M_PI)adphi-=2.*M_PI;
 	  return  std::sqrt(adphi*adphi + (etaJet-etaTr)*(etaJet-etaTr));
  }


    /* Invariant mass calculation for V0 decays*/
    /* Gives correct mass assignment in case of nonequal masses*/


   double InDetVKalVxInJetTool::massV0(std::vector< std::vector<double> >& trkAtVrt,
                               double massP, double massPi )
   
   {
        double ap1=1./std::abs(trkAtVrt[0][2]);
	double ap2=1./std::abs(trkAtVrt[1][2]);
        CxxUtils::sincos phi1  (trkAtVrt[0][0]);
        CxxUtils::sincos theta1(trkAtVrt[0][1]);
        CxxUtils::sincos phi2  (trkAtVrt[1][0]);
        CxxUtils::sincos theta2(trkAtVrt[1][1]);
        double px = phi1.cs*theta1.sn*ap1 
                  + phi2.cs*theta2.sn*ap2;
        double py = phi1.sn*theta1.sn*ap1 
                  + phi2.sn*theta2.sn*ap2;
        double pz =         theta1.cs*ap1 
                  +         theta2.cs*ap2;
        double ee= (ap1 > ap2) ? 
            (std::sqrt(ap1*ap1+massP*massP)+std::sqrt(ap2*ap2+massPi*massPi)):
            (std::sqrt(ap2*ap2+massP*massP)+std::sqrt(ap1*ap1+massPi*massPi));
        double test=(ee-pz)*(ee+pz)-px*px-py*py;
        return test>0 ? std::sqrt(test) : 0.; 
    }


//
// Search for outliers using track Chi2 and track Ranking
   int InDetVKalVxInJetTool::findMax( std::vector<double>& chi2PerTrk, std::vector<float> & rank)
   
   { 
      double chi2Ref=0.;
      int position=-1;
      if( chi2PerTrk.empty() ) return position ;
      for (int i=0; i< (int)chi2PerTrk.size(); i++){
	if(chi2PerTrk[i]/std::max(rank[i],(float)0.1) > chi2Ref) { chi2Ref=chi2PerTrk[i]/std::max(rank[i],(float)0.1); position=i;}
      }
      return position;
   }      
  

//  Function returns a transverse momentum of track w/r some direction
//
  double InDetVKalVxInJetTool::pTvsDir(const Amg::Vector3D &dir, const std::vector< double >& inpTrk) 
  
  {
     double norm=std::hypot(dir.x(),dir.y(),dir.z());
     double sx=dir.x()/norm; double sy=dir.y()/norm; double sz=dir.z()/norm;

     double px=0.,py=0.,pz=0.; double scale;
     double api=1./std::abs(inpTrk[2]);
     CxxUtils::sincos phi  (inpTrk[0]);
     CxxUtils::sincos theta(inpTrk[1]);

     px = phi.cs * theta.sn*api;
     py = phi.sn * theta.sn*api;
     pz =          theta.cs*api;
       scale = px*sx + py*sy + pz*sz;
     px -= sx*scale;
     py -= sy*scale; 
     pz -= sz*scale;
     return std::hypot(px,py,pz);
   }

  TLorentzVector InDetVKalVxInJetTool::totalMom(const std::vector<const Trk::Perigee*>& inpTrk) 
  const
  {
     AmgVector(5) vectPerig; vectPerig.setZero();
     double px=0.,py=0.,pz=0.,ee=0.;
     for (const auto *i : inpTrk) {
       if(!i) continue;
       vectPerig = i->parameters(); 
       double api=1./std::abs(vectPerig[4]);
       CxxUtils::sincos phi  (vectPerig[2]);
       CxxUtils::sincos theta(vectPerig[3]);
       px += phi.cs * theta.sn*api;
       py += phi.sn * theta.sn*api;
       pz +=          theta.cs*api;
       ee += std::sqrt( api*api + m_massPi*m_massPi);
     }
     return {px,py,pz,ee}; 
   }

  TLorentzVector InDetVKalVxInJetTool::totalMom(const std::vector<const xAOD::TrackParticle*>& InpTrk) 
  
  {
     TLorentzVector sum(0.,0.,0.,0.); 
     for (const auto *i : InpTrk) {
       if( i == nullptr ) continue; 
       sum += i->p4();
     }
     return sum; 
   }


  TLorentzVector InDetVKalVxInJetTool::momAtVrt(const std::vector< double >& inpTrk) 
  const
  {
     double api=1./std::abs(inpTrk[2]);
     CxxUtils::sincos phi  (inpTrk[0]);
     CxxUtils::sincos theta(inpTrk[1]);
     double px = phi.cs * theta.sn*api;
     double py = phi.sn * theta.sn*api;
     double pz =          theta.cs*api;
     double ee = std::sqrt( api*api + m_massPi*m_massPi);
     return {px,py,pz,ee}; 
   }
//
//-- Perigee in xAOD::TrackParticle
//

  const Trk::Perigee* InDetVKalVxInJetTool::getPerigee( const xAOD::TrackParticle* i_ntrk) 
  {
       return &(i_ntrk->perigeeParameters());
  }



  StatusCode  InDetVKalVxInJetTool::VKalVrtFitFastBase(
    const std::vector<const xAOD::TrackParticle*>& listTrk,
    Amg::Vector3D& FitVertex,
    Trk::IVKalState& istate) const
  {
    return m_fitSvc->VKalVrtFitFast(listTrk, FitVertex, istate);
  }


  StatusCode InDetVKalVxInJetTool::VKalVrtFitBase(const std::vector<const xAOD::TrackParticle*> & listPart,
                                                  Amg::Vector3D&                   Vertex,
                                                  TLorentzVector&                  Momentum,
                                                  long int&                        Charge,
                                                  std::vector<double>&             ErrorMatrix,
                                                  std::vector<double>&             Chi2PerTrk,
                                                  std::vector< std::vector<double> >& TrkAtVrt,
                                                  double& Chi2,
                                                  Trk::IVKalState& istate,
                                                  bool ifCovV0) const
  {
     std::vector<const xAOD::NeutralParticle*> netralPartDummy(0);
     return m_fitSvc->VKalVrtFit( listPart, netralPartDummy,Vertex, Momentum, Charge,
                                  ErrorMatrix, Chi2PerTrk, TrkAtVrt, Chi2,
                                  istate, ifCovV0 );

  }

  StatusCode InDetVKalVxInJetTool::GetTrkFitWeights(std::vector<double> & wgt,
                                                    const Trk::IVKalState& istate) const
  {
    return m_fitSvc->VKalGetTrkWeights(wgt, istate);
  }
/*************************************************************************************************************/
  void   InDetVKalVxInJetTool::getPixelLayers(const xAOD::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit, int &nLays ) const
  {
    	blHit=l1Hit=l2Hit=nLays=0; 
        if(m_existIBL){              // 4-layer pixel detector
          uint8_t IBLhit,BLhit,NPlay,IBLexp,BLexp;
          if(!Part->summaryValue( IBLhit,  xAOD::numberOfInnermostPixelLayerHits) )        IBLhit = 0;
          if(!Part->summaryValue(  BLhit,  xAOD::numberOfNextToInnermostPixelLayerHits) )   BLhit = 0;
          if(!Part->summaryValue(  NPlay,  xAOD::numberOfContribPixelLayers) )              NPlay = 0;
          if(!Part->summaryValue( IBLexp,  xAOD::expectInnermostPixelLayerHit) )           IBLexp = 0;
          if(!Part->summaryValue(  BLexp,  xAOD::expectNextToInnermostPixelLayerHit) )      BLexp = 0;
          blHit=IBLhit; if( IBLexp==0 ) blHit=-1;
          l1Hit= BLhit; if(  BLexp==0 ) l1Hit=-1;
          nLays=NPlay;
          //if((IBLhit+BLhit) == 0){      //no hits in IBL and BL  VK OLD VERSION WITHOUT PATTERN AVAILABLE
	  //   if(NPlay>=1) { l2Hit=1; }  // at least one of remaining layers is fired
	  //   if(NPlay==0) { l2Hit=0; }
          //}else if( IBLhit*BLhit == 0){ // one hit in IBL and BL. Others are presumably also fired
	  //   if(NPlay>=2) { l2Hit=1; }
	  //   if(NPlay<=1) { l2Hit=0; }  // no fired layer except for IBL/BL
          //}
          uint32_t HitPattern=Part->hitPattern();
	  l2Hit=0; if( HitPattern&((1<<Trk::pixelBarrel2)) ) l2Hit=1;
	  //   bitH=HitPattern&((int)std::pow(2,Trk::pixelBarrel1));
        } else {                     // 3-layer pixel detector
          uint8_t BLhit,NPlay,NHoles,IBLhit;
          if(!Part->summaryValue( BLhit,  xAOD::numberOfBLayerHits) )          BLhit = 0;
          if(!Part->summaryValue(IBLhit,  xAOD::numberOfInnermostPixelLayerHits) )  IBLhit = 0; // Some safety
          BLhit=BLhit>IBLhit ? BLhit : IBLhit;                                                  // Some safety
          if(!Part->summaryValue( NPlay,  xAOD::numberOfContribPixelLayers) )  NPlay = 0;
          if(!Part->summaryValue(NHoles,  xAOD::numberOfPixelHoles) )         NHoles = 0;
          blHit=BLhit;  //B-layer hit is fired. Presumable all other layers are also fired.
          nLays=NPlay;
          //if (BLhit==0) {   //B-layer hit is absent. 
	  //   if(NPlay>=2) { l1Hit=l2Hit=1;}
	  //   if(NPlay==0) { l1Hit=l2Hit=0;}
	  //   if(NPlay==1) { 
	  //     if( NHoles==0) {l1Hit=0; l2Hit=1;}  
	  //     if( NHoles>=1) {l1Hit=1; l2Hit=0;}  
          //   }
          //}
          uint32_t HitPattern=Part->hitPattern();
	  l1Hit=0; if( HitPattern&((1<<Trk::pixelBarrel1)) ) l1Hit=1;
	  l2Hit=0; if( HitPattern&((1<<Trk::pixelBarrel2)) ) l2Hit=1;
        }

  }
  void InDetVKalVxInJetTool::getPixelProblems(const xAOD::TrackParticle* Part, int &splshIBL, int &splshBL ) const
  {
    	splshIBL=splshBL=0;
        if(m_existIBL){              // 4-layer pixel detector
          uint8_t share,split;
	  //if(!Part->summaryValue( IBLout,  xAOD::numberOfInnermostPixelLayerOutliers ) )        IBLout = 0;
	  if(!Part->summaryValue( share,   xAOD::numberOfInnermostPixelLayerSharedHits ) )   share = 0;
	  if(!Part->summaryValue( split,  xAOD::numberOfInnermostPixelLayerSplitHits ) )        split = 0;
          splshIBL=share+split;
	  if(!Part->summaryValue( share,   xAOD::numberOfNextToInnermostPixelLayerSharedHits ) )   share = 0;
	  if(!Part->summaryValue( split,  xAOD::numberOfNextToInnermostPixelLayerSplitHits ) )        split = 0;
          splshBL=share+split;
       }
  }
  void   InDetVKalVxInJetTool::getPixelDiscs(const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) 
  {
        uint32_t HitPattern=Part->hitPattern();
	d0Hit=0; if( HitPattern&((1<<Trk::pixelEndCap0)) ) d0Hit=1;
	d1Hit=0; if( HitPattern&((1<<Trk::pixelEndCap1)) ) d1Hit=1;
	d2Hit=0; if( HitPattern&((1<<Trk::pixelEndCap2)) ) d2Hit=1;
  }

/*************************************************************************************************************/

  Amg::MatrixX InDetVKalVxInJetTool::makeVrtCovMatrix( std::vector<double> & errorMatrix )
  
  {
      Amg::MatrixX vrtCovMtx(3,3);  
      vrtCovMtx(0,0)                  = errorMatrix[0];
      vrtCovMtx(0,1) = vrtCovMtx(1,0) = errorMatrix[1];
      vrtCovMtx(1,1)                  = errorMatrix[2];
      vrtCovMtx(0,2) = vrtCovMtx(2,0) = errorMatrix[3];
      vrtCovMtx(1,2) = vrtCovMtx(2,1) = errorMatrix[4];
      vrtCovMtx(2,2)                  = errorMatrix[5];
      return vrtCovMtx;
  }

  void InDetVKalVxInJetTool::fillVrtNTup( std::vector<Vrt2Tr>  & all2TrVrt)
  const
  {
         if (!m_h) return;
         Hists& h = getHists();
         int ipnt=0;
         Amg::Vector3D pf1,pf2; 
         for(auto & vrt : all2TrVrt) {
           if(ipnt==DevTuple::maxNTrk)break;
           h.m_curTup->VrtDist2D[ipnt]=vrt.fitVertex.perp();
           h.m_curTup->VrtSig3D[ipnt]=vrt.signif3D;
	   h.m_curTup->VrtSig2D[ipnt]=vrt.signif2D;
	   h.m_curTup->itrk[ipnt]=vrt.i;
	   h.m_curTup->jtrk[ipnt]=vrt.j;
	   h.m_curTup->mass[ipnt]=vrt.momentum.M();
	   h.m_curTup->Chi2[ipnt]=vrt.chi2;
	   h.m_curTup->badVrt[ipnt]=vrt.badVrt;
	   h.m_curTup->VrtDR[ipnt]=vrt.dRSVPV;
	   h.m_curTup->VrtErrR[ipnt]= vrtRadiusError(vrt.fitVertex, vrt.errorMatrix);
           Amg::setRThetaPhi(pf1, 1., vrt.trkAtVrt[0][1], vrt.trkAtVrt[0][0]);
           Amg::setRThetaPhi(pf2, 1., vrt.trkAtVrt[1][1], vrt.trkAtVrt[1][0]);
           h.m_curTup->VrtdRtt[ipnt]=Amg::deltaR(pf1,pf2);
           ipnt++; h.m_curTup->nVrt=ipnt;
        }
  } 

  void InDetVKalVxInJetTool::fillNVrtNTup(std::vector<WrkVrt> & VrtSet, std::vector< std::vector<float> > & trkScore,
                                          const xAOD::Vertex  & PV, const TLorentzVector & JetDir)
  const
  {
         if (!m_h) return;
         Hists& h = getHists();
         int ipnt=0;
         TLorentzVector VertexMom;
         for(auto & vrt : VrtSet) {
	   if(ipnt==DevTuple::maxNVrt)break;
	   h.m_curTup->NVrtDist2D[ipnt]=vrt.vertex.perp();
	   h.m_curTup->NVrtNT[ipnt]=vrt.selTrk.size();
           h.m_curTup->NVrtTrkI[ipnt]=vrt.selTrk[0];
	   h.m_curTup->NVrtM[ipnt]=vrt.vertexMom.M();
	   h.m_curTup->NVrtChi2[ipnt]=vrt.chi2;
           float maxW=0., sumW=0.;
           for(auto trk : vrt.selTrk){ sumW+=trkScore[trk][0]; maxW=std::max(trkScore[trk][0], maxW);}
	   h.m_curTup->NVrtMaxW[ipnt]=maxW;
	   h.m_curTup->NVrtAveW[ipnt]=sumW/vrt.selTrk.size();
           TLorentzVector SVPV(vrt.vertex.x()-PV.x(),vrt.vertex.y()-PV.y(),vrt.vertex.z()-PV.z(),1.);
           h.m_curTup->NVrtDR[ipnt]=JetDir.DeltaR(SVPV);
           VertexMom += vrt.vertexMom;
           ipnt++; h.m_curTup->nNVrt=ipnt;
        }
        h.m_curTup->TotM=VertexMom.M();
  } 


  int InDetVKalVxInJetTool::getIdHF(const xAOD::TrackParticle* TP ) {
      if( TP->isAvailable< ElementLink< xAOD::TruthParticleContainer> >( "truthParticleLink") ) {
        const ElementLink<xAOD::TruthParticleContainer>& tplink = 
                               TP->auxdata< ElementLink< xAOD::TruthParticleContainer > >("truthParticleLink");
        if( !tplink.isValid() ) return 0;
        if( TP->auxdata< float >( "truthMatchProbability" ) < 0.5 ) return 0;
        if (HepMC::is_simulation_particle(*tplink)) return 0;
        if( (*tplink)->hasProdVtx()){
          if( (*tplink)->prodVtx()->nIncomingParticles()==1){
             int PDGID1=0, PDGID2=0, PDGID3=0, PDGID4=0;
	     const xAOD::TruthParticle * parTP1=getPreviousParent(*tplink, PDGID1);
	     const xAOD::TruthParticle * parTP2=nullptr ;
	     const xAOD::TruthParticle * parTP3=nullptr ;
	     int noBC1=notFromBC(PDGID1);
             if(noBC1)  parTP2 = getPreviousParent(parTP1, PDGID2);
	     int noBC2=notFromBC(PDGID2);
             if(noBC2 && parTP2) parTP3 = getPreviousParent(parTP2, PDGID3);
	     int noBC3=notFromBC(PDGID3);
             if(noBC3 && parTP3) getPreviousParent(parTP3, PDGID4);
	     int noBC4=notFromBC(PDGID4);
             if(noBC1 && noBC2 && noBC3 && noBC4)return 0;
             return 1;  //This is a reconstructed track from B/C decays
      } } }
      return 0;
  }

  int InDetVKalVxInJetTool::notFromBC(int PDGID) {
    int noBC=0;
    if(PDGID<=0)return 1;
    if(PDGID>600 && PDGID<4000)noBC=1;
    if(PDGID<400 || PDGID>5600)noBC=1;
    if(PDGID==513  || PDGID==523  || PDGID==533  || PDGID==543)noBC=1;  //Remove tracks from B* (they are in PV)
    if(PDGID==5114 || PDGID==5214 || PDGID==5224 || PDGID==5314 || PDGID==5324)noBC=1; //Remove tracks from B_Barions* (they are in PV)
  //if(PDGID==413  || PDGID==423  || PDGID==433 )continue;  //Keep tracks from D* (they are from B vertex)
  //if(PDGID==4114 || PDGID==4214 || PDGID==4224 || PDGID==4314 || PDGID==4324)continue;
    return noBC;
  }
  const xAOD::TruthParticle * InDetVKalVxInJetTool::getPreviousParent(const xAOD::TruthParticle * child, int & ParentPDG) {
    ParentPDG=0;
    if( child->hasProdVtx() ){
       if( child->prodVtx()->nIncomingParticles()==1 ){
            ParentPDG = abs((*(child->prodVtx()->incomingParticleLinks())[0])->pdgId());
            return *(child->prodVtx()->incomingParticleLinks())[0];
       }
    }
    return nullptr;
  }


  int InDetVKalVxInJetTool::getG4Inter(const xAOD::TrackParticle* TP ) {
      if( TP->isAvailable< ElementLink< xAOD::TruthParticleContainer> >( "truthParticleLink") ) {
        const ElementLink<xAOD::TruthParticleContainer>& tplink = 
                               TP->auxdata< ElementLink< xAOD::TruthParticleContainer > >("truthParticleLink");
        if( tplink.isValid() && HepMC::is_simulation_particle(*tplink)) return 1;
      }
      return 0;
  }
  int InDetVKalVxInJetTool::getMCPileup(const xAOD::TrackParticle* TP ) {
      if( TP->isAvailable< ElementLink< xAOD::TruthParticleContainer> >( "truthParticleLink") ) {
        const ElementLink<xAOD::TruthParticleContainer>& tplink = 
                               TP->auxdata< ElementLink< xAOD::TruthParticleContainer > >("truthParticleLink");
        if( !tplink.isValid() ) return 1;
      } else { return 1; }
      return 0;
  }

}  //end namespace
