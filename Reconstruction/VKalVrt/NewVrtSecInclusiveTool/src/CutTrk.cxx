/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
///
///   @author   V.Kostykhin <Vadim.Kostyukhin@cern.ch>
/// 

// Header include
#include "NewVrtSecInclusiveTool/NewVrtSecInclusiveTool.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include "TH1.h"

//-------------------------------------------------
namespace Rec{


//==============================================================================================================
//          xAOD based stuff
//
   void NewVrtSecInclusiveTool::selGoodTrkParticle( workVectorArrxAOD * xAODwrk,
                                                    const xAOD::Vertex & primVrt)
   const
   {    
    std::vector<const xAOD::TrackParticle*>& inpTrk          = xAODwrk->inpTrk;
    std::vector<const xAOD::TrackParticle*>& selectedTracks  = xAODwrk->listSelTracks;
    std::vector<const xAOD::TrackParticle*>::const_iterator   i_ntrk;
    std::vector<double> impact,impactError;
    std::multimap<double,const xAOD::TrackParticle*> orderedTrk;
    if(m_fillHist){
      Hists& h = getHists();
      h.m_hb_ntrkInput->Fill( inpTrk.size()+0.1, m_w_1);
    }
    for (i_ntrk = inpTrk.begin(); i_ntrk < inpTrk.end(); ++i_ntrk) {
//
//-- Perigee in TrackParticle
//
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 0., m_w_1);
          }
          if((*i_ntrk)->numberDoF() == 0) continue; //Protection
          double trkChi2 = (*i_ntrk)->chiSquared() / (*i_ntrk)->numberDoF();
          if(trkChi2           > m_cutChi2)          continue;
          if( (*i_ntrk)->pt()  < m_cutPt)            continue;
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 1., m_w_1);
          }

          const Trk::Perigee mPer=(*i_ntrk)->perigeeParameters() ;
          const AmgSymMatrix(5) * locCov = mPer.covariance();
          const double CovTrkMtx00 = (*locCov)(0,0);

          uint8_t PixelHits,SctHits,BLayHits,TRTHits;
          if( !((*i_ntrk)->summaryValue(PixelHits,xAOD::numberOfPixelHits)) )   continue; // Track is
          if( !((*i_ntrk)->summaryValue(  SctHits,xAOD::numberOfSCTHits))   )   continue; // definitely
          if( SctHits<3 )                                                       continue; // bad
          if( !((*i_ntrk)->summaryValue(  TRTHits,xAOD::numberOfTRTHits))   )   continue;
          if( !((*i_ntrk)->summaryValue(BLayHits,xAOD::numberOfInnermostPixelLayerHits)))  BLayHits=0;
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 2., m_w_1);
          }

          Amg::Vector3D perigeePos=mPer.position();
          double impactD0=sqrt( (perigeePos.x()-primVrt.x())*(perigeePos.x()-primVrt.x())
                               +(perigeePos.y()-primVrt.y())*(perigeePos.y()-primVrt.y()) );
          double impactZ=perigeePos.z()-primVrt.z();
          if(m_fillHist){  
            Hists& h = getHists();
            h.m_hb_trkD0->Fill( impactD0, m_w_1);
            h.m_hb_trkZ ->Fill( impactZ, m_w_1);
          }
          if(std::abs(impactZ)*std::sin((*i_ntrk)->theta())>m_cutZVrt) continue;
          if(impactD0>m_cutD0Max)        continue;
          if(impactD0<m_cutD0Min)        continue;
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 3., m_w_1);
          }

          double bX=xAODwrk->beamX + (perigeePos.z()-xAODwrk->beamZ)*xAODwrk->tanBeamTiltX;
          double bY=xAODwrk->beamY + (perigeePos.z()-xAODwrk->beamZ)*xAODwrk->tanBeamTiltY;
          double impactBeam=sqrt( (perigeePos.x()-bX)*(perigeePos.x()-bX) + (perigeePos.y()-bY)*(perigeePos.y()-bY));
//----Anti-pileup
          double signifBeam = impactBeam    / sqrt(CovTrkMtx00);
          if(signifBeam < m_antiPileupSigRCut) continue;   // cut against tracks from pileup vertices
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 4., m_w_1);
          }
//----
          if(PixelHits	    < m_cutPixelHits)       continue;
          if(SctHits	    < m_cutSctHits)         continue;
          if((PixelHits+SctHits) < m_cutSiHits)     continue;
          if(BLayHits	    < m_cutBLayHits)        continue;
          if(std::abs((*i_ntrk)->eta())<1.9 && TRTHits < m_cutTRTHits) continue; //TRT hits must be present inside TRT
          if(m_fillHist){
            Hists& h = getHists();
            h.m_hb_trkSelect->Fill( 5., m_w_1);
          }
//----
          orderedTrk.emplace(signifBeam,*i_ntrk);
          selectedTracks.push_back(*i_ntrk);
      }
//---- Order tracks according to ranks
      std::map<double,const xAOD::TrackParticle*>::reverse_iterator rt=orderedTrk.rbegin();
      selectedTracks.resize(orderedTrk.size());
      for ( int cntt=0; rt!=orderedTrk.rend(); ++rt,++cntt) {selectedTracks[cntt]=(*rt).second;}
        }

}//end namespace
