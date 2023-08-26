/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// Class to monitoring TRT HL threshold monitoring by looking at electrons
// Created by Benjamin Kaplan (kaplan) 2009-10-14
// Major Update by Benjamin Kaplan (kaplan) 2010-06-02
// xAOD migration: Steffen Henkelmann
// **********************************************************************

#include "InDetPerformanceMonitoring/TRT_Electron_Monitoring_Tool.h"
#include "LWHists/TH1F_LW.h"
#include "LWHists/TH2F_LW.h"
#include "LWHists/TProfile_LW.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "CxxUtils/checker_macros.h"

namespace{
constexpr float electron_mass = 0.511 * Gaudi::Units::MeV;
constexpr float muon_mass = 105.66 * Gaudi::Units::MeV;
constexpr float pion_mass = 139.6 * Gaudi::Units::MeV;

constexpr double profile_x_label_size = 0.03;
constexpr double profile_y_label_size = 0.03;
constexpr double profile_msize = 0.5;
constexpr int profile_mcolor = 2;
constexpr int profile_mstyle = 2;
}

// *********************************************************************
// Public Methods
// *********************************************************************

TRT_Electron_Monitoring_Tool::
TRT_Electron_Monitoring_Tool( const std::string & type,
			const std::string & name,
                        const IInterface* parent )
  : ManagedMonitorToolBase( type, name, parent ),
  m_pTRTHelper			(nullptr),
  m_mgr				(nullptr)
{
  declareProperty( "trackName", m_tracksName="InDetTrackParticles" );
  declareProperty( "electronName", m_electronsName="Electrons");
  declareProperty( "conversionName", m_conversionName="ConversionCandidate");
  declareProperty( "muonName", m_muonsName="Muons");
  declareProperty( "isOnline",m_isOnline = true);
  declareProperty( "doExpert",m_doExpert = true);
  declareProperty( "doShift" ,m_doShift  = true);
  declareProperty( "doTracksMon", m_doTracksMon = true);
  declareProperty( "doMuonMon", m_doMuonMon = true);
  declareProperty( "doElectronMon", m_doElectronMon = true);
  declareProperty( "doRecElectrons", m_doRecElectrons = true);
  declareProperty( "doConversions", m_doConversions = true);
  declareProperty( "isEMFlag", m_isEMFlag = "Tight");
  declareProperty( "NEtaBins",m_nEtaBins = 100);
  declareProperty( "NPhiBins",m_nPhiBins = 100);
  declareProperty( "NZRBins" ,m_nZRBins = 100);
  declareProperty( "ZRMax"   ,m_ZRMax = 1.0);
  declareProperty( "useTRTOnly", m_useTRTOnly = false);
  declareProperty( "NMinTRTHits", m_NMinTRTHits = 0);
  declareProperty( "pionTRTHitCut",m_PionTRTHitCut = 19);
}

TRT_Electron_Monitoring_Tool::~TRT_Electron_Monitoring_Tool()
{
}

StatusCode
TRT_Electron_Monitoring_Tool::initialize()
{
  if(ManagedMonitorToolBase::initialize().isFailure()) return StatusCode::FAILURE;
  //    Get ID helper for TRT to access various detector components
  // 	like... straw, straw_layer, layer_or_wheel, phi_module etc...
  ATH_CHECK(detStore()->retrieve(m_pTRTHelper,"TRT_ID"));
  // Retrieve detector manager
  ATH_CHECK (detStore()->retrieve(m_mgr, "TRT"));
  if(m_isEMFlag=="Loose") m_isEMType = ISEMLOOSE;
  else if(m_isEMFlag=="Medium") m_isEMType = ISEMMEDIUM;
  else if(m_isEMFlag=="Tight") m_isEMType = ISEMTIGHT;
  else {
    ATH_MSG_WARNING( "Unrecognized isEM jobOption " << m_isEMFlag );
    ATH_MSG_WARNING( "Choose from Loose,Medium or Tight" );
    ATH_MSG_WARNING( "Skipping TRT Electron HT Monitoring! " );
    m_doElectronMon = false;
  }
  return StatusCode::SUCCESS;
}

