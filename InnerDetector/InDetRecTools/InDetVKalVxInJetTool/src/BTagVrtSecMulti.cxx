/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "InDetVKalVxInJetTool/InDetVKalVxInJetTool.h"
//-------------------------------------------------
// Other stuff
#include  "AnalysisUtils/AnalysisMisc.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include  "boost/graph/bron_kerbosch_all_cliques.hpp"
#include  "TMath.h"
#include  "TH1D.h"
#include  "TH2D.h"
#include  "TProfile.h"

#include  <algorithm>


///----------------------------------------------------------------------------------------
///  getVrtSecMulti resurns the vector results with the following information:
///   1) Vertex mass
///   2) Vertex/jet energy fraction
///   3) Number of initially selected 2tr vertices
///   4) Number of selected for vertexing tracks in jet 
///   5) Number of track in secondary vertex
///   6) 0. 
///   7) Jet energy used in (2) calculation 
///  
///  Author:  Vadim Kostyukhin (vadim.kostyukhin@cern.ch)
///---------------------------------------------------------------------------------------- 


namespace InDet{

const double c_vrtBCMassLimit=5500.;  // Mass limit to consider a vertex not coming from B,C-decays


std::vector<xAOD::Vertex*>
InDetVKalVxInJetTool::getVrtSecMulti(workVectorArrxAOD* xAODwrk,
                                     const xAOD::Vertex& primVrt,
                                     const TLorentzVector& jetDir,
                                     std::vector<double>& results,
                                     compatibilityGraph_t& compatibilityGraph) const
{

  const double probVrtMergeLimit = 0.01;

  int inpNPart = 0;
  if (xAODwrk) {
    inpNPart = xAODwrk->InpTrk.size();
    xAODwrk->FoundSecondTracks.clear(); // Input clearing for failure return
    results.clear();                    // Input clearing for failure return
  }
  ATH_MSG_DEBUG( "InDet getVrtSecMulti() called with NPart=" << inpNPart );

  std::vector<xAOD::Vertex*> finalVertices(0);

  if (inpNPart < 2) {
    return finalVertices;
  } // 0,1 track => nothing to do!

  float evtWgt = 1.;
  const EventContext & ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoKey,ctx};
  if (eventInfo.isValid()) {
    if(eventInfo->hasBeamSpotWeight()) evtWgt *= eventInfo->beamSpotWeight();
  } else ATH_MSG_DEBUG("No event info object found!");


  long int nTracks = 0;
  TLorentzVector momentumJet;
  int nRefPVTrk = 0;
  if (xAODwrk) {
    nRefPVTrk = selGoodTrkParticle(xAODwrk->InpTrk, primVrt, jetDir, xAODwrk->listJetTracks,evtWgt);
    while (!xAODwrk->listJetTracks.empty() &&
           xAODwrk->listJetTracks[0]->pt() / jetDir.Pt() > 1.){
      xAODwrk->listJetTracks.erase(xAODwrk->listJetTracks.begin());
    }
    nTracks = xAODwrk->listJetTracks.size();
    momentumJet = totalMom(xAODwrk->listJetTracks);
  }

  if (nTracks < 2) {
    return finalVertices;
  } // 0,1 selected track => nothing to do!

  
  ATH_MSG_DEBUG("Number of selected tracks inside jet= " << nTracks);

  if (m_fillHist) {
    Hists& h = getHists();
    h.m_hb_jmom->Fill(momentumJet.Perp(), evtWgt);
    h.m_hb_ntrkjet->Fill(static_cast<double>(nTracks), evtWgt);
  }

  //
  //  InpTrk[]           - input track list
  //  listJetTracks[]    - list of good tracks in jet for vertex search
  //------------------------------------------------------------
  //                     Initial track list ready
  //                     Find 2track vertices
  //

  std::vector<double> inpMass(nTracks, m_massPi);
  double vrt2TrackNumber = 0;

  if (xAODwrk) {
    select2TrVrt(xAODwrk->listJetTracks,
                 xAODwrk->TracksForFit,
                 primVrt,
                 jetDir,
                 inpMass,
		 nRefPVTrk,
                 xAODwrk->TrkFromV0,
                 xAODwrk->listSecondTracks,
                 compatibilityGraph);
    if (xAODwrk->TrkFromV0.size() > 1) {
      removeDoubleEntries(xAODwrk->TrkFromV0);
      AnalysisUtils::Sort::pT(&(xAODwrk->TrkFromV0));
    }
    vrt2TrackNumber = xAODwrk->listSecondTracks.size()/2.0;
    removeDoubleEntries(xAODwrk->listSecondTracks);
    AnalysisUtils::Sort::pT(&(xAODwrk->listSecondTracks));
  }


  ATH_MSG_DEBUG(" Found different tracks in pairs=" << vrt2TrackNumber);

  //
  //  listSecondTracks[]   -  list of all tracks which participate in some
  //  2-track vertex TrkFromV0[]          -  "bad" tracks from any
  //  V0/material/conversion m_Incomp[]           -  main vector of pointers for
  //  multivertex search
  //-----------------------------------------------------------------------------------------------------
  //            Secondary track list is ready
  //            Creation of initial vertex set
  //

  std::unique_ptr<std::vector<WrkVrt>> wrkVrtSet = std::make_unique<std::vector<WrkVrt>>();
  WrkVrt newvrt;
  newvrt.Good = true;
  std::unique_ptr<Trk::IVKalState> state = m_fitSvc->makeState();
  StatusCode sc;

  //================================================== Boost version (don't
  //forget to uncomment addEdge in select2TrVrt()
  std::vector<std::vector<int>> allCliques;
  bron_kerbosch_all_cliques(compatibilityGraph, clique_visitor(allCliques));

  for (const auto& clique : allCliques) {

    newvrt.selTrk.clear();
    if (xAODwrk) xAODwrk->tmpListTracks.clear();

    for (int i_trk : clique){
      newvrt.selTrk.push_back(i_trk);
      if(xAODwrk) xAODwrk->tmpListTracks.push_back(xAODwrk->listJetTracks.at(i_trk));
    }

    if (xAODwrk) sc = VKalVrtFitFastBase(xAODwrk->tmpListTracks, newvrt.vertex, *state);
    if (sc.isFailure() ||
        newvrt.vertex.perp() > m_rLayer2 * 2.) {
      // No initial estimation
      m_fitSvc->setApproximateVertex(primVrt.x(),
                                     primVrt.y(),
                                     primVrt.z(),
                                     *state);       // Use as starting point
      if (m_multiWithPrimary)
        m_fitSvc->setApproximateVertex(0., 0., 0., *state);
    } else {
      Amg::Vector3D vDist = newvrt.vertex - primVrt.position();
      double jetVrtDir = jetDir.Px() * vDist.x() + jetDir.Py() * vDist.y() +
                         jetDir.Pz() * vDist.z();
      if (m_multiWithPrimary) jetVrtDir = std::abs(jetVrtDir); // Always positive when primary vertex is seeked for
      if (jetVrtDir > 0.) {
	// Good initial estimation
        m_fitSvc->setApproximateVertex(newvrt.vertex.x(),
                                       newvrt.vertex.y(),
                                       newvrt.vertex.z(),
                                       *state);     //Use as starting point
      } else {
        m_fitSvc->setApproximateVertex(primVrt.x(), primVrt.y(), primVrt.z(), *state);
      }
    }

    sc = StatusCode::FAILURE;
    if (xAODwrk) {
      sc = VKalVrtFitBase(xAODwrk->tmpListTracks,
                          newvrt.vertex,
                          newvrt.vertexMom,
                          newvrt.vertexCharge,
                          newvrt.vertexCov,
                          newvrt.chi2PerTrk,
                          newvrt.trkAtVrt,
                          newvrt.chi2,
                          *state,
                          false);
    }
    if (sc.isFailure()) continue; // Bad fit - goto next solution

    if (clique.size() == 2 && newvrt.chi2 > 10.) continue; // Bad 2track vertex

    if (newvrt.chi2PerTrk.size() == 2){
      newvrt.chi2PerTrk[0] = newvrt.chi2PerTrk[1] = newvrt.chi2 / 2.;
    }
    newvrt.Good = true;
    newvrt.nCloseVrt = 0;
    newvrt.dCloseVrt = 1000000.;
    newvrt.projectedVrt =
    jetProjDist(newvrt.vertex, primVrt, jetDir); // 3D SV-PV distance
    wrkVrtSet->push_back(newvrt);
  }

  //
  //========= Initial cleaning of solutions
  //-Remove worst track from vertices with very bad Chi2
  bool disassembled = false;

  do {
    disassembled = false;
    int NSoluI = (*wrkVrtSet).size();
    for (int iv = 0; iv < NSoluI; iv++) {
      WrkVrt vrt = (*wrkVrtSet)[iv];
      if (!vrt.Good || vrt.selTrk.size() == 2) continue;
      if (TMath::Prob(vrt.chi2, 2 * vrt.selTrk.size() - 3) < 1.e-3) {
        if (xAODwrk) disassembleVertex(wrkVrtSet.get(), iv, xAODwrk->listJetTracks, *state);
        disassembled = true;
      }
    }
  }while(disassembled);

