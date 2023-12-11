/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
///
///     @author  Vadim Kostyukhin <vadim.kostyukhin@cern.ch>
///
// Header include
#include "NewVrtSecInclusiveTool/NewVrtSecInclusiveTool.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include  "MVAUtils/BDT.h" 
//-------------------------------------------------
// Other stuff
#include  "AnalysisUtils/AnalysisMisc.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include "TMath.h"
#include "TH1.h"
#include "TH2D.h"
//


namespace Rec{


//
//
//--------------------------------------------------------
//   Template routine for 2track secondary vertices selection
//

    void NewVrtSecInclusiveTool::select2TrVrt(std::vector<const xAOD::TrackParticle*>  & selectedTracks,
                                  const xAOD::Vertex                 & primVrt,
                                  std::map<long int,std::vector<double>> & goodVrt,
                                  compatibilityGraph_t& compatibilityGraph )
    const
    {
      std::vector<const xAOD::NeutralParticle*> neutralPartDummy(0);
      std::vector<const xAOD::TrackParticle*>  tracksForFit(2,nullptr);
      std::vector<double> impact,impactError;
      std::vector<double> inpMass(2,m_massPi);
      long int      Charge;
      int i,j;
      StatusCode sc;
      Vrt2Tr tmpVrt;
      std::vector<Vrt2Tr> all2TrVrt(0);
      TLorentzVector PSum2T; 
      Amg::Vector3D iniVrt(0.,0.,0.);
//
      int NTracks = (int) (selectedTracks.size());
//
//  impact parameters with sign calculations
//
      uint8_t nPixelHits;
      double signifR=0.,signifZ=0.;
      std::vector<int> nPixHits(NTracks,0);
      std::vector<double> trackSignif(NTracks),dRdZratio(NTracks);
      for (i=0; i<NTracks; i++) {
         m_fitSvc->VKalGetImpact(selectedTracks[i], primVrt.position(), 1, impact, impactError);
         signifR = impact[0]/ sqrt(impactError[0]);
         signifZ = impact[1]/ sqrt(impactError[2]);
         trackSignif[i] = sqrt( signifR*signifR + signifZ*signifZ);
         dRdZratio[i] = std::abs(signifR/signifZ);
         if( !(selectedTracks[i]->summaryValue(nPixelHits,xAOD::numberOfPixelHits)) ) nPixelHits=0;
         nPixHits[i]=nPixelHits;
         if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_impactR->Fill( signifR, m_w_1);
            h.m_hb_impactZ->Fill( signifZ, m_w_1);
            h.m_hb_impactRZ->Fill(signifR, signifZ, m_w_1); 
            h.m_hb_impact->Fill( trackSignif[i], m_w_1);
            if( i<DevTuple::maxNTrk){
               Hists& h = getHists();
               h.m_curTup->pttrk[i]=selectedTracks[i]->pt();
               h.m_curTup->d0trk[i]=selectedTracks[i]->d0();
               h.m_curTup->Sig3D[i]=trackSignif[i];
               h.m_curTup->idHF[i] =getIdHF(selectedTracks[i]);
               h.m_curTup->dRdZrat[i] =dRdZratio[i];
               uint8_t TRTHits;
               if( !(selectedTracks[i]->summaryValue(  TRTHits,xAOD::numberOfTRTHits))) TRTHits=0;
               h.m_curTup->trkTRT[i] =TRTHits;
               h.m_curTup->etatrk[i] =selectedTracks[i]->eta();
           }
         }
      }

      if( m_fillHist ){
          Hists& h = getHists();
          h.m_curTup->nTrk=NTracks < DevTuple::maxNTrk ? NTracks : DevTuple::maxNTrk ;
          h.m_curTup->n2Vrt=0;
      }
   