StatusCode
TRT_Electron_Monitoring_Tool::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
TRT_Electron_Monitoring_Tool::bookHistograms()
{
  ATH_MSG_DEBUG( "Booking TRT Electron Histograms" );
  //Check for keys in Store Gate
  if(!evtStore()->contains<xAOD::TrackParticleContainer>(m_tracksName))
  {
    ATH_MSG_WARNING("No TrackCollection by the name of "<<m_tracksName<<" in storegate");
    ATH_MSG_WARNING(" Skipping TRT Track HT Monitoring! ");
    m_doTracksMon = false;
  }
  if(!evtStore()->contains<xAOD::ElectronContainer>(m_electronsName))
  {
    ATH_MSG_WARNING(  "No ElectronCollection by the name of "<<m_electronsName<<" in storegate");
    ATH_MSG_WARNING(" Skipping TRT Electron HT Monitoring! ");
    m_doRecElectrons = false;
  }
  if(!evtStore()->contains<xAOD::MuonContainer>(m_muonsName))
  {
    ATH_MSG_WARNING(  "No MuonCollection by the name of "<<m_muonsName<<" in storegate");
    ATH_MSG_WARNING(" Skipping TRT Muon HT Monitoring! ");
    m_doMuonMon = false;
  }
  if(!evtStore()->contains<xAOD::VertexContainer>(m_conversionName))
  {
    ATH_MSG_WARNING(  "No VxContainer by the name of "<<m_conversionName<<" in storegate");
    ATH_MSG_WARNING(" Skipping TRT Conversion HT Monitoring! ");
    m_doConversions = false;
  }
  m_doElectronMon = m_doElectronMon && (m_doRecElectrons || m_doConversions); //Electrons need to be turned on and one of the containers (rec or conv) needs to be available


  try
  {
    MonGroup hl_monGroup_pcand	( this, "TRT//HTMonitoring/ParticleCandidates",  run);
    if(m_doTracksMon){
      bookGeoHistograms		(m_tBarrelA	,"BarrelA"    );
      bookGeoHistograms		(m_tBarrelC	,"BarrelC"    );
      bookGeoHistograms		(m_tEndCapA	,"EndCapA"    );
      bookGeoHistograms		(m_tEndCapC	,"EndCapC"    );
      bookPCandHistograms	(hl_monGroup_pcand,m_tPi		,"Pion");
    }
    if(m_doMuonMon)	bookPCandHistograms(hl_monGroup_pcand,m_tMu,"Muon");
    if(m_doElectronMon)	bookPCandHistograms(hl_monGroup_pcand,m_tEl,"Electron");
  }
  catch(const char* hName)
  {
    ATH_MSG_FATAL("Could not register histogram " << hName << "!" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

void
TRT_Electron_Monitoring_Tool::
bookGeoHistograms( lw_geo_hists_t &hists, const std::string& name)
{
  std::string ZRn = "ZorR";
  std::string ZRt = "ZorR";
  float ZRMin = 0;
  float ZRMax = 1;
  if(name=="BarrelA"||name=="BarrelC"){
    ZRn = "Z";
    ZRt = "|Z|";
    ZRMin = 0;
    ZRMax = 800;
  }
  if(name=="EndCapA"||name=="EndCapC"){
    ZRn = "R";
    ZRt = "R";
    ZRMin = 640;
    ZRMax = 1020;
  }
  MonGroup hl_monGroup_shift	( this, "TRT//HTMonitoring/"+name,  run,ATTRIB_MANAGED,"", "");
  if( newRunFlag() )
  {
    hists.hNTracks		= trtBookHistoLW(hl_monGroup_shift, "hNTracks"		,"Number of Tracks per Event in "+name	, 150 , 0, 150 , profile_x_label_size, profile_y_label_size, "Number of Tracks", "Frequency");
    hists.hHTFracTrack		= trtBookHistoLW(hl_monGroup_shift, "hHTFracTrack"	,"HT Fraction per Track in "+name	, 50 , 0, 1 , profile_x_label_size, profile_y_label_size, "HT Fraction", "Frequency");
    hists.pHTFracTrackPhi	= trtBookHistoLW(hl_monGroup_shift, "pHTFracTrackPhi"	,"HT Fraction per Track vs. Phi in "+name			, m_nPhiBins, 0, 360     , 0, 1 , profile_x_label_size, profile_y_label_size, "PHI, degrees", "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    hists.pHTFracStrawZR	= trtBookHistoLW(hl_monGroup_shift, "pHTFracStraw"+ZRn	,"HT Fraction per Track vs. "+ZRt+" along the straw in "+name	, m_nZRBins, ZRMin, ZRMax , 0, 1 , profile_x_label_size, profile_y_label_size, ZRt+", mm"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    if(name=="BarrelA"||name=="BarrelC"){
      hists.pHTFracStrawZAI	= trtBookHistoLW(hl_monGroup_shift, "pHTFracStraw"+ZRn+"GinA"	,"HT Fraction per Track vs. "+ZRt+" along the straw in "+name+" (Gas in Side A)"	, m_nZRBins, ZRMin, ZRMax , 0, 1 , profile_x_label_size, profile_y_label_size, ZRt+", mm"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
      hists.pHTFracStrawZCO	= trtBookHistoLW(hl_monGroup_shift, "pHTFracStraw"+ZRn+"GinC"	,"HT Fraction per Track vs. "+ZRt+" along the straw in "+name+" (Gas in Side C)"	, m_nZRBins, ZRMin, ZRMax , 0, 1 , profile_x_label_size, profile_y_label_size, ZRt+", mm"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
    else{
      hists.pHTFracStrawZAI	= trtBookHistoLW(hl_monGroup_shift, "pHTFracStraw"+ZRn+"GinInner","HT Fraction per Track vs. "+ZRt+" along the straw in "+name+" (Gas in Inner End)"	, m_nZRBins, ZRMin, ZRMax , 0, 1 , profile_x_label_size, profile_y_label_size, ZRt+", mm"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
      hists.pHTFracStrawZCO	= trtBookHistoLW(hl_monGroup_shift, "pHTFracStraw"+ZRn+"GinOuter","HT Fraction per Track vs. "+ZRt+" along the straw in "+name+" (Gas in Outer End)"	, m_nZRBins, ZRMin, ZRMax , 0, 1 , profile_x_label_size, profile_y_label_size, ZRt+", mm"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
    if(m_doElectronMon){
      hists.pHTFracGammaEl	= trtBookHistoLW(hl_monGroup_shift, "pHTFracGammaEl"	,"HT Fraction per e Candidate vs. Lorentz Gamma Factor in "+name	, NGAMMABINS, 0, 5 , 0, 1, profile_x_label_size, profile_y_label_size, "Log10(Lorentz Gamma)"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
    if(m_doMuonMon){
      hists.pHTFracGammaMu	= trtBookHistoLW(hl_monGroup_shift, "pHTFracGammaMu"	,"HT Fraction per #mu Candidate vs. Lorentz Gamma Factor in "+name	, NGAMMABINS, 0, 5 , 0, 1, profile_x_label_size, profile_y_label_size, "Log10(Lorentz Gamma)"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
    if(m_doTracksMon){
      hists.pHTFracGammaPi	= trtBookHistoLW(hl_monGroup_shift, "pHTFracGammaPi"	,"HT Fraction per #pi Candidate vs. Lorentz Gamma Factor in "+name	, NGAMMABINS, 0, 5 , 0, 1, profile_x_label_size, profile_y_label_size, "Log10(Lorentz Gamma)"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
    if(m_doElectronMon||m_doMuonMon||m_doTracksMon){
      hists.pHTFracGammaAll	= trtBookHistoLW(hl_monGroup_shift, "pHTFracGammaAll"	,"HT Fraction per Candidate vs. Lorentz Gamma Factor in "+name	, NGAMMABINS, 0, 5 , 0, 1, profile_x_label_size, profile_y_label_size, "Log10(Lorentz Gamma)"     , "HT Fraction", profile_msize,profile_mcolor,profile_mstyle);
    }
  }//if( isNewRun )
  return;
}//bookGeoHistograms

void
TRT_Electron_Monitoring_Tool::
bookPCandHistograms( MonGroup& monGroup, lw_partcand_hists_t &hists, const std::string& name )
{

  if( newRunFlag() )
  {
    hists.hNCand		= trtBookHistoLW(monGroup, "hNCand"+name	,"Number of "+name+" Candidates per Event"		, 150 , 0, 150 , profile_x_label_size, profile_y_label_size, "Number of "+name+" Candidates", "Frequency");
    hists.hPIDProb		= trtBookHistoLW(monGroup, "hPIDProb"+name	,"PID Likelihood per "+name+" Candidate"		, 50 , 0, 1 , profile_x_label_size, profile_y_label_size, "PID Likelihood", "Frequency");
    hists.pPIDProbEta		= trtBookHistoLW(monGroup, "pPIDProbEta"+name	,"PID Likelihood per "+name+" Candidate vs. Eta"	, m_nEtaBins, -2, 2 , 0, 1 , profile_x_label_size, profile_y_label_size, "Eta", "PID Likelihood", profile_msize,profile_mcolor,profile_mstyle);
    hists.hHTFrac		= trtBookHistoLW(monGroup, "hHTFrac"+name	,"HT Fraction per "+name+" Candidate"			, 50 , 0, 1 , profile_x_label_size, profile_y_label_size, "HT Fraction", "Frequency");
    hists.pHTFracEta		= trtBookHistoLW(monGroup, "pHTFracEta"+name	,"HT Fraction per "+name+" Candidate vs. Eta"		, m_nEtaBins, -2, 2 , 0, 1 , profile_x_label_size, profile_y_label_size, "Eta", "HTFraction", profile_msize,profile_mcolor,profile_mstyle);
  }//if( isNewRun )
  return;
}//bookPCandHistograms

StatusCode
TRT_Electron_Monitoring_Tool::fillHistograms()
{
   ATH_MSG_DEBUG("Filling TRT Electron Monitor Histograms");
  m_tBarrelA.N = 0;
  m_tBarrelC.N = 0;
  m_tEndCapA.N = 0;
  m_tEndCapC.N = 0;
  m_tEl.N = 0;
  m_tMu.N = 0;
  m_tPi.N = 0;

  std::vector<const Trk::Track*> v_usedTrks;
  loopOverConversions(v_usedTrks);
  loopOverRecElectrons(v_usedTrks);
  loopOverMuons(v_usedTrks);
  loopOverTracks(v_usedTrks);

  if(m_doElectronMon)	m_tEl.hNCand->Fill(m_tEl.N);
  if(m_doMuonMon)	m_tMu.hNCand->Fill(m_tMu.N);
  if(m_doTracksMon)	m_tPi.hNCand->Fill(m_tPi.N);
  m_tBarrelA.hNTracks->Fill(m_tBarrelA.N);
  m_tBarrelC.hNTracks->Fill(m_tBarrelC.N);
  m_tEndCapA.hNTracks->Fill(m_tBarrelA.N);
  m_tEndCapC.hNTracks->Fill(m_tBarrelC.N);

  return StatusCode::SUCCESS;
}//fillHistograms

void
TRT_Electron_Monitoring_Tool::loopOverConversions(std::vector<const Trk::Track*> &v_usedTrks)
{
   ATH_MSG_DEBUG( "Entering loopOverConversions." );

  if(!m_doConversions) return;

  if(!evtStore()->retrieve(m_conversionContainer, m_conversionName).isFailure()){
    ATH_MSG_DEBUG( "Conversion Container has " << int(m_conversionContainer->size()) << " entries." );

    for(const auto* conv_elem : *m_conversionContainer)
      {
	const xAOD::Vertex* vertex = conv_elem;
	double chi2 = vertex->numberDoF()>0 ? vertex->chiSquared()/vertex->numberDoF() : 0;
	double r    = std::sqrt(std::pow(vertex->position().x(),2)+std::pow(vertex->position().y(),2));

	const std::vector< ElementLink< xAOD::TrackParticleContainer > > tpLinks =  conv_elem->trackParticleLinks();
	int i(-1);
	for(const auto& tp_elem : tpLinks ){
	  const xAOD::TrackParticle* trkTag = *tp_elem;
	  if(!trkTag) continue;
	  i++;
	  v_usedTrks.push_back( trkTag->track() );

	  if(!m_doElectronMon) continue;
	  //Vertex cuts
	  //  the following statement (checking number of TrackParticles in the *primary* vertex of this track)
	  //  appears to be a logic error; instead check the nnumber TrackParticles in the conversion vertex
	  //if(trkTag->vertex()->nTrackParticles() != 2) continue;
	  if (vertex->nTrackParticles() != 2) continue;
	  if( chi2 >= 5 ) continue;
	  if( r <= 40 ) continue;
	  const xAOD::TrackParticle* trkProbe = *tpLinks.at((i+1) % 2);
	  if(!trkProbe) continue;

	  if(!conversionQualityCuts(trkTag,trkProbe)) continue;
	  m_tEl.N++;
	  if(!fillAllHistograms( trkProbe, electron_mass, PCAND_EL ))
	    {
	      ATH_MSG_DEBUG( "fillStructHistograms failed!" );
	    }
	  else  ATH_MSG_DEBUG( "Conv: fillStructHistograms succeeded!" );
	}//loop over trackparticles
      }//loop over vertices
  }
  else ATH_MSG_WARNING( "Could not retrieve vertex container this event" );
  ATH_MSG_DEBUG( "Leaving loopOverConversions." );
  return;
}//loopOverConversions

void
TRT_Electron_Monitoring_Tool::
loopOverRecElectrons (std::vector<const Trk::Track*> &v_usedTrks)
{
   ATH_MSG_DEBUG( "Entering loopOverRecElectrons." );
  if(!m_doRecElectrons) return;
  if(!evtStore()->retrieve(m_electronContainer, m_electronsName).isFailure()){
    for(const auto p_Electron:*m_electronContainer)
    {
      if(not p_Electron) continue;
      if(!electronQualityCuts(p_Electron)) continue;
      const xAOD::TrackParticle* trkP = p_Electron->trackParticle();
      if(!trkP) continue;
      const Trk::Track *trk = trkP->track();
      const bool matched = (std::find(v_usedTrks.begin(), v_usedTrks.end(), trk) != v_usedTrks.end());
      if(matched) continue;
      v_usedTrks.push_back( trk );
      m_tEl.N++;
      if(!fillAllHistograms( trkP, electron_mass, PCAND_EL )){
	      ATH_MSG_DEBUG( "fillStructHistograms failed!" );
      } else  {
        ATH_MSG_DEBUG( "fillStructHistograms succeeded!" );
      }
    }//for(; p_Electron!=m_ElectronCollection->end(); ++p_Electron)
  }
  else ATH_MSG_WARNING( "Could not retrieve electron container this event" );
   ATH_MSG_DEBUG( "Leaving loopOverRecElectrons." );
  return;
}//loopOverRecElectrons

void
TRT_Electron_Monitoring_Tool::loopOverMuons (std::vector<const Trk::Track*> &v_usedTrks)
{
   ATH_MSG_DEBUG( "Entering loopOverMuons." );
  if(!m_doMuonMon) return;
  if(!evtStore()->retrieve(m_muonContainer, m_muonsName).isFailure())
  {
    for(const auto p_Muon:*m_muonContainer)
    {
      if(!(p_Muon)) continue;
      if(!muonQualityCuts(p_Muon)) continue;
      const xAOD::TrackParticle* trkP = p_Muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      if(!trkP) continue;
      const Trk::Track *trk = trkP->track();
      v_usedTrks.push_back( trk );
      m_tMu.N++;
      if(!fillAllHistograms( trkP, muon_mass, PCAND_MU )){
	      ATH_MSG_DEBUG( "fillStructHistograms failed!" );
      }
      else  ATH_MSG_DEBUG( "fillStructHistograms succeeded!" );

    }//for(; p_Muon!=m_MuonCollection->end(); ++p_Muon)
  }
  else ATH_MSG_WARNING( "Could not retrieve muon container this event" );
   ATH_MSG_DEBUG( "Leaving loopOverMuons." );
  return;
}//loopOverMuons

void
TRT_Electron_Monitoring_Tool::loopOverTracks (std::vector<const Trk::Track*> &v_usedTrks)
{
   ATH_MSG_DEBUG( "Entering loopOverTracks." );
  if(!m_doTracksMon) return;
  if(!evtStore()->retrieve(m_trkpCollection, m_tracksName).isFailure())
  {
   for(const auto* tp : *m_trkpCollection)
    {
      fillAllHistograms(tp);
      if(!pionQualityCuts(tp,v_usedTrks)) continue;
      m_tPi.N++;
      if(!fillAllHistograms(tp, pion_mass, PCAND_PI ))
      {
	     ATH_MSG_DEBUG( "fillStructHistograms failed!" );
      }
      else  ATH_MSG_DEBUG( "fillStructHistograms succeeded!" );
    }//for(; p_trk!=m_trkCollection->end(); ++p_trk)
  }
  else ATH_MSG_WARNING( "Could not retrieve track container this event" );
   ATH_MSG_DEBUG( "Leaving loopOverTTracks." );
}//loopOverTracks


bool
TRT_Electron_Monitoring_Tool::
conversionQualityCuts(const xAOD::TrackParticle* trkTag, const xAOD::TrackParticle* trkProbe)
{
  const Trk::Perigee* perigeeTag = trkTag->track()->perigeeParameters();
  int chargeTag(0);
  if (perigeeTag) chargeTag = int(perigeeTag->charge());
  else
  {
    ATH_MSG_WARNING( "Tag Track has no perigee!" );
    return false;
  }
  const Trk::Perigee* perigeeProbe = trkProbe->track()->perigeeParameters();
  int chargeProbe(0);
  if (perigeeProbe) chargeProbe = int(perigeeProbe->charge());
  else
  {
    ATH_MSG_WARNING( "Probe Track has no perigee!" );
    return false;
  }
  if(chargeTag!=-1*chargeProbe) return false;
  int nPix, nSCT, nTRT, nHT, nTRTOut, nHTOut;
  double HTRatioOut;
  uint8_t dummy(255);
  //Check that tag passes cuts
  nPix = trkTag->summaryValue(dummy,xAOD::numberOfPixelHits)?dummy:-1;
  nSCT = trkTag->summaryValue(dummy,xAOD::numberOfSCTHits)?dummy:-1;
  nTRT = trkTag->summaryValue(dummy,xAOD::numberOfTRTHits)?dummy:-1;
  nHT = trkTag->summaryValue(dummy,xAOD::numberOfTRTHighThresholdHits)?dummy:-1;
  nTRTOut = trkTag->summaryValue(dummy,xAOD::numberOfTRTOutliers)?dummy:-1;
  nHTOut = trkTag->summaryValue(dummy,xAOD::numberOfTRTHighThresholdOutliers)?dummy:-1;
  HTRatioOut = nTRT+nTRTOut>0 ? double(nHT+nHTOut)/double(nTRT+nTRTOut) : 0;
  if(!m_useTRTOnly && nPix+nSCT <= 3 ) return false;
  if(nTRT+nTRTOut <= 19) return false;
  if(HTRatioOut <= 0.12) return false;
  //Check that probe passes cuts
  nPix = trkProbe->summaryValue(dummy,xAOD::numberOfPixelHits)?dummy:-1;
  nSCT = trkProbe->summaryValue(dummy,xAOD::numberOfSCTHits)?dummy:-1;
  nTRT = trkProbe->summaryValue(dummy,xAOD::numberOfTRTHits)?dummy:-1;
  nHT = trkProbe->summaryValue(dummy,xAOD::numberOfTRTHighThresholdHits)?dummy:-1;
  nTRTOut = trkProbe->summaryValue(dummy,xAOD::numberOfTRTOutliers)?dummy:-1;
  nHTOut = trkProbe->summaryValue(dummy,xAOD::numberOfTRTHighThresholdOutliers)?dummy:-1;
  //
  if(!m_useTRTOnly && nPix+nSCT <= 3 ) return false;
  if(nTRT+nTRTOut <= 19) return false;
  return true;
}//conversionQualityCuts

bool
TRT_Electron_Monitoring_Tool::electronQualityCuts(const xAOD::Electron *electron)
{
   bool val_loose=0;
  bool val_medium=0;
  bool val_tight=0;
  if(m_isEMType==ISEMLOOSE && !(electron->passSelection(val_loose, "Loose")==0))  return false;
  if(m_isEMType==ISEMMEDIUM && !(electron->passSelection(val_medium, "Medium")==0))  return false;
  if(m_isEMType==ISEMTIGHT && !(electron->passSelection(val_tight, "Tight")==0))  return false;
  return true;
}//electronQualityCuts

bool
TRT_Electron_Monitoring_Tool::
muonQualityCuts(const xAOD::Muon *muon)
{
  if(!muon) return false;
  const xAOD::TrackParticle* trkM = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
  if(!trkM) return false;
  uint8_t dummy(255);
  int nTRT = trkM->summaryValue(  dummy , xAOD::numberOfTRTHits  )? dummy :-1;
  int nTRTOut = trkM->summaryValue(  dummy , xAOD::numberOfTRTOutliers  )? dummy :-1;
  if(nTRT+nTRTOut <= 0) return false;
  return true;
}//muonQualityCuts

bool
TRT_Electron_Monitoring_Tool::pionQualityCuts(const xAOD::TrackParticle *trkP,std::vector<const Trk::Track*> &v_usedTrks)
{
  if (std::find(v_usedTrks.begin(), v_usedTrks.end(), trkP->track()) != v_usedTrks.end()) return false;
  uint8_t dummy(255);
  int nPix = trkP->summaryValue(dummy,xAOD::numberOfPixelHits)?dummy:-1;
  int nSCT = trkP->summaryValue(dummy,xAOD::numberOfSCTHits)?dummy:-1;
  int nBLa = trkP->summaryValue(dummy,xAOD::numberOfInnermostPixelLayerHits)?dummy:-1;
  int nTRT = trkP->summaryValue(dummy,xAOD::numberOfTRTHits)?dummy:-1;
  int nTRTOut = trkP->summaryValue(dummy,xAOD::numberOfTRTOutliers)?dummy:-1;
  float dEdX = trkP->summaryValue(dummy,xAOD::pixeldEdx)?dummy:-1;

  if(!m_useTRTOnly && nPix+nSCT <= 3 ) return false;
  if(!m_useTRTOnly && nBLa <= 0 ) return false;
  if(nTRT+nTRTOut <= m_PionTRTHitCut) return false;
  if(dEdX >= 1.6 ) return false;

  return true;
}//pionQualityCuts

bool
TRT_Electron_Monitoring_Tool::
fillAllHistograms (const xAOD::TrackParticle *trkP, float mass, int PCand){
  uint8_t dummy(255);
  int nPix = trkP->summaryValue(dummy,xAOD::numberOfPixelHits)?dummy:-1;
  int nSCT = trkP->summaryValue(dummy,xAOD::numberOfSCTHits)?dummy:-1;
  if(!m_useTRTOnly && nPix+nSCT <= 3 ) return false;
  double phi(-999),eta(-999);
  const Trk::Perigee* perigee = trkP->track()->perigeeParameters();
  if (perigee)
  {
    phi = perigee->momentum().phi()*180/M_PI;
    if(phi<0) phi +=360;
    eta = perigee->momentum().eta();
  }
  else
  {
    ATH_MSG_WARNING( "Track has no perigee!" );
    return false;
  }
  float gamma = 0;
  if(mass>0){
    CLHEP::HepLorentzVector hlv;
    hlv.setVectM(  CLHEP::Hep3Vector(perigee->momentum().x(),perigee->momentum().y(),perigee->momentum().z() ),mass);
    if(hlv.isSpacelike())
    {
      ATH_MSG_DEBUG( "Track has Spacelike Lorentz Vector!!  Skipping..." );
      return false;
    }
    // Explicitly specify tolerance to avoid use of static global.
    if(hlv.isLightlike(1e-12))
    {
      ATH_MSG_DEBUG( "Track has Lightlike Lorentz Vector!!  Skipping..." );
      return false;
    }
    gamma = std::log10(hlv.gamma());
  }

  int nLLHits[4]={0,0,0,0};
  int nHLHits[4]={0,0,0,0};
  int nTRTHTHits = 0;
  int nTRTHits   = 0;
  int barrel_ec(0);
  const Trk::TrackStates* trackStates = trkP->track()->trackStateOnSurfaces();

  Trk::TrackStates::const_iterator TSOSItBegin = trackStates->begin();
  Trk::TrackStates::const_iterator TSOSItEnd   = trackStates->end();

  for(;TSOSItBegin!=TSOSItEnd; ++TSOSItBegin)
  {
    if((*TSOSItBegin)==nullptr) continue;
    else if ((*TSOSItBegin)->type(Trk::TrackStateOnSurface::Measurement))
    {
      const InDet::TRT_DriftCircleOnTrack *trtCircle = dynamic_cast<const InDet::TRT_DriftCircleOnTrack*>((*TSOSItBegin)->measurementOnTrack());
      const Trk::TrackParameters        *aTrackParam = dynamic_cast<const Trk::TrackParameters*>((*TSOSItBegin)->trackParameters());
      if(!trtCircle || !aTrackParam) continue;
      else
      {
	Identifier DCoTId = trtCircle->identify();
	barrel_ec = myBarrelEC(m_pTRTHelper->barrel_ec(DCoTId));
	if(barrel_ec<0) continue;

	const InDet::TRT_DriftCircle *RawDriftCircle = dynamic_cast<const InDet::TRT_DriftCircle*>(trtCircle->prepRawData());
	const Trk::MeasurementBase* mesb=(*TSOSItBegin)->measurementOnTrack();
	bool isTubeHit = ( mesb->localCovariance()(Trk::locX,Trk::locX) > 1.0) ? 1 : 0;
	float TRThitX = trtCircle->globalPosition().x();
	float TRThitY = trtCircle->globalPosition().y();
	float TRThitZ = std::abs(trtCircle->globalPosition().z());
	float zr = barrel_ec<2 ? TRThitZ : sqrt(pow(TRThitX,2)+pow(TRThitY,2));
	int phi_or_plane   =  barrel_ec<2 ? m_pTRTHelper->phi_module(DCoTId) :m_pTRTHelper->straw_layer(DCoTId);
	int layer_or_wheel = m_pTRTHelper->layer_or_wheel(DCoTId);
	bool inCorOuter = isGasInCorOuter(barrel_ec,phi_or_plane,layer_or_wheel);
	if(RawDriftCircle && !isTubeHit)
	{
	  bool highL = RawDriftCircle->highLevel();
	  TProfile_LW	*pHTFracTrackPhi{};
	  TProfile_LW	*pHTFracStrawZR{};
	  TProfile_LW	*pHTFracStrawZAI{};
	  TProfile_LW	*pHTFracStrawZCO{};
    TProfile_LW	*pHTFracGamma{};
    lw_geo_hists_t * myGeoHists{};
	  if(mass==0){ switch (barrel_ec){
	    case DET_BARRELA:
	      pHTFracTrackPhi = m_tBarrelA.pHTFracTrackPhi;
	      pHTFracStrawZR  = m_tBarrelA.pHTFracStrawZR;
	      pHTFracStrawZAI  = m_tBarrelA.pHTFracStrawZAI;
	      pHTFracStrawZCO  = m_tBarrelA.pHTFracStrawZCO;
	      break;
	    case DET_BARRELC:
	      pHTFracTrackPhi = m_tBarrelC.pHTFracTrackPhi;
	      pHTFracStrawZR  = m_tBarrelC.pHTFracStrawZR;
	      pHTFracStrawZAI = m_tBarrelC.pHTFracStrawZAI;
	      pHTFracStrawZCO = m_tBarrelC.pHTFracStrawZCO;
	      break;
	    case DET_ENDCAPA:
	      pHTFracTrackPhi = m_tEndCapA.pHTFracTrackPhi;
	      pHTFracStrawZR  = m_tEndCapA.pHTFracStrawZR;
	      pHTFracStrawZAI = m_tEndCapA.pHTFracStrawZAI;
	      pHTFracStrawZCO = m_tEndCapA.pHTFracStrawZCO;
	      break;
	    case DET_ENDCAPC:
	      pHTFracTrackPhi = m_tEndCapC.pHTFracTrackPhi;
	      pHTFracStrawZR  = m_tEndCapC.pHTFracStrawZR;
	      pHTFracStrawZAI = m_tEndCapC.pHTFracStrawZAI;
	      pHTFracStrawZCO = m_tEndCapC.pHTFracStrawZCO;
	      break;
	    default : continue;
	  }
	  }
	  else{
	    switch (barrel_ec){
              case DET_BARRELA:
                myGeoHists = &m_tBarrelA;
	        break;
              case DET_BARRELC:
                myGeoHists = &m_tBarrelC;
            	  break;
              case DET_ENDCAPA:
                myGeoHists = &m_tEndCapA;
	        break;
              case DET_ENDCAPC:
                myGeoHists = &m_tEndCapC;
	        break;
              default : break;
            }
            if(!myGeoHists) continue;
            switch (PCand){
              case PCAND_EL:
        	pHTFracGamma	= myGeoHists->pHTFracGammaEl;
	      break;
              case PCAND_MU:
	        pHTFracGamma	= myGeoHists->pHTFracGammaMu;
	        break;
              case PCAND_PI:
	        pHTFracGamma	= myGeoHists->pHTFracGammaPi;
	        break;
              default : break;
            }
          }
	  if(highL)
	  {
	    //--------------------------------//
	    //This means this was a HL TRT Hit//
	    //--------------------------------//
	    if(mass==0){
	      if(pHTFracTrackPhi) pHTFracTrackPhi->Fill(phi,1);
	      if(pHTFracStrawZR)  pHTFracStrawZR ->Fill(zr ,1);
	      if(pHTFracStrawZAI && !inCorOuter)  pHTFracStrawZAI ->Fill(zr ,1);
	      if(pHTFracStrawZCO &&  inCorOuter)  pHTFracStrawZCO ->Fill(zr ,1);
	    }
	    else{
	      if(pHTFracGamma) pHTFracGamma->Fill(gamma,1);
	      myGeoHists->pHTFracGammaAll->Fill(gamma,1);
            }
	    nHLHits[barrel_ec]++;
	    nTRTHTHits++;

	  }//if(RawDriftCircle->highLevel())
	  if(mass==0){
	    if(pHTFracTrackPhi) pHTFracTrackPhi->Fill(phi,0);
	    if(pHTFracStrawZR)  pHTFracStrawZR ->Fill(zr ,0);
	    if(pHTFracStrawZAI && !inCorOuter)  pHTFracStrawZAI ->Fill(zr ,0);
	    if(pHTFracStrawZCO &&  inCorOuter)  pHTFracStrawZCO ->Fill(zr ,0);
	  }
	  else{
	    if(pHTFracGamma) pHTFracGamma->Fill(gamma,0);
	    myGeoHists->pHTFracGammaAll->Fill(gamma,0);
	  }
	  nLLHits[barrel_ec]++;
	  nTRTHits++;
	}//if(RawDriftCircle && !isTubeHit)
      }//else  ---- came from if(!(trtCircle) || !(aTrackParam))
    }//else if ((*TSOSItBegin)->type(Trk::TrackStateOnSurface::Measurement))
  }//for(;TSOSItBegin!=TSOSItEnd; ++TSOSItBegin)

  if(mass==0) for(int i=0;i<4;i++){
    lw_geo_hists_t * myGeoHists{};
    switch (i){
      case DET_BARRELA:
        myGeoHists = &m_tBarrelA;
        break;
      case DET_BARRELC:
        myGeoHists = &m_tBarrelC;
	      break;
      case DET_ENDCAPA:
        myGeoHists = &m_tEndCapA;
	      break;
      case DET_ENDCAPC:
        myGeoHists = &m_tEndCapC;
	      break;
    }
    if(!myGeoHists) continue;
    if(nLLHits[i]>0){
      myGeoHists->N++;
      myGeoHists->hHTFracTrack->Fill(float(nHLHits[i])/float(nLLHits[i]));
    }
  }
  else{
    uint8_t dummy(255);
    float HTProb  = trkP->summaryValue(dummy,xAOD::eProbabilityHT)?dummy:-1;
    TH1F_LW		*hPIDProb{};
    TH1F_LW		*hHTFrac{};
    TProfile_LW		*pPIDProbEta{};
    TProfile_LW		*pHTFracEta{};
    switch (PCand){
      case PCAND_EL:
        hPIDProb	= m_tEl.hPIDProb;
        hHTFrac		= m_tEl.hHTFrac;
        pPIDProbEta	= m_tEl.pPIDProbEta;
        pHTFracEta	= m_tEl.pHTFracEta;
        break;
      case PCAND_MU:
        hPIDProb	= m_tMu.hPIDProb;
        hHTFrac		= m_tMu.hHTFrac;
        pPIDProbEta	= m_tMu.pPIDProbEta;
        pHTFracEta	= m_tMu.pHTFracEta;
        break;
      case PCAND_PI:
        hPIDProb	= m_tPi.hPIDProb;
        hHTFrac		= m_tPi.hHTFrac;
        pPIDProbEta	= m_tPi.pPIDProbEta;
        pHTFracEta	= m_tPi.pHTFracEta;
        break;
      default : break;
    }
    if(hPIDProb)    hPIDProb	->Fill(HTProb);
    if(pPIDProbEta) pPIDProbEta	->Fill(eta,HTProb);
    if(hHTFrac    && nTRTHits>0) hHTFrac	->Fill(float(nTRTHTHits)/float(nTRTHits));
    if(pHTFracEta && nTRTHits>0) pHTFracEta	->Fill(eta,float(nTRTHTHits)/float(nTRTHits));
  }
  return true;
}//getHits

bool
TRT_Electron_Monitoring_Tool::
isGasInCorOuter(int det, int phi_plane, int module_wheel)
{
  //EndCap
  if(det>1)  return (phi_plane%8)<4; //first four straw layers (planes) have gas fed from outsid

  //Barrel
  //Modules (1-3) have values (0-2)
  if(module_wheel==2) return phi_plane%2==0;  //event stacks in module 3 get gas on the A side

  return ((phi_plane - module_wheel*2)%8)<4; //Gas side alternates every four stacks.  Module 1 is shifted by 2.
}

StatusCode
TRT_Electron_Monitoring_Tool::
procHistograms()
{
  return StatusCode::SUCCESS;
}//procHistograms

//
//  Function that does the actually booking of histograms
//
//-------------------------------------------------------------------------------------//
TH1F_LW* TRT_Electron_Monitoring_Tool::trtBookHistoLW(MonGroup &mongroup,
						    const std::string& hName,
						    const std::string& hTitle,
						    int nbins,
						    double firstbin,
						    double lastbin,
						    double x_labelSize,
						    double y_labelSize,
						    const std::string& xTitle,
						    const std::string& yTitle)
{
  TH1F_LW* hist = TH1F_LW::create(hName.c_str(),hTitle.c_str(),nbins,firstbin,lastbin);
  trtRegHist(hist, mongroup, hName.c_str());
  hist->GetXaxis()->SetLabelSize(x_labelSize);
  hist->GetYaxis()->SetLabelSize(y_labelSize);
  hist->GetXaxis()->SetTitle(xTitle.c_str());
  hist->GetYaxis()->SetTitle(yTitle.c_str());
  return hist;
}

void TRT_Electron_Monitoring_Tool::trtRegHist(LWHist* hist, MonGroup &mongrp, const char* hName){
  if (mongrp.regHist(hist).isFailure()){
    ATH_MSG_WARNING( "Failed to register histogram "<< hName );
  }
    return;
}

TProfile_LW* TRT_Electron_Monitoring_Tool::trtBookHistoLW(MonGroup &mongroup,
						   const std::string& hName,
						   const std::string& hTitle,
						   int nbins,
						   double firstbin,
						   double lastbin,
						   double ymin,
						   double ymax,
						   double x_labelSize,
						   double y_labelSize,
						   const std::string& xTitle,
						   const std::string& yTitle,
						   double msize,
						   int mcolor,
						   int mstyle)
{
  TProfile_LW* hist = TProfile_LW::create(hName.c_str(), hTitle.c_str(), nbins, firstbin, lastbin, ymin, ymax);
  trtRegHist(hist, mongroup, hName.c_str());
  hist->SetMarkerSize(msize);
  hist->SetMarkerStyle(mstyle);
  hist->SetMarkerColor(mcolor);
  hist->GetXaxis()->SetLabelSize(x_labelSize);
  hist->GetYaxis()->SetLabelSize(y_labelSize);
  hist->GetXaxis()->SetTitle(xTitle.c_str());
  hist->GetYaxis()->SetTitle(yTitle.c_str());
  return hist;

}

TH2F_LW* TRT_Electron_Monitoring_Tool::trtBookHistoLW(MonGroup &mongroup,
					      const std::string& hName,
					      const std::string& hTitle,
					      int xnbins,
					      double xfirstbin,
					      double xlastbin,
					      int ynbins,
					      double yfirstbin,
					      double ylastbin,
					      double x_labelSize,
					      double y_labelSize,
					      const std::string& xTitle,
					      const std::string& yTitle)
{
  TH2F_LW* hist = TH2F_LW::create(hName.c_str(), hTitle.c_str(), xnbins, xfirstbin, xlastbin, ynbins, yfirstbin, ylastbin);
  trtRegHist(hist, mongroup, hName.c_str());
  hist->GetXaxis()->SetLabelSize(x_labelSize);
  hist->GetYaxis()->SetLabelSize(y_labelSize);
  hist->GetXaxis()->SetTitle(xTitle.c_str());
  hist->GetYaxis()->SetTitle(yTitle.c_str());
  return hist;

}

int
TRT_Electron_Monitoring_Tool::myBarrelEC(int barrel_ec){
  switch(barrel_ec){
    case  1 : return DET_BARRELA;
    case -1 : return DET_BARRELC;
    case  2 : return DET_ENDCAPA;
    case -2 : return DET_ENDCAPC;
    default : return -1;
  }
  return -1;
}