  //-Remove vertices fully contained in other vertices
  while( (*wrkVrtSet).size()>1 ){
    int tmpN = (*wrkVrtSet).size();

    int iv = 0;
    for(; iv<tmpN-1; iv++){
      WrkVrt vert_i = (*wrkVrtSet)[iv];
      if(!vert_i.Good ) continue;
      int ntrk_i = (*wrkVrtSet)[iv].selTrk.size();

      int jv = iv+1;
      for(; jv<tmpN; jv++){
	WrkVrt vert_j = (*wrkVrtSet)[jv];
	if(!vert_j.Good ) continue;
	int ntrk_j = (*wrkVrtSet)[jv].selTrk.size();

	int nTCom = nTrkCommon(wrkVrtSet.get(), iv, jv);
	if(nTCom==ntrk_i){
	  (*wrkVrtSet).erase((*wrkVrtSet).begin()+iv);
	  break;
	}
	else if(nTCom==ntrk_j){
	  (*wrkVrtSet).erase((*wrkVrtSet).begin()+jv);
	  break;
	}
      }
      if(jv!=tmpN) break;  // One vertex is erased. Restart check

    }
    if(iv==tmpN-1) break;  // No vertex deleted

  } // end while( (*wrkVrtSet).size()>1 )


  //
  //- Try to merge all vertices with common tracks
  std::multimap<int,std::pair<int,int>> vrtWithCommonTrk;
  std::multimap<int,std::pair<int,int>>::reverse_iterator icvrt;
  do{
    int NSoluI = (*wrkVrtSet).size();
    vrtWithCommonTrk.clear();
    for(int iv = 0; iv<NSoluI-1; iv++ ){
      for(int jv = iv+1; jv<NSoluI; jv++){
	if(!(*wrkVrtSet)[iv].Good || !(*wrkVrtSet)[jv].Good)    continue;
	int nTCom=nTrkCommon(wrkVrtSet.get(), iv, jv);
	if(!nTCom)continue;
	vrtWithCommonTrk.emplace(nTCom,std::make_pair(iv,jv));
      }
    }

    for(icvrt = vrtWithCommonTrk.rbegin(); icvrt!=vrtWithCommonTrk.rend(); ++icvrt){
      int nTCom = (*icvrt).first;
      int iv = (*icvrt).second.first;
      int jv = (*icvrt).second.second;
      int nTrkI = (*wrkVrtSet)[iv].selTrk.size();
      int nTrkJ = (*wrkVrtSet)[jv].selTrk.size();
      double probV = -1.;
      if (xAODwrk) {
	probV = mergeAndRefitVertices(wrkVrtSet.get(), iv, jv, newvrt, xAODwrk->listJetTracks, *state);
      }

      if(probV<probVrtMergeLimit){
	if(nTrkI==2 || nTrkJ==2 || nTCom<2) {
	  continue;
	}
	if(nTCom>nTrkI-nTCom || nTCom>nTrkJ-nTCom){
	  //2 and more common tracks for NTr>=3 vertices. Merge anyway.
	  if(xAODwrk) mergeAndRefitOverlapVertices( wrkVrtSet.get(), iv, jv, xAODwrk->listJetTracks, *state);
	  break; //Vertex list is changed. Restart merging from scratch.
	}
	continue;  //Continue merging loop
      }

      newvrt.Good = true;
      (*wrkVrtSet).push_back(newvrt);
      (*wrkVrtSet)[iv].Good = false;
      (*wrkVrtSet)[jv].Good = false;
      break;  //Merging successful. Break merging loop and remake list of connected vertices
    } // end for(icvrt)

  } while( icvrt != vrtWithCommonTrk.rend() );


  if(m_fillHist){
    int cvgood=0;
    for(const auto& vrt : (*wrkVrtSet)){
      if(vrt.Good) cvgood++;
    }
    Hists& h = getHists();
    h.m_hb_rawVrtN->Fill( static_cast<float>(cvgood), evtWgt);
  }

  //-Remove all bad vertices from the working set
  for(auto &tmpV : (*wrkVrtSet) ) {
    if(tmpV.vertex.perp()>m_rLayer3+10.) tmpV.Good = false; //Vertices outside Pixel detector
    TLorentzVector SVPV(tmpV.vertex.x()-primVrt.x(),
			tmpV.vertex.y()-primVrt.y(),
			tmpV.vertex.z()-primVrt.z(), 1.);
    if(jetDir.DeltaR(SVPV)>m_coneForTag) tmpV.Good = false; // SV is outside of the jet cone
  }

  unsigned int tmpV = 0;
  while( tmpV<(*wrkVrtSet).size() ){
    if( !(*wrkVrtSet)[tmpV].Good ) (*wrkVrtSet).erase((*wrkVrtSet).begin()+tmpV);
    else tmpV++;
  }

  if((*wrkVrtSet).empty()){             // No vertices at all
    return finalVertices;
  }

  std::vector< std::vector<float> > trkScore(0);
  if(xAODwrk && m_useTrackClassificator){
    for(auto &trk : xAODwrk->listJetTracks) trkScore.push_back(m_trackClassificator->trkTypeWgts(trk, primVrt, jetDir));
  }

  for(auto &tmpV : (*wrkVrtSet) ) tmpV.projectedVrt=jetProjDist(tmpV.vertex, primVrt, jetDir);  //Setup ProjectedVrt

  //----------------------------------------------------------------------------
  //             Here we have the overlapping solutions.
  //             Vertices may have only 1 common track.
  //              Now solution cleaning

  std::unique_ptr<std::vector< std::deque<long int> > > trkInVrt = std::make_unique<std::vector<std::deque<long int>>>(nTracks);
  trackClassification(wrkVrtSet.get(), trkInVrt.get());

  double foundMaxT;
  long int selectedTrack, selectedVertex;
  int foundV1, foundV2;

  state = m_fitSvc->makeState();
  while( ( foundMaxT = MaxOfShared(wrkVrtSet.get(), trkInVrt.get(), selectedTrack, selectedVertex) ) > 0) {

    double foundMinVrtDst = 1000000.;
    if(foundMaxT<m_trackDetachCut) foundMinVrtDst = minVrtVrtDist(wrkVrtSet.get(), foundV1, foundV2);

    //Choice of action
    if( foundMaxT<m_trackDetachCut && foundMinVrtDst<m_vertexMergeCut
	&& nTrkCommon(wrkVrtSet.get(), foundV1, foundV2) ){

      bool vrtMerged=false;   //to check whether something is really merged
      while(foundMinVrtDst<m_vertexMergeCut){
	if(foundV1<foundV2) std::swap(foundV1, foundV2); // Always drop vertex with smallest number

	double probV = 0.;
	if (xAODwrk) probV = mergeAndRefitVertices(wrkVrtSet.get(), foundV1, foundV2, newvrt, xAODwrk->listJetTracks, *state);

	if(probV>probVrtMergeLimit && newvrt.vertexMom.M()<c_vrtBCMassLimit){
	  //  Good merged vertex found
	  double tstDst = jetProjDist(newvrt.vertex, primVrt, jetDir);
	  if(tstDst>0.){
	    // only positive vertex directions are accepted as merging result
	    std::swap((*wrkVrtSet)[foundV1], newvrt);
	    (*wrkVrtSet)[foundV1].projectedVrt = tstDst;
	    (*wrkVrtSet)[foundV2].Good = false;         //Drop vertex
	    (*wrkVrtSet)[foundV2].selTrk.clear();     //Clean dropped vertex
	    vrtMerged=true;
	  }
	}

	(*wrkVrtSet)[foundV1].nCloseVrt = -1;
	(*wrkVrtSet)[foundV1].dCloseVrt = 1000000.; //For minVrtVrtDistNext optimisation(!)
	(*wrkVrtSet)[foundV2].nCloseVrt = -1;
	(*wrkVrtSet)[foundV2].dCloseVrt = 1000000.; //Exclude given pair
	foundMinVrtDst = minVrtVrtDistNext(wrkVrtSet.get(), foundV1, foundV2);     //Check other vertices

      } // end while(foundMinVrtDst<m_vertexMergeCut)

      if(vrtMerged){
	trkInVrt->resize(nTracks);
	trackClassification(wrkVrtSet.get(), trkInVrt.get());
	continue;  // Something was merged => goto next cycle. Otherwise break the found track-vertex link
      }

    } // end choice of action

    removeTrackFromVertex(wrkVrtSet.get(), trkInVrt.get(), selectedTrack, selectedVertex);

    sc = StatusCode::FAILURE;
    if(xAODwrk) sc = refitVertex( wrkVrtSet.get(), selectedVertex, xAODwrk->listJetTracks, *state, false);

    (*wrkVrtSet)[selectedVertex].projectedVrt = jetProjDist((*wrkVrtSet)[selectedVertex].vertex, primVrt, jetDir);

    if( sc.isFailure() ) (*wrkVrtSet)[selectedVertex].Good = false;  //bad vertex
    if( (*wrkVrtSet)[selectedVertex].projectedVrt<0.
	&& (*wrkVrtSet)[selectedVertex].selTrk.size()==2 ){
      (*wrkVrtSet)[selectedVertex].Good = false;  // 2track vertex migrates behind PV - drop it.
    }

  } // end while( (foundMaxT = MaxOfShared)>0 )