      std::vector<std::vector<std::tuple<int,float>>> trkCount(NTracks);
      std::unique_ptr<Trk::IVKalState> state = m_fitSvc->makeState();
      m_fitSvc->setMassInputParticles( inpMass, *state );     // Use pion masses for fit
      for (i=0; i<NTracks-1; i++) {
         if(trackSignif[i]<m_trkSigCut || dRdZratio[i]<m_dRdZRatioCut )continue;
         for (j=i+1; j<NTracks; j++) {
             if(trackSignif[j]<m_trkSigCut || dRdZratio[j]<m_dRdZRatioCut )continue;
             PSum2T=selectedTracks[i]->p4()+selectedTracks[j]->p4();
             if(PSum2T.M()>1.5*m_vrt2TrMassLimit)continue;  //Approximate mass
             if( std::abs(selectedTracks[i]->eta()-selectedTracks[j]->eta())==0 &&
                 std::abs(selectedTracks[i]->phi()-selectedTracks[j]->phi())==0 &&
                 std::abs(selectedTracks[i]->pt() -selectedTracks[j]->pt())==0 ) continue; //remove duplicated tracks
             float ihitR  = selectedTracks[i]->radiusOfFirstHit();
             float jhitR  = selectedTracks[j]->radiusOfFirstHit();
             if(std::abs(ihitR-jhitR)>50.)continue;                         //- FMPs are in very different layers
             
             tracksForFit[0]=selectedTracks[i];
             tracksForFit[1]=selectedTracks[j];
             double minDZ=0.;
             sc=m_fitSvc->VKalVrtFitFast(tracksForFit,tmpVrt.fitVertex,minDZ,*state);            /* Fast crude estimation*/
             if( sc.isFailure()  ) {   /* No initial estimation */ 
                iniVrt=primVrt.position();
             } else {
                double cosMomVrtDir = projSV_PV(tmpVrt.fitVertex,primVrt,PSum2T);
                if( cosMomVrtDir>0. ) iniVrt=tmpVrt.fitVertex;                /* Good initial estimation */ 
                else                  iniVrt=primVrt.position();
             }
             if(nPixHits[i]>0 && nPixHits[j]>0){ if(minDZ>   m_fastZSVCut) continue; } // Drop SV candidates with big Z track-track distance.
             else{                               if(minDZ>2.*m_fastZSVCut) continue; } // Drop SV candidates with big Z track-track distance.  
             m_fitSvc->setApproximateVertex(iniVrt.x(), iniVrt.y(), iniVrt.z(),*state);
             sc=m_fitSvc->VKalVrtFit(tracksForFit, neutralPartDummy, tmpVrt.fitVertex, tmpVrt.momentum, Charge,
                                  tmpVrt.errorMatrix, tmpVrt.chi2PerTrk, tmpVrt.trkAtVrt, tmpVrt.chi2, *state, false );
             if(sc.isFailure())                       continue;          /* No fit */ 
             double Prob2v=TMath::Prob(tmpVrt.chi2,1);
             if( Prob2v < m_sel2VrtProbCut )                 continue;
             if( tmpVrt.momentum.M()> m_vrt2TrMassLimit )    continue; 
             if( tmpVrt.fitVertex.perp() > m_maxSVRadiusCut) continue;                  // Too far from interaction point
             double cosSVPV=projSV_PV(tmpVrt.fitVertex, primVrt, tmpVrt.momentum);
             TLorentzVector SVPV(tmpVrt.fitVertex.x()-primVrt.x(),
                                 tmpVrt.fitVertex.y()-primVrt.y(),
                                 tmpVrt.fitVertex.z()-primVrt.z(), 10.);
             if(m_fillHist){
               Hists& h = getHists();
               if(Charge==0){h.m_hb_massPiPi->Fill(tmpVrt.momentum.M(),1.);}
               h.m_hb_cosSVMom->Fill(cosSVPV,1.);
               h.m_hb_etaSV->Fill(SVPV.Eta(),1.);
             }
             if(cosSVPV<m_cosSVPVCut)continue;
             if(tmpVrt.momentum.Pt()<1000.)continue;
             double vrtR=tmpVrt.fitVertex.perp();
             double vrtRErr=vrtRadiusError(tmpVrt.fitVertex,tmpVrt.errorMatrix );
//Check close material layer
             double dstMatSignif=1.e4;
             if(m_removeTrkMatSignif>0. && vrtR>20.){
                if(vrtR<30.){ dstMatSignif=std::abs(vrtR-m_beampipeR)/vrtRErr;}
                else        { dstMatSignif=distToMatLayerSignificance(tmpVrt);}     //Material in Pixel volume
                if(dstMatSignif<m_removeTrkMatSignif)continue;
             }
//
// Check pixel hits vs vertex positions.
             int ihitIBL  = getIBLHit(selectedTracks[i]);
             int jhitIBL  = getIBLHit(selectedTracks[j]);
             if( (ihitIBL==0&&jhitIBL>0) || (ihitIBL>0&&jhitIBL==0) ) continue;
             int ihitBL   = getBLHit (selectedTracks[i]);
             int jhitBL   = getBLHit (selectedTracks[j]);
//--Very general cleaning cuts based on ID geometry and applicable to all processes
             if(tmpVrt.fitVertex.perp()<m_firstPixelLayerR-2.*vrtRErr){
                if( ihitIBL<1 && ihitBL<1) continue;
                if( jhitIBL<1 && jhitBL<1) continue;
             }
             if( vrtR-std::min(ihitR,jhitR) > 50.) continue; //- FMP is closer to (0,0) than SV itself
             if(ihitR-vrtR > 180.+2.*vrtRErr)continue;  //- Distance FMP-vertex should be less then SCT-Pixel gap
             if(jhitR-vrtR > 180.+2.*vrtRErr)continue;  //- Distance FMP-vertex should be less then SCT-Pixel gap
//-------------------------------------------------------
              if(m_useVertexCleaning){ //More agressive cleaning 
               if(std::abs(ihitR-jhitR)>12.) continue;
               if( ihitR-vrtR > 36.) continue; // Too big dR between vertex and hit in pixel
               if( jhitR-vrtR > 36.) continue; // Should be another layer in between 
               if( ihitR-vrtR <-2.*vrtRErr) continue; // Vertex is behind hit in pixel 
               if( jhitR-vrtR <-2.*vrtRErr) continue; // Vertex is behind hit in pixel 
             }
//
// Debugging and BDT
             double minPtT = std::min(tracksForFit[0]->pt(),tracksForFit[1]->pt());
             if( m_fillHist ){
                Hists& h = getHists();
                double Sig3D=0.,Sig2D=0., Dist2D=0.; 
                int idisk1=0,idisk2=0,idisk3=0,jdisk1=0,jdisk2=0,jdisk3=0;
                int sumIBLHits =  std::max(ihitIBL,0)+std::max(jhitIBL,0);
                int sumBLHits  =  std::max(ihitBL, 0)+std::max(jhitBL, 0);
                getPixelDiscs(selectedTracks[i],idisk1,idisk2,idisk3);
                getPixelDiscs(selectedTracks[j],jdisk1,jdisk2,jdisk3);
                vrtVrtDist(primVrt, tmpVrt.fitVertex, tmpVrt.errorMatrix, Sig3D);
                Dist2D=vrtVrtDist2D(primVrt, tmpVrt.fitVertex, tmpVrt.errorMatrix, Sig2D);
                h.m_hb_signif3D->Fill(Sig3D,1.);
                h.m_curTup->VrtTrkHF [h.m_curTup->n2Vrt] = getIdHF(tracksForFit[0])+ getIdHF(tracksForFit[1]);
                h.m_curTup->VrtTrkI  [h.m_curTup->n2Vrt] = getG4Inter(tracksForFit[0])+ getG4Inter(tracksForFit[1]);
                h.m_curTup->VrtCh    [h.m_curTup->n2Vrt] = Charge;
                h.m_curTup->VrtProb  [h.m_curTup->n2Vrt] = Prob2v;
                h.m_curTup->VrtSig3D [h.m_curTup->n2Vrt] = Sig3D;
                h.m_curTup->VrtSig2D [h.m_curTup->n2Vrt] = Sig2D;
                h.m_curTup->VrtDist2D[h.m_curTup->n2Vrt] = vrtR<20. ? Dist2D : vrtR;
                h.m_curTup->VrtM     [h.m_curTup->n2Vrt] = tmpVrt.momentum.M();
                h.m_curTup->VrtZ     [h.m_curTup->n2Vrt] = tmpVrt.fitVertex.z();
                h.m_curTup->VrtPt    [h.m_curTup->n2Vrt] = tmpVrt.momentum.Pt();
                h.m_curTup->VrtEta   [h.m_curTup->n2Vrt] = SVPV.Eta();
                h.m_curTup->VrtIBL   [h.m_curTup->n2Vrt] = sumIBLHits;
                h.m_curTup->VrtBL    [h.m_curTup->n2Vrt] = sumBLHits;
                h.m_curTup->VrtCosSPM[h.m_curTup->n2Vrt] = cosSVPV;
                h.m_curTup->VMinPtT  [h.m_curTup->n2Vrt] = minPtT;
                h.m_curTup->VMinS3DT [h.m_curTup->n2Vrt] = std::min(trackSignif[i],trackSignif[j]);
                h.m_curTup->VMaxS3DT [h.m_curTup->n2Vrt] = std::max(trackSignif[i],trackSignif[j]);
                h.m_curTup->VrtBDT   [h.m_curTup->n2Vrt] = 1.1;
                h.m_curTup->VrtHR1   [h.m_curTup->n2Vrt] = ihitR;
                h.m_curTup->VrtHR2   [h.m_curTup->n2Vrt] = jhitR;
                h.m_curTup->VrtDZ    [h.m_curTup->n2Vrt] = minDZ;
                h.m_curTup->VrtDisk  [h.m_curTup->n2Vrt] = idisk1+10*idisk2+20*idisk3+30*jdisk1+40*jdisk2+50*jdisk3;
                h.m_curTup->VSigMat  [h.m_curTup->n2Vrt] = dstMatSignif;
                if(h.m_curTup->n2Vrt<DevTuple::maxNVrt-1)h.m_curTup->n2Vrt++;
             }
//-------------------BDT based rejection
             if(tmpVrt.momentum.Pt() > m_vrt2TrPtLimit) continue;
             std::vector<float> VARS(10);
             VARS[0]=Prob2v;
             VARS[1]=log(tmpVrt.momentum.Pt());
             VARS[2]=log(std::max(minPtT,m_cutPt));
             VARS[3]=log(vrtR<20. ? SVPV.Perp() : vrtR);
             VARS[4]=log(std::max(std::min(trackSignif[i],trackSignif[j]),m_trkSigCut));
             VARS[5]=log(std::max(trackSignif[i],trackSignif[j]));
             VARS[6]=tmpVrt.momentum.M();
             VARS[7]=sqrt(std::abs(1.-cosSVPV*cosSVPV));
             VARS[8]=SVPV.Eta();
             VARS[9]=std::max(ihitR,jhitR);
             float wgtSelect=m_SV2T_BDT->GetGradBoostMVA(VARS);
             if( m_fillHist ) {
               Hists& h = getHists();
               h.m_curTup->VrtBDT[h.m_curTup->n2Vrt-1] = wgtSelect;
             }
             if(wgtSelect<m_v2tIniBDTCut) continue;
//
//---  Save good candidate for multi-vertex fit
//
             add_edge(i,j,compatibilityGraph);
             goodVrt[NTracks*i+j]=std::vector<double>{tmpVrt.fitVertex.x(),tmpVrt.fitVertex.y(),tmpVrt.fitVertex.z()};
             trkCount[i].emplace_back(j,wgtSelect); trkCount[j].emplace_back(i,wgtSelect);            
         }
      }
      //=== Resolve -!----!- case to speed up cluster finding
      for(int t=0; t<NTracks; t++){
         if(trkCount[t].size()==2){
            i=std::get<0>(trkCount[t][0]);
            j=std::get<0>(trkCount[t][1]);
            if(trkCount[i].size()==1 && trkCount[j].size()==1 ){
               if( std::get<1>(trkCount[t][0]) < std::get<1>(trkCount[t][1]) ) {
                  remove_edge(t,i,compatibilityGraph);
                  if(t<i)goodVrt.erase(NTracks*t+i); else goodVrt.erase(NTracks*i+t);
                  trkCount[i].clear();
                  trkCount[t].erase(trkCount[t].begin()+0);
               } else {
                  remove_edge(t,j,compatibilityGraph);
                  if(t<j)goodVrt.erase(NTracks*t+j); else goodVrt.erase(NTracks*j+t);
                  trkCount[j].clear();
                  trkCount[t].erase(trkCount[t].begin()+1);
               }
            }      
         }
      }
      //=== Remove isolated 2track vertices
      for(int t=0; t<NTracks; t++){
         if(trkCount[t].size()==1){
            i=std::get<0>(trkCount[t][0]);
            if(trkCount[i].size()==1){
               if( std::get<1>(trkCount[t][0]) < m_v2tFinBDTCut ) {
                  remove_edge(t,i,compatibilityGraph);
                  if(t<i)goodVrt.erase(NTracks*t+i); else goodVrt.erase(NTracks*i+t);
                  trkCount[t].clear();
                  trkCount[i].clear();
               }
            }      
         }
      }

        }


}  //end of namespace