  //
  // Final check/merge for close vertices
  //
  double minDistVV = minVrtVrtDist( wrkVrtSet.get(), foundV1, foundV2); //recalculate VV distances
  while ( minDistVV < m_vertexMergeCut) {
    if(foundV1<foundV2) std::swap(foundV1, foundV2);

    double probV = 0.;
    if(xAODwrk) probV = mergeAndRefitVertices(wrkVrtSet.get(), foundV1, foundV2, newvrt, xAODwrk->listJetTracks, *state);

    if(probV>probVrtMergeLimit && newvrt.vertexMom.M()<c_vrtBCMassLimit){
      //  Good merged vertex found
      double tstDst = jetProjDist(newvrt.vertex, primVrt, jetDir);
      if(tstDst>0.){
	// only positive vertex directions are accepted as merging result
	std::swap((*wrkVrtSet)[foundV1],newvrt);
	(*wrkVrtSet)[foundV1].projectedVrt = tstDst;
	(*wrkVrtSet)[foundV2].Good = false;         //Drop vertex
	(*wrkVrtSet)[foundV2].selTrk.clear();     //Clean dropped vertex
      }
    }

    (*wrkVrtSet)[foundV1].nCloseVrt = -1;
    (*wrkVrtSet)[foundV1].dCloseVrt = 1000000.; //For minVrtVrtDistNext optimisation(!)
    (*wrkVrtSet)[foundV2].nCloseVrt = -1;
    (*wrkVrtSet)[foundV2].dCloseVrt = 1000000.; //Exclude given pair
    minDistVV = minVrtVrtDistNext(wrkVrtSet.get(), foundV1, foundV2);
  }

  //
  // Try to improve vertices with big Chi2 if something went wrong. Just precaution.
  for(unsigned int iv=0; iv<wrkVrtSet->size(); iv++) {
    WrkVrt vert_i = (*wrkVrtSet)[iv];
    if(!vert_i.Good) continue;  //don't work on vertex which is already bad
    if( vert_i.selTrk.size()<3 ) continue;

    double tmpProb = TMath::Prob( vert_i.chi2, 2*vert_i.selTrk.size()-3 ); //Chi2 of the original vertex
    if(tmpProb<0.001){
      if(xAODwrk) tmpProb = improveVertexChi2(wrkVrtSet.get(), iv, xAODwrk->listJetTracks, *state, false);
      if(tmpProb<0.001) vert_i.Good = false;
      vert_i.projectedVrt = jetProjDist(vert_i.vertex, primVrt, jetDir);
    }
  }

  // Final vertex selection/cleaning
  state = m_fitSvc->makeState();

  //--------- Start with 1-track vertices
  //=First check if the track was detached from a multitrack vertex. If so - reattach.
  for(auto &ntrVrt : (*wrkVrtSet)){
    if(!ntrVrt.Good || ntrVrt.selTrk.size()<=1) continue;

    for(auto &onetVrt : (*wrkVrtSet)){
      if(!onetVrt.Good || onetVrt.selTrk.size()!=1) continue;

      if(ntrVrt.detachedTrack==onetVrt.selTrk[0]){
	WrkVrt newV(ntrVrt);
	newV.selTrk.push_back(ntrVrt.detachedTrack);
	double vProb = 0.;
	if(xAODwrk) vProb = refitVertex(newV, xAODwrk->listJetTracks, *state, true);
	if(vProb>probVrtMergeLimit){
	  onetVrt.Good=false;
	  ntrVrt=newV;
	  ntrVrt.detachedTrack=-1;
	}
	break;
      }
    }
  }

  //=Recheck if the track was detached from a 2track vertex. If so - reattach.
  for(auto &iVrt : (*wrkVrtSet)){
    if(!iVrt.Good || iVrt.selTrk.size()!=1) continue;

    for(auto &jVrt : (*wrkVrtSet)){
      if(!jVrt.Good || jVrt.selTrk.size()!=1) continue;

      if(iVrt.detachedTrack==jVrt.selTrk[0]){
	WrkVrt newV(iVrt);
	newV.selTrk.push_back(iVrt.detachedTrack);
	double vProb = 0.;
	if(xAODwrk) vProb = refitVertex(newV, xAODwrk->listJetTracks, *state, true);
	if(vProb>probVrtMergeLimit){
	  jVrt.Good = false;
	  iVrt = newV;
	  iVrt.detachedTrack = -1;
	}
	break;
      }
    }

  }


  for(auto &ntrVrt : (*wrkVrtSet)){
    if(!ntrVrt.Good  || ntrVrt.selTrk.size()<=1) continue;

    for(auto tr : ntrVrt.selTrk){
      for(auto &onetVrt : (*wrkVrtSet)){
	if(!onetVrt.Good || onetVrt.selTrk.size()!=1) continue;

	if(onetVrt.detachedTrack==tr){
	  WrkVrt newV(ntrVrt);
	  newV.selTrk.push_back(onetVrt.selTrk[0]);
	  double vProb = 0.;
	  if(xAODwrk) vProb = refitVertex( newV, xAODwrk->listJetTracks, *state, true);
	  if(vProb>probVrtMergeLimit){
	    onetVrt.Good = false;
	    ntrVrt = newV;
	  }
	}

      }
    }

  } // end for(auto &ntrVrt : (*wrkVrtSet))


  for(auto & curVrt : (*wrkVrtSet) ) {
    if(!curVrt.Good ) continue;  //don't work on vertex which is already bad
    if( std::abs(curVrt.vertex.z())>650. ){
      curVrt.Good = false;
      continue;
    }  //vertex outside Pixel det. For ALL vertices
    if(curVrt.selTrk.size() != 1) continue;

    curVrt.Good = false;       // Make them bad by default

    if(m_multiWithOneTrkVrt){
      // 1track vertices left unassigned from good 2tr vertices
      double Signif3D = 0.;
      vrtVrtDist(primVrt,curVrt.vertex, curVrt.vertexCov, Signif3D); //VK non-projected Signif3D is worse
      double tmpProb = TMath::Prob(curVrt.chi2, 1);                 //Chi2 of the original 2tr vertex

      bool trkGood = false;
      if(xAODwrk) trkGood = check1TrVertexInPixel(xAODwrk->listJetTracks[curVrt.selTrk[0]], curVrt.vertex, curVrt.vertexCov);

      if(trkGood && tmpProb>0.01){
	// accept only good tracks coming from good 2tr vertex
	std::vector<double> Impact,ImpactError;
	double Signif3DP = 0;
	if(xAODwrk) Signif3DP = m_fitSvc->VKalGetImpact(xAODwrk->listJetTracks[curVrt.selTrk[0]], primVrt.position(), 1, Impact, ImpactError, *state);

	if(m_fillHist && curVrt.vertex.perp()>20.){
	  Hists& h = getHists();
	  h.m_hb_diffPS->Fill(Signif3DP, evtWgt);
	}

	if(Signif3DP>2.*m_trkSigCut && Signif3D>m_sel2VrtSigCut) curVrt.Good = true; // accept only tracks which are far from primary vertex
      }

    }

  } // end for(auto & curVrt : (*wrkVrtSet) )


  for(auto& curVrt : (*wrkVrtSet)){
    if(!curVrt.Good ) continue;  //don't work on vertex which is already bad
    int nth = curVrt.selTrk.size();
    if(nth==1) continue; // 1track vertices are treated already

    double Signif3D = 0.;
    vrtVrtDist(primVrt,curVrt.vertex, curVrt.vertexCov, Signif3D); //VK non-projected Signif3D is worse
    if(xAODwrk) xAODwrk->tmpListTracks.resize(nth);

    for(int i = 0; i < nth; i++) {
      if(xAODwrk) xAODwrk->tmpListTracks[i] = xAODwrk->listJetTracks[curVrt.selTrk[i]];
    }
    curVrt.Good = false;                        // Make all vertices bad for futher selection

    if(nth <= 1) continue;                      // Definitely bad vertices
    if(curVrt.projectedVrt<0.)  continue;       // Remove vertices behind primary one

    if(TMath::Prob( curVrt.chi2, 2*nth-3)<0.001) continue;  // Bad Chi2 of refitted vertex

    if(nth==2 && xAODwrk){
      // Check track pixel hit patterns vs vertex position.
      if(m_useVertexCleaningPix){
	if(!check2TrVertexInPixel(xAODwrk->tmpListTracks[0],xAODwrk->tmpListTracks[1],curVrt.vertex,curVrt.vertexCov))continue;
      }

      // Check track first measured points vs vertex position.
      if(m_useVertexCleaningFMP){
	float ihitR = xAODwrk->tmpListTracks[0]->radiusOfFirstHit();
	float jhitR = xAODwrk->tmpListTracks[1]->radiusOfFirstHit();
	float vrErr = vrtRadiusError(curVrt.vertex, curVrt.vertexCov);
	if(std::abs(ihitR-jhitR)>25.) continue;                             // Hits in different pixel layers
	if(curVrt.vertex.perp()-std::min(ihitR,jhitR) > 2.*vrErr) continue; // Vertex is behind hit in pixel
      }

    }

    //---  Check interactions on pixel layers
    if(m_fillHist && nth==2){
      Hists& h = getHists();
      h.m_hb_r2d->Fill(curVrt.vertex.perp(), evtWgt);
    }

    if(m_useITkMaterialRejection){
      double xvt = curVrt.vertex.x();
      double yvt = curVrt.vertex.y();
      double zvt = curVrt.vertex.z();
      double Rvt = std::hypot(xvt,yvt);
      int bin = m_ITkPixMaterialMap->FindBin(zvt,Rvt);
      if(m_ITkPixMaterialMap->GetBinContent(bin)>0) continue;
    }

    if(m_fillHist && nth==2){
      Hists& h = getHists();
      h.m_hb_r2d->Fill(curVrt.vertex.perp(), evtWgt);
    }

    //---  Check V0s and conversions
    if(nth==2 && curVrt.vertexCharge==0 && curVrt.detachedTrack<0){
      double mass_PiPi = curVrt.vertexMom.M();
      double mass_PPi  = massV0(curVrt.trkAtVrt,m_massP,m_massPi);
      double mass_EE   = massV0(curVrt.trkAtVrt,m_massE,m_massE);
      if(m_fillHist){
	Hists& h = getHists();
	h.m_hb_massPiPi->Fill(mass_PiPi, evtWgt);
	h.m_hb_massPPi ->Fill(mass_PPi,  evtWgt);
	if(curVrt.vertex.perp()>20.) h.m_hb_massEE->Fill(mass_EE,   evtWgt);
      }

      if(std::abs(mass_PiPi-m_massK0) < 22.) continue;
      if(std::abs(mass_PPi-m_massLam) <  8.) continue;
      if(mass_EE < 60. && curVrt.vertex.perp() > 20.) continue;
    }

    if(m_fillHist){
      Hists& h = getHists();
      h.m_hb_sig3DTot->Fill(Signif3D, evtWgt);
    }
    if(Signif3D<m_sel2VrtSigCut)continue;      //Main PV-SV distance quality cut

    curVrt.Good = true;  // Vertex is absolutely good

  } // end for(auto& curVrt : (*wrkVrtSet))

  //--Final cleaning of the 1-track vertices set. Must be behind all other cleanings.
  if(m_multiWithOneTrkVrt) clean1TrVertexSet(wrkVrtSet.get());

  //Checks
  int n2trVrt = 0;               // N of vertices with 2  tracks
  int nNtrVrt = 0;               // N vertices with 3 and more tracks
  int ngoodVertices = 0;         // Final number of good vertices
  std::vector<WrkVrt> goodVertices(0);

  for(auto & iv : (*wrkVrtSet)) {
    int nth = iv.selTrk.size();
    if(nth == 0) continue;   // Definitely bad vertices

    if(iv.Good){
      ngoodVertices++;
      goodVertices.emplace_back(iv);
      if(nth==2) n2trVrt++;
      if(nth>=3) nNtrVrt++;
    }
  }

  if(ngoodVertices == 0 || (n2trVrt+nNtrVrt)==0){  // No good vertices at all
    return finalVertices;
  }

  //sorting
  while(1){
    bool swapDone = false;  // Sort on XY distance from (0.,0.)
    for(unsigned int iv = 0; iv<goodVertices.size()-1; iv++) {
      if(goodVertices[iv].vertex.perp() > goodVertices[iv+1].vertex.perp()){
	std::swap( goodVertices[iv], goodVertices[iv+1] );
	swapDone = true;
      }
    }
    if(!swapDone) break;
  }

  while(1){
    bool swapDone = false;  // Then sort on Projected dist if R<20.
    for(unsigned int iv = 0; iv<goodVertices.size()-1; iv++) {
      if(goodVertices[iv+1].vertex.perp() > 400.) continue;
      if(goodVertices[iv].projectedVrt > goodVertices[iv+1].projectedVrt){
	std::swap( goodVertices[iv], goodVertices[iv+1] );
	swapDone = true;
      }
    }
    if(!swapDone) break;
  }

  if(ngoodVertices>1){
    if( goodVertices[1].vertexMom.M()-goodVertices[0].vertexMom.M() > 5000.){
      std::swap( goodVertices[0], goodVertices[1] );
    }
  }

  if(m_fillHist){
    Hists& h = getHists();
    h.m_hb_distVV->Fill(minVrtVrtDist(wrkVrtSet.get(), foundV1, foundV2), evtWgt);
  }


  //----------------------------------------------------------------------------------
  //  Nonused tracks for one-track-vertex search
  //
  // VK - It tries to recover tracks which were already rejected on previous stages of algorithm.

  std::vector<int> nonusedTrk(0);
  for(int trk=0; trk<nTracks; trk++){
    bool present = false;

    for(const auto& iv : (*wrkVrtSet)){
      if(iv.Good){
	for(auto t_in_V : iv.selTrk){
	  if(trk==t_in_V){
	    present = true;
	    break;
	  }
	}
      }
      if(present) break;
    }

    if(!present) nonusedTrk.push_back(trk);
  }

  struct MatchedSV{int indVrt; double Signif3D;};
  std::vector<MatchedSV> matchSV(0);

  for(const auto& i_trk : nonusedTrk){
    MatchedSV tmpV = {0, 1.e9};

    for(unsigned int iv = 0; iv<goodVertices.size(); iv++){
      //--Find vertex closest to the given track
      if(!goodVertices[iv].Good) continue;
      if(goodVertices[iv].selTrk.size()<2) continue;
      if(vrtVrtDist(primVrt, goodVertices[iv].vertex, goodVertices[iv].vertexCov, jetDir) < 10.) continue;   //--Too close to PV

      double Signif = 0.;
      std::vector<double> Impact, ImpactError;
      if(xAODwrk) Signif = m_fitSvc->VKalGetImpact(xAODwrk->listJetTracks[i_trk], goodVertices[iv].vertex, 1, Impact, ImpactError, *state);
      if(Signif<tmpV.Signif3D){
	tmpV.Signif3D = Signif;
	tmpV.indVrt=iv;
      }
    }

    matchSV.push_back(tmpV);
  }

  for(int iv = 0; iv<static_cast<int>(goodVertices.size()); iv++){
    WrkVrt newV(goodVertices[iv]);
    bool addedT = false;
    std::map<double,int> addTrk;

    for(unsigned int it = 0; it<nonusedTrk.size(); it++){
      if(matchSV[it].indVrt==iv) addTrk[matchSV[it].Signif3D]=it;
    }

    std::map<double,int>::iterator atrk=addTrk.begin();
    if(!addTrk.empty()){
      if(atrk->first<4.){
	newV.selTrk.push_back(nonusedTrk[atrk->second]);
	addedT=true;
      }
    }

    if(addTrk.size()>1){
      ++atrk;
      if(atrk->first<4.){
	newV.selTrk.push_back(nonusedTrk[atrk->second]);
      }
    }

    if(addedT){
      double vProb = 0.;
      if(xAODwrk) vProb = refitVertex(newV, xAODwrk->listJetTracks, *state, true);
      if(vProb>0.01) goodVertices[iv] = newV;
      else{
	std::vector<WrkVrt> TestVertices(1,newV);
	if(xAODwrk) vProb = improveVertexChi2(&TestVertices, 0, xAODwrk->listJetTracks, *state, true);
	if(vProb>0.01) goodVertices[iv] = TestVertices[0];
      }
    }

  } // end for(unsigned int iv)


  //-------------------------------------------
  // Saving and final cleaning
  //
  int ngoodVertices_final = 0;         // Final number of good vertices
  int n1trVrt = 0;           // Final number of good 1-track vertices
  TLorentzVector vertexMom;
  bool isPrimaryVertex = true;

  for(auto& vrt : goodVertices){
    int nth = vrt.selTrk.size();
    if(xAODwrk) xAODwrk->tmpListTracks.clear();

    for(int i = 0; i < nth; i++) {
      int j = vrt.selTrk[i]; // Track number
      if(xAODwrk) xAODwrk->tmpListTracks.push_back( xAODwrk->listJetTracks[j] );
    }

    if( m_fillHist ){
      Hists& h = getHists();
      if(nth==1) h.m_hb_r1dc->Fill(vrt.vertex.perp(), evtWgt);
      if(nth==2) h.m_hb_r2dc->Fill(vrt.vertex.perp(), evtWgt);
      if(nth==3) h.m_hb_r3dc->Fill(vrt.vertex.perp(), evtWgt);
      if(nth> 3) h.m_hb_rNdc->Fill(vrt.vertex.perp(), evtWgt);
      double Signif3D = vrtVrtDist(primVrt, vrt.vertex, vrt.vertexCov, jetDir);

      if(nth==2){
	if(vrt.vertexCharge==0) h.m_hb_totmass2T1->Fill(vrt.vertexMom.M(), evtWgt);
	else                    h.m_hb_totmass2T2->Fill(vrt.vertexMom.M(), evtWgt);
	h.m_hb_sig3D2tr->Fill(Signif3D , evtWgt);
	if(vrt.vertexCharge==0) h.m_hb_totmassEE->Fill(massV0(vrt.trkAtVrt, m_massE, m_massE), evtWgt);

      }
      else if(vrt.vertexMom.M()>6000.) h.m_hb_sig3D1tr->Fill(Signif3D, evtWgt);
      else h.m_hb_sig3DNtr->Fill(Signif3D, evtWgt);
    }

    // xAOD::Vertex creation-----------------------------
    xAOD::Vertex * tmpVertex=new (std::nothrow) xAOD::Vertex();
    if(!tmpVertex){ //Premature stop due memory allocation failure
      return finalVertices;
    }
    tmpVertex->makePrivateStore();
    tmpVertex->setPosition(vrt.vertex);

    std::vector<float> floatErrMtx;
    floatErrMtx.resize(6);
    for(int i = 0; i<6; i++) floatErrMtx[i] = vrt.vertexCov[i];
    tmpVertex->setCovariance(floatErrMtx);

    tmpVertex->setFitQuality(vrt.chi2, (float)(nth*2-3));

    std::vector<Trk::VxTrackAtVertex> & tmpVTAV = tmpVertex->vxTrackAtVertex();
    tmpVTAV.clear();

    for(int ii = 0; ii<nth; ii++) {
      AmgSymMatrix(5) CovMtxP;
      CovMtxP.setIdentity();
      Trk::Perigee * tmpMeasPer = new Trk::Perigee( 0., 0.,
						    vrt.trkAtVrt[ii][0],
						    vrt.trkAtVrt[ii][1],
						    vrt.trkAtVrt[ii][2],
						    Trk::PerigeeSurface(vrt.vertex),
						    std::move(CovMtxP) );
      tmpVTAV.emplace_back( 1., tmpMeasPer );

      if(xAODwrk){
	ElementLink<xAOD::TrackParticleContainer> TEL;
	TEL.setElement( xAODwrk->tmpListTracks[ii] );
	const xAOD::TrackParticleContainer* cont = (const xAOD::TrackParticleContainer* ) (xAODwrk->tmpListTracks[ii]->container() );
	TEL.setStorableObject(*cont);
	tmpVertex->addTrackAtVertex(TEL,1.);
      }
    }

    finalVertices.push_back(tmpVertex);
    ngoodVertices_final++;
    if(nth==1) n1trVrt++;

    if(isPrimaryVertex && m_multiWithPrimary ){ // skip primary vertex if present
      isPrimaryVertex = false; // for next vertices in the loop
      continue;
    }

    vertexMom += vrt.vertexMom;

  } // end for(const auto& vrt : goodVertices)

  if(m_fillHist){
    Hists& h = getHists();
    h.m_hb_goodvrtN->Fill(ngoodVertices_final+0.1, evtWgt);
    h.m_curTup->ewgt = evtWgt;
    if(n1trVrt) h.m_hb_goodvrtN->Fill(n1trVrt+15., evtWgt);
    fillNVrtNTup(goodVertices, trkScore, primVrt, jetDir);
  }

  if(ngoodVertices_final==0){
    return finalVertices;
  }

  //-----------------------------------------------------------------------------------
  //  Saving of results
  //
  //
  double totMass = vertexMom.M();
  results.push_back(totMass);                           //1st
  double eRatio = vertexMom.E()/momentumJet.E();
  results.push_back(eRatio<1. ? eRatio : 1.);           //2nd
  results.push_back(vrt2TrackNumber);                   //3rd
  results.push_back(nTracks);                           //4th
  if(xAODwrk) results.push_back(xAODwrk->listSecondTracks.size());   //5th
  results.push_back(0.);                                //6th  -  not clear what to use here -> return 0.
  results.push_back(momentumJet.E());                   //7th

  if(m_fillHist){
    Hists& h = getHists();
    h.m_hb_ratio->Fill( results[1], evtWgt);
    h.m_hb_totmass->Fill( results[0], evtWgt);
    h.m_hb_nvrt2->Fill( results[2], evtWgt);
    h.m_hb_mom->Fill( momentumJet.Perp(), evtWgt);
  }

  return finalVertices;

}




                      /* Service routines*/

   //-----------------------------------------------------------------------------------
   //  Detach the worst Chi2 track from the vertex and join it (if possible) with other track
   //    from the vertex into additional 2-track vertices
   //
   template <class Particle>
   void InDetVKalVxInJetTool::disassembleVertex(std::vector<WrkVrt> *wrkVrtSet, int iv, 
                                                std::vector<const Particle*>  AllTracks,
                                                Trk::IVKalState& istate)
   const
   {
      WrkVrt newvrt;
      newvrt.Good = true;
      std::vector<const Particle*> ListBaseTracks;
      int NTrk = (*wrkVrtSet)[iv].selTrk.size();
      int SelT = -1;
      if(NTrk<3) return;

      StatusCode sc;
      //=== To get robust definition of most bad outlier
      m_fitSvc->setRobustness(5, istate);
      sc = refitVertex(wrkVrtSet, iv, AllTracks, istate, false);
      if(sc.isFailure()){
	(*wrkVrtSet)[iv].Good = false;
	return;
      }

      m_fitSvc->setRobustness(0, istate);
      double Chi2Max=0.;
      for(int i = 0; i<NTrk; i++){
         if((*wrkVrtSet)[iv].chi2PerTrk[i]>Chi2Max) {
	   Chi2Max = (*wrkVrtSet)[iv].chi2PerTrk[i];
	   SelT = i;
	 }
      }

      unsigned int NVrtCur = wrkVrtSet->size();
      for(int i = 0; i<NTrk; i++){
	if(i==SelT) continue;
	ListBaseTracks.clear();
	ListBaseTracks.push_back( AllTracks[(*wrkVrtSet)[iv].selTrk[i]] );
	ListBaseTracks.push_back( AllTracks[(*wrkVrtSet)[iv].selTrk[SelT]] );
	newvrt.selTrk.resize(2);
	newvrt.selTrk[0] = (*wrkVrtSet)[iv].selTrk[i];
	newvrt.selTrk[1] = (*wrkVrtSet)[iv].selTrk[SelT];

	sc = VKalVrtFitFastBase(ListBaseTracks,newvrt.vertex,istate);
	if( sc.isFailure() ) continue;
	if( newvrt.vertex.perp() > m_rLayer2*2. ) newvrt.vertex = Amg::Vector3D(0.,0.,0.);
	m_fitSvc->setApproximateVertex(newvrt.vertex[0], newvrt.vertex[1], newvrt.vertex[2], istate);

	sc=VKalVrtFitBase(ListBaseTracks,
			  newvrt.vertex,
			  newvrt.vertexMom,
			  newvrt.vertexCharge,
			  newvrt.vertexCov,
			  newvrt.chi2PerTrk,
			  newvrt.trkAtVrt,
			  newvrt.chi2,
			  istate, false);
	if(sc.isFailure() ) continue;
	if(newvrt.chi2>10.) continue;  // Too bad 2-track vertex fit

	newvrt.chi2PerTrk[0] = newvrt.chi2PerTrk[1] = newvrt.chi2/2.;
	newvrt.nCloseVrt     = 0;
	newvrt.dCloseVrt     = 1000000.;
	newvrt.projectedVrt  = 0.9999;

	if(wrkVrtSet->size()==NVrtCur){
	  wrkVrtSet->push_back(newvrt);
	  continue;
	}  // just the first added vertex

	if( (*wrkVrtSet).at(NVrtCur).chi2<newvrt.chi2 ) continue;  // previously added 2tr vertex was better

	wrkVrtSet->pop_back();
	wrkVrtSet->push_back(newvrt);
      } // end for(int i = 0; i<NTrk; i++)

      (*wrkVrtSet)[iv].selTrk.erase( (*wrkVrtSet)[iv].selTrk.begin() + SelT ); //remove track
      sc = refitVertex(wrkVrtSet, iv, AllTracks, istate, false);
      if( sc.isFailure() ) (*wrkVrtSet)[iv].Good = false;
   }


   void InDetVKalVxInJetTool::clean1TrVertexSet(std::vector<WrkVrt> *wrkVrtSet)
   {
     std::vector<int> countVT(wrkVrtSet->size(),0);
     std::vector<int> linkedVrt(wrkVrtSet->size(),0);
     //--- Mark as bad the 1track vertex if the detached track is NOT present in any remaining good vertex (>=2tr)

     for(unsigned int i1tv = 0; i1tv<wrkVrtSet->size(); i1tv++) {
       WrkVrt vrt1 = (*wrkVrtSet)[i1tv];
       if( vrt1.selTrk.size()!=1) continue;
       if(!vrt1.Good)             continue;
       int Trk1 = vrt1.detachedTrack;

       int foundInGoodVrt = 0;
       for(unsigned int mtv=0; mtv<wrkVrtSet->size(); mtv++) { //cycle over good vertices with many tracks
         WrkVrt vrtm = (*wrkVrtSet)[mtv];
         if( vrtm.selTrk.size()<2) continue;
         if(!vrtm.Good)            continue;

         if( std::find(vrtm.selTrk.begin(), vrtm.selTrk.end(), Trk1) != vrtm.selTrk.end()){
	   foundInGoodVrt++;
	   countVT[mtv]++;
	   linkedVrt[i1tv] = mtv;  //Linked vertex found
         }
       }

       if(!foundInGoodVrt) vrt1.Good=false;             // Make the vertex bad
     }

     //---Select SINGLE 1tr-vertex from many pointing to one multi-track vertex
     for(int mtv = 0; mtv<static_cast<int>(wrkVrtSet->size()); mtv++) {
       WrkVrt vrtm = (*wrkVrtSet)[mtv];
       if(vrtm.selTrk.size()<2) continue;
       if(!vrtm.Good)           continue;
       if(countVT[mtv] < 1)     continue;

       double distM = 1.e9;
       int best1TVrt = -1;
       for(unsigned int i1tv = 0; i1tv<wrkVrtSet->size(); i1tv++) {
         WrkVrt vrt1 = (*wrkVrtSet)[i1tv];
         if(vrt1.selTrk.size()!=1) continue;
         if(!vrt1.Good)            continue;
         if(linkedVrt[i1tv]!=mtv)  continue;

         double dist = (vrtm.vertexMom+vrt1.vertexMom).M();
         if(dist < distM){
	   distM = dist;
	   best1TVrt = i1tv;
	 }
         vrt1.Good=false;
       }

       if(best1TVrt>-1 && distM<c_vrtBCMassLimit) (*wrkVrtSet)[best1TVrt].Good=true;
     }
   }
   
   
   void InDetVKalVxInJetTool::trackClassification(std::vector<WrkVrt> *wrkVrtSet, 
						  std::vector< std::deque<long int> > *TrkInVrt)
   
   { 
      int NSet = wrkVrtSet->size();
      for(int iv = 0; iv<NSet; iv++) {
	if(!(*wrkVrtSet)[iv].Good) continue;
	int NTrkAtVrt = (*wrkVrtSet)[iv].selTrk.size();
	if(NTrkAtVrt<2) continue;

	for(int jt = 0; jt<NTrkAtVrt; jt++){
	  int tracknum = (*wrkVrtSet)[iv].selTrk[jt];
	  (*TrkInVrt).at(tracknum).push_back(iv);
	}
      }
   }


   double InDetVKalVxInJetTool::MaxOfShared(std::vector<WrkVrt> *wrkVrtSet, 
					    std::vector< std::deque<long int> > *TrkInVrt,
					    long int & selectedTrack,
					    long int & selectedVertex)
   
   {
      long int NTrack = TrkInVrt->size();
      double MaxOf = -999999999, SelectedProb = -1.;
      int NShareMax = 0;
      for(const auto& trk : (*TrkInVrt)){
	 int NVrt = trk.size(); // Number of vertices for this track
	 if(NVrt > NShareMax) NShareMax=NVrt;
      }
      if(NShareMax<=1) return MaxOf; // No shared tracks

      for(int it = 0; it<NTrack; it++) {
         int NVrt = (*TrkInVrt)[it].size();   // Number of vertices for this track
         if(  NVrt <= 1 )        continue;    // Should ALWAYS be checked for safety
         if(  NVrt < NShareMax ) continue;    // Not a shared track with maximal sharing

         int N2trVrt = 0;
         for(auto &vrtn : (*TrkInVrt)[it] ){
	    if((*wrkVrtSet).at(vrtn).selTrk.size()==2) N2trVrt++;
	 } //count number of 2-track vertices

	 for(const auto& VertexNumber : (*TrkInVrt)[it]){
	    WrkVrt vrt = (*wrkVrtSet).at(VertexNumber);
	    if(!vrt.Good) continue;
	    int NTrkInVrt = vrt.selTrk.size();
	    if( NTrkInVrt <= 1) continue;                               // one track vertex - nothing to do
            if(N2trVrt>0 && N2trVrt<NShareMax && NTrkInVrt>2) continue; // Mixture of multi-track and 2-track vrts. Skip multi-track then.

	    for(int itmp = 0; itmp<NTrkInVrt; itmp++){
	        if( vrt.selTrk[itmp] == it ) {  // Track found.
	          double Chi2Red = vrt.chi2PerTrk.at(itmp);  //   Normal Chi2 seems the best
                  if(NTrkInVrt==2){
		    Chi2Red = vrt.chi2/2.;                   //VK 2track vertices with Normal Chi2Red
	            if(vrt.vertexMom.M()>c_vrtBCMassLimit) Chi2Red = 100.; //VK break immediately very heavy 2tr vertices
                  }

                  double prob_vrt = TMath::Prob(vrt.chi2, 2*vrt.selTrk.size()-3);
                  if( MaxOf < Chi2Red ){
		      if(MaxOf>0 && prob_vrt>0.01 && SelectedProb<0.01) continue; // Don't disassemble good vertices if a bad one is present
		      MaxOf = Chi2Red;
		      selectedTrack = it;
		      selectedVertex = VertexNumber;
                      SelectedProb = prob_vrt;
		  }
		}
            }

	 } // end for(const auto& VertexNumber)

      } // end for(int it = 0; it<NTrack; it++)

      //-- Additional check for a common track in 2tr-2tr/Ntr vertex topology
      if( (*TrkInVrt)[selectedTrack].size() == 2){
          int v1 = (*TrkInVrt)[selectedTrack][0];
	  int v2 = (*TrkInVrt)[selectedTrack][1];
	  WrkVrt vrt1 = (*wrkVrtSet)[v1];
	  WrkVrt vrt2 = (*wrkVrtSet)[v2];
          double prb1 = TMath::Prob(vrt1.chi2, 2*vrt1.selTrk.size()-3);
	  double prb2 = TMath::Prob(vrt2.chi2, 2*vrt2.selTrk.size()-3);
	  double dst1 = vrt1.projectedVrt;
	  double dst2 = vrt2.projectedVrt; 

          if(prb1>0.05 && prb2>0.05){
	    if(vrt1.selTrk.size()==2 && vrt2.selTrk.size()==2){
              if     (selectedVertex==v1 && dst2<dst1)  selectedVertex = v2;  // Swap to remove the closest to PV vertex
              else if(selectedVertex==v2 && dst1<dst2)  selectedVertex = v1;  // Swap to remove the closest to PV vertex

              double M1 = vrt1.vertexMom.M();
	      double M2 = vrt2.vertexMom.M();
              if( M1>c_vrtBCMassLimit && M2<c_vrtBCMassLimit ) selectedVertex = v1;
              if( M1<c_vrtBCMassLimit && M2>c_vrtBCMassLimit ) selectedVertex = v2;
            }

            if( vrt1.selTrk.size()+vrt2.selTrk.size() > 4){
	       if( vrt1.selTrk.size()==2 && dst1>dst2) selectedVertex = v2;
	       if( vrt2.selTrk.size()==2 && dst2>dst1) selectedVertex = v1;
	    }
	  }	
      }

      return MaxOf;
   }


    void InDetVKalVxInJetTool::removeTrackFromVertex(std::vector<WrkVrt> *wrkVrtSet, 
						     std::vector< std::deque<long int> > *TrkInVrt,
						     long int & selectedTrack,
						     long int & selectedVertex)
    const
    {   
        int posInVrtFit = 0;                    //Position of selectedTrack in vertex fit track list.
	std::deque<long int>::iterator it;
	WrkVrt vrt = (*wrkVrtSet)[selectedVertex];
	std::deque<long int> trk = (*TrkInVrt).at(selectedTrack);

	for(it = vrt.selTrk.begin(); it!=vrt.selTrk.end(); ++it) {
	    if( (*it) == selectedTrack ) { 
	       vrt.selTrk.erase(it); break;
	    }   
            posInVrtFit++;
	}

	for(it = trk.begin(); it!=trk.end(); ++it) {
	    if( (*it) == selectedVertex ) { 
	       trk.erase(it); break;
	    }
	}

        vrt.detachedTrack = selectedTrack;

	//Check if track is removed from 2tr vertex => then sharing of track left should also be decreased
        if( vrt.selTrk.size() == 1){
	   long int LeftTrack = vrt.selTrk[0];  // track left in 1tr vertex
	   for(it = (*TrkInVrt).at(LeftTrack).begin(); it!=(*TrkInVrt)[LeftTrack].end(); ++it) {
	      if( (*it) == selectedVertex ) {
	       (*TrkInVrt)[LeftTrack].erase(it); break;
	      }
	   }

           if( TMath::Prob(vrt.chi2, 1) < 0.05 ) vrt.Good = false; // Not really good Chi2 for one-track vertex
	   if( vrt.vertexMom.M()>c_vrtBCMassLimit) vrt.Good = false; // Vertex is too heavy

           int ipos = 0;
	   if(posInVrtFit==0) ipos = 1;  // Position of remaining track in previous 2track vertex fit
	   vrt.vertexMom = momAtVrt(vrt.trkAtVrt[ipos]); //Redefine vertexMom using remaining track
	   if(!(*TrkInVrt)[LeftTrack].empty()) vrt.Good = false;    //Vertex is declared false only if remaining track is still linked to another vertex
	   vrt.trkAtVrt.erase(vrt.trkAtVrt.begin()+posInVrtFit);  //Remove also TrkAtVrt
        }

   }

   //
   //  Number of common tracks for 2 vertices
   //
   int InDetVKalVxInJetTool::nTrkCommon( std::vector<WrkVrt> *wrkVrtSet, 
                                         int V1, int V2)
   
   {
      WrkVrt vrt1 = (*wrkVrtSet).at(V1);
      WrkVrt vrt2 = (*wrkVrtSet).at(V2);
      int NTrk_V1 = vrt1.selTrk.size(); if( NTrk_V1< 2) return 0;   // Bad vertex
      int NTrk_V2 = vrt2.selTrk.size(); if( NTrk_V2< 2) return 0;   // Bad vertex

      int nTrkCom = 0;
      if(NTrk_V1 < NTrk_V2){
	for(const auto& trk : vrt1.selTrk){
          if(std::find(vrt2.selTrk.begin(), vrt2.selTrk.end(), trk) != vrt2.selTrk.end()) nTrkCom++;
        }
      } else {
	for(const auto& trk : vrt2.selTrk){
          if( std::find(vrt1.selTrk.begin(), vrt1.selTrk.end(), trk) != vrt1.selTrk.end()) nTrkCom++;
        }
      }
      return nTrkCom;
   }

   //
   //  Minimal normalized vertex-vertex distance
   //
   double InDetVKalVxInJetTool::minVrtVrtDist(std::vector<WrkVrt> *wrkVrtSet, int & V1, int & V2)
   const
   {  
     V1 = V2 = 0;
     for(auto& vrt : (*wrkVrtSet)){
       vrt.dCloseVrt = 1000000.;
       vrt.nCloseVrt = -1;
     }
     double foundMinVrtDst = 1000000.;

     for(unsigned int iv = 0; iv<wrkVrtSet->size()-1; iv++) {
        WrkVrt vrt_i = (*wrkVrtSet)[iv];
        if( vrt_i.selTrk.size()< 2)    continue;   // Bad vertices
        if(!vrt_i.Good )               continue;   // Bad vertices

        for(unsigned int jv = iv+1; jv<wrkVrtSet->size(); jv++) {
 	   WrkVrt vrt_j = (*wrkVrtSet)[jv];
	   if( vrt_j.selTrk.size()< 2) continue;   // Bad vertices
           if(!vrt_j.Good )            continue;   // Bad vertices
           double tmp = std::abs(vrt_i.vertex.x()-vrt_j.vertex.x())
	     + std::abs(vrt_i.vertex.y()-vrt_j.vertex.y())
	     + std::abs(vrt_i.vertex.z()-vrt_j.vertex.z());
           if(tmp>20.) continue;

           double tmpDst = vrtVrtDist(vrt_i.vertex, vrt_i.vertexCov,
                                      vrt_j.vertex, vrt_j.vertexCov);
           if(tmpDst < foundMinVrtDst){
	     foundMinVrtDst = tmpDst;
	     V1 = iv;
	     V2 = jv;
	   } 
           if(tmpDst < vrt_i.dCloseVrt){
	     vrt_i.dCloseVrt = tmpDst;
	     vrt_i.nCloseVrt = jv;
	   }
           if(tmpDst < vrt_j.dCloseVrt){
	     vrt_j.dCloseVrt=tmpDst;
	     vrt_j.nCloseVrt=iv;
	   }

         }
      }

      return foundMinVrtDst;
   }

   //
   // Give minimal distance between nonmodifed yet vertices
   //
   double InDetVKalVxInJetTool::minVrtVrtDistNext(std::vector<WrkVrt> *wrkVrtSet, int & V1, int & V2)
   {  
     V1 = 0; V2 = 0;
     double foundMinVrtDst = 1000000.;

     for(unsigned int iv = 0; iv<wrkVrtSet->size()-1; iv++) {
        WrkVrt vrt_i = (*wrkVrtSet)[iv];
        if(vrt_i.selTrk.size()< 2) continue;   // Bad vertex
        if(vrt_i.nCloseVrt==-1)    continue;   // Used vertex

        if(vrt_i.dCloseVrt<foundMinVrtDst){
	   int vtmp = vrt_i.nCloseVrt;
           if((*wrkVrtSet)[vtmp].nCloseVrt==-1) continue; // Close vertex to given [iv] one is modified already
	   foundMinVrtDst = vrt_i.dCloseVrt;
	   V1 = iv;
	   V2 = vtmp;
	}
      }

      return foundMinVrtDst;
   }

   //
   //  Check two close vertices by explicit vertex fit and create combined vertex if successful
   //
   template <class Particle>
   double  InDetVKalVxInJetTool::mergeAndRefitVertices(std::vector<WrkVrt> *wrkVrtSet, int V1, int V2, WrkVrt & newvrt,
						       std::vector<const Particle*> & AllTrackList,
						       Trk::IVKalState& istate)
   const
   {
      WrkVrt vrt1 = (*wrkVrtSet)[V1];
      WrkVrt vrt2 = (*wrkVrtSet)[V2];
      if(!vrt1.Good) return -1.;         //bad vertex
      if(!vrt2.Good) return -1.;         //bad vertex

      newvrt.Good = true;
      int NTrk_V1 = vrt1.selTrk.size();
      int NTrk_V2 = vrt2.selTrk.size();
      newvrt.selTrk.resize(NTrk_V1+NTrk_V2);
      std::copy(vrt1.selTrk.begin(), vrt1.selTrk.end(), newvrt.selTrk.begin());
      std::copy(vrt2.selTrk.begin(), vrt2.selTrk.end(), newvrt.selTrk.begin()+NTrk_V1);

      std::deque<long int>::iterator TransfEnd ;
      sort( newvrt.selTrk.begin(), newvrt.selTrk.end() );
      TransfEnd = unique( newvrt.selTrk.begin(), newvrt.selTrk.end() );
      newvrt.selTrk.erase( TransfEnd, newvrt.selTrk.end());

      std::vector<const Particle*> fitTrackList(0);
      for(const auto& trk : newvrt.selTrk) fitTrackList.push_back( AllTrackList[trk] );

      m_fitSvc->setApproximateVertex( 0.5*(vrt1.vertex[0]+vrt2.vertex[0]),
                                      0.5*(vrt1.vertex[1]+vrt2.vertex[1]),
                                      0.5*(vrt1.vertex[2]+vrt2.vertex[2]),
                                      istate);
      
      StatusCode sc=VKalVrtFitBase(fitTrackList,
                                   newvrt.vertex,
                                   newvrt.vertexMom,
                                   newvrt.vertexCharge,
                                   newvrt.vertexCov,
                                   newvrt.chi2PerTrk, 
                                   newvrt.trkAtVrt,
                                   newvrt.chi2,
                                   istate,
                                   false);
      if( sc.isFailure() )             return -1.;  
      if( newvrt.chi2>500. )           return -1.;  //VK protection
      if( newvrt.chi2PerTrk.size()==2) newvrt.chi2PerTrk[0] = newvrt.chi2PerTrk[1] = newvrt.chi2/2.;
      return TMath::Prob(newvrt.chi2, 2*newvrt.selTrk.size()-3);
   }


   //================== Intelligent merge of multitrack vertices with 2 and more common tracks
   template <class Particle>
   void  InDetVKalVxInJetTool::mergeAndRefitOverlapVertices(std::vector<WrkVrt> *wrkVrtSet, int V1, int V2,
							    std::vector<const Particle*> & AllTrackList,
							    Trk::IVKalState& istate) const
   {
      WrkVrt vrt1 = (*wrkVrtSet)[V1];
      WrkVrt vrt2 = (*wrkVrtSet)[V2];
      if(!vrt1.Good)return;         //bad vertex
      if(!vrt2.Good)return;         //bad vertex

      WrkVrt newvrt;
      newvrt.Good = true;
      if( nTrkCommon( wrkVrtSet, V1, V2)<2 )return;       //No overlap
      //- V1 should become ref. vertex. Another Vrt tracks will be added to it. 
      if( vrt1.selTrk.size() <  vrt2.selTrk.size() ) std::swap(V1,V2); //Vertex with NTrk=max is chosen
      else if( vrt1.selTrk.size() == vrt2.selTrk.size() ){
	if( vrt1.chi2 > vrt2.chi2 ) std::swap(V1,V2);  // Vertex with minimal Chi2 is chosen
      }

      //- Fill base track list for new vertex
      newvrt.selTrk.resize( vrt1.selTrk.size() );
      std::copy(vrt1.selTrk.begin(), vrt1.selTrk.end(), newvrt.selTrk.begin());
      //- Identify non-common tracks in second vertex
      std::vector<int> noncommonTrk(0);
      for(auto &it : vrt2.selTrk){
        if( std::find(vrt1.selTrk.begin(), vrt1.selTrk.end(), it) == vrt1.selTrk.end() ) noncommonTrk.push_back(it);
      }

      // Try to add non-common tracks one by one
      std::vector<const Particle*>  fitTrackList(0);
      std::vector<int> detachedTrk(0);
      StatusCode sc;
      WrkVrt bestVrt;
      bool foundMerged = false;

      for(auto nct : noncommonTrk){
         fitTrackList.clear();
	 for(const auto& trk : newvrt.selTrk) fitTrackList.push_back( AllTrackList[trk] );
         fitTrackList.push_back( AllTrackList.at(nct) );

         m_fitSvc->setApproximateVertex(vrt1.vertex[0], vrt1.vertex[1], vrt1.vertex[2], istate);
         sc = VKalVrtFitBase(fitTrackList, newvrt.vertex, newvrt.vertexMom,
			     newvrt.vertexCharge, newvrt.vertexCov,
			     newvrt.chi2PerTrk, newvrt.trkAtVrt, newvrt.chi2,
			     istate, false);
         if( sc.isFailure() || TMath::Prob(newvrt.chi2, 2*newvrt.selTrk.size()+2-3)<0.001 ) {
           detachedTrk.push_back(nct);
           continue;
         }

         newvrt.selTrk.push_back(nct);   // Compatible track. Add to common vertex.
         bestVrt = newvrt;
         foundMerged = true;
      }

      if(foundMerged) vrt1=bestVrt;
      vrt2.Good=false;

      // Now detached tracks
      if(detachedTrk.size()>1){
         WrkVrt nVrt;
         fitTrackList.clear();
	 nVrt.selTrk.clear();
         
	 for(auto nct : detachedTrk){
	   fitTrackList.push_back( AllTrackList[nct] );
	   nVrt.selTrk.push_back(nct);
	 }

         m_fitSvc->setApproximateVertex(vrt2.vertex[0], vrt2.vertex[1], vrt2.vertex[2], istate);
         sc = VKalVrtFitBase(fitTrackList, nVrt.vertex, nVrt.vertexMom,
			     nVrt.vertexCharge, nVrt.vertexCov,
			     nVrt.chi2PerTrk, nVrt.trkAtVrt, nVrt.chi2,
			     istate, false);
         if(sc.isSuccess()) (*wrkVrtSet).push_back(nVrt);

      } else if( detachedTrk.size()==1 ){
         bool tFound = false;
         for( auto &vrt : (*wrkVrtSet) ){
           if( !vrt.Good || vrt.selTrk.size()<2 ) continue;
           if( std::find(vrt.selTrk.begin(), vrt.selTrk.end(), detachedTrk[0]) != vrt.selTrk.end() ) {
	     tFound=true; break;
	   }
         }

         if(!tFound) {   //Track is not present in other vertices.
           double Chi2min = 1.e9;
	   int selectedTrk = -1;
           WrkVrt saveVrt;
           fitTrackList.resize(2);
           fitTrackList[0]= AllTrackList[detachedTrk[0]];

           for(auto trk : vrt2.selTrk){
              if(trk==detachedTrk[0]) continue;
              WrkVrt nVrt;
	      nVrt.Good = true;
              fitTrackList[1] = AllTrackList[trk];
              m_fitSvc->setApproximateVertex(vrt2.vertex[0], vrt2.vertex[1], vrt2.vertex[2], istate);
              sc = VKalVrtFitBase(fitTrackList, nVrt.vertex, nVrt.vertexMom,
				  nVrt.vertexCharge, nVrt.vertexCov,
				  nVrt.chi2PerTrk, nVrt.trkAtVrt, nVrt.chi2,
				  istate, false);
              if(sc.isSuccess() && nVrt.chi2<Chi2min) {
		Chi2min = nVrt.chi2;
		saveVrt = nVrt;
		selectedTrk = trk;
	      }
           }
  
	   if(Chi2min<1.e9){
             saveVrt.selTrk.resize(1);
	     saveVrt.selTrk[0] = detachedTrk[0];
             saveVrt.detachedTrack = selectedTrk;
             saveVrt.vertexMom = momAtVrt(saveVrt.trkAtVrt[0]);  //redefine vertex momentum
             (*wrkVrtSet).push_back(saveVrt);
           }
         } // end if(!tFound)

      } // end else if( detachedTrk.size()==1 )

   }

   //
   //  Iterate track removal until vertex get good Chi2
   //
   template <class Particle>
   double  InDetVKalVxInJetTool::improveVertexChi2(std::vector<WrkVrt> *wrkVrtSet, int V, std::vector<const Particle*> & AllTrackList,
						   Trk::IVKalState& istate,
						   bool ifCovV0)
   const
   {
      WrkVrt vrt = (*wrkVrtSet)[V];
      int NTrk = vrt.selTrk.size();
      if( NTrk<2 ) return 0.;

      double Prob=TMath::Prob(vrt.chi2, 2*NTrk-3);
      //----Start track removal iterations
      while(Prob<0.01){
        NTrk = vrt.selTrk.size();
        if(NTrk==2) return Prob;

        int SelT = -1;
	double Chi2Max = 0.;
        for(int i = 0; i<NTrk; i++){
	  double Chi2 = vrt.chi2PerTrk[i];
          if(Chi2>Chi2Max){
            Chi2Max = Chi2;
            SelT = i;
          }
        }

        if (SelT<0) return 0; 
        vrt.detachedTrack=vrt.selTrk[SelT];
        vrt.selTrk.erase( vrt.selTrk.begin() + SelT ); //remove track
        StatusCode sc = refitVertex(wrkVrtSet, V, AllTrackList, istate, ifCovV0);
        if(sc.isFailure()) return 0.;

        Prob = TMath::Prob(vrt.chi2, 2*(NTrk-1)-3);
      }
      return Prob;
   }

 
   template <class Particle>
   StatusCode InDetVKalVxInJetTool::refitVertex(std::vector<WrkVrt> *wrkVrtSet,
						int selectedVertex,
						std::vector<const Particle*> & selectedTracks,
						Trk::IVKalState& istate,
						bool ifCovV0) const
   {
      WrkVrt vrt = (*wrkVrtSet)[selectedVertex];
      int nth = vrt.selTrk.size();
      if(nth<2) return StatusCode::SUCCESS;
 
      double vProb = refitVertex(vrt, selectedTracks, istate, ifCovV0);
      if(vProb<0) return StatusCode::FAILURE;
      else return StatusCode::SUCCESS;
   }


   template <class Particle>
   double InDetVKalVxInJetTool::refitVertex(WrkVrt &Vrt,
                                            std::vector<const Particle*> & selectedTracks,
                                            Trk::IVKalState& istate,
                                            bool ifCovV0) const
   {
      int nth = Vrt.selTrk.size(); 
      if(nth<2) return -1.;
 
      std::vector<const Particle*>  ListTracks(0);
      for(const auto& j : Vrt.selTrk) ListTracks.push_back( selectedTracks[j] );
      Vrt.Good = false;
      Vrt.chi2PerTrk.resize(nth);
      for(int i = 0; i < nth; i++) Vrt.chi2PerTrk[i]=100000.+i; //VK safety

      m_fitSvc->setApproximateVertex(Vrt.vertex[0], Vrt.vertex[1], Vrt.vertex[2], istate);
      StatusCode SC = VKalVrtFitBase(ListTracks, Vrt.vertex, Vrt.vertexMom,
				     Vrt.vertexCharge, Vrt.vertexCov,
				     Vrt.chi2PerTrk, Vrt.trkAtVrt, Vrt.chi2,
				     istate, ifCovV0);
      if(SC.isSuccess()) Vrt.Good = true;
      else{
	Vrt.Good = false;
	return -1.;
      }

      if(Vrt.chi2PerTrk.size()==2) Vrt.chi2PerTrk[0] = Vrt.chi2PerTrk[1] = Vrt.chi2/2.;
      return TMath::Prob(Vrt.chi2, 2*nth-1);
   }

    bool InDetVKalVxInJetTool::isPart(std::deque<long int> test, std::deque<long int> base)
   {
      std::deque<long int>::iterator trk = test.begin();
      for(trk = test.begin(); trk!=test.end(); ++trk){
         if(std::find(base.begin(), base.end(), (*trk)) == base.end()) return false;  //element not found => test is not part of base
      }
      return true;
   }

   double InDetVKalVxInJetTool::jetProjDist(Amg::Vector3D & SecVrt,const xAOD::Vertex &primVrt,const TLorentzVector &jetDir)
   {
      Amg::Vector3D vv = SecVrt-primVrt.position();
      return ( vv.x()*jetDir.X()+vv.y()*jetDir.Y()+vv.z()*jetDir.Z() )/ jetDir.P();
   }

} //end namespace


