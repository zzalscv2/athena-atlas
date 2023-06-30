/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TMath.h"

#include "FPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include <AsgMessaging/MessageCheck.h>
#include "TruthUtils/MagicNumbers.h"

using namespace asg::msgUserCode;

FPGATrackSimEventSelectionSvc::FPGATrackSimEventSelectionSvc(const std::string& name, ISvcLocator* svc) :
  base_class(name,svc)
{}

StatusCode FPGATrackSimEventSelectionSvc::initialize()
{
  if (!m_regions) createRegions();

  m_min = m_regions->getMin(m_regionID);
  m_max = m_regions->getMax(m_regionID);

  if (m_sampleType.value() == "skipTruth") m_st = SampleType::skipTruth;
  else if (m_sampleType.value() == "singleElectrons") m_st = SampleType::singleElectrons;
  else if (m_sampleType.value() == "singleMuons") m_st = SampleType::singleMuons;
  else if (m_sampleType.value() == "singlePions") m_st = SampleType::singlePions;
  else if (m_sampleType.value() == "LLPs") m_st = SampleType::LLPs;
  else {
    ATH_MSG_ERROR("initialize(): sampleType doesn't exist. ");
    return StatusCode::FAILURE;
  }

  // If we are doing large-radius tracks, we want to restrict the hits entering the 
  // process as before, but we will accept any track as belonging to this region that has
  // most of its hits within the region, regardless of eta and phi.
  // We will also set a minimum pT of 5 GeV.
  m_trackmin = m_min;
  m_trackmax = m_max;
  double pTCutVal = 1./(1000.*m_minLRTpT);
  if (m_LRT) {
    m_trackmin.qOverPt = -1*pTCutVal;
    m_trackmin.d0 = -300;
    m_trackmin.z0 = -500;
    m_trackmax.qOverPt = pTCutVal;
    m_trackmax.d0 = 300;
    m_trackmax.z0 = 500;
  }

  // Should really be printing the full information, but it conflicts with 
  // the requirement that log files be identical .....
  ATH_MSG_INFO("FPGATrackSimEventSelectionSvc::initialize(): "       <<
               "regionID = "   << m_regionID                << ", "  <<
               "min = {"       << m_min                     << "}, " <<
               "max = {"       << m_max                     << "}, " <<
               "sampleType = " << std::string(m_sampleType) << ", "  <<
               "withPU = "     << static_cast<bool>(m_withPU));

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimEventSelectionSvc::finalize()
{
  if (m_regions) delete m_regions;
  ATH_MSG_INFO("FPGATrackSimEventSelectionSvc::finalize()");

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimEventSelectionSvc::queryInterface(const InterfaceID& riid, void** ppvIf)
{
 if ( !ppvIf ) return StatusCode::FAILURE;
 
 // find indirect interfaces :
 if (IFPGATrackSimEventSelectionSvc::interfaceID().versionMatch(riid)) {
  *ppvIf = dynamic_cast<IFPGATrackSimEventSelectionSvc*>(this);
 } else if (base_class::queryInterface(riid, ppvIf).isSuccess()) {
   return StatusCode::SUCCESS;
 } else {
   // Interface is not directly available: try out a base class
   return ::AthService::queryInterface(riid, ppvIf);
 }
 addRef();
 return StatusCode::SUCCESS;
}


bool FPGATrackSimEventSelectionSvc::passCuts(const FPGATrackSimHit& hit) const
{
  float eta = TMath::ASinH(hit.getGCotTheta());
  float phi = hit.getGPhi();

  if (eta >= m_min.eta && eta <= m_max.eta
      &&  phi >= m_min.phi && phi <= m_max.phi)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passCuts(const FPGATrackSimTrack& track) const
{
  if (track.getQOverPt() >= m_trackmin.qOverPt && track.getQOverPt() <= m_trackmax.qOverPt
      && track.getEta() >= m_trackmin.eta && track.getEta() <= m_trackmax.eta
      && track.getPhi() >= m_trackmin.phi && track.getPhi() <= m_trackmax.phi
      && track.getD0() >= m_trackmin.d0 && track.getD0() <= m_trackmax.d0
      && track.getZ0() >= m_trackmin.z0 && track.getZ0() <= m_trackmax.z0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passQOverPt(const FPGATrackSimTrack& track) const
{
  if (track.getQOverPt() >= m_trackmin.qOverPt && track.getQOverPt() <= m_trackmax.qOverPt)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passEta(const FPGATrackSimTrack& track) const
{
  if (track.getEta() >= m_trackmin.eta && track.getEta() <= m_trackmax.eta)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passPhi(const FPGATrackSimTrack& track) const
{
  if (track.getPhi() >= m_trackmin.phi && track.getPhi() <= m_trackmax.phi)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passD0(const FPGATrackSimTrack& track) const
{
  if (track.getD0() >= m_trackmin.d0 && track.getD0() <= m_trackmax.d0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passZ0(const FPGATrackSimTrack& track) const
{
  if (track.getZ0() >= m_trackmin.z0 && track.getZ0() <= m_trackmax.z0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passCuts(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getQOverPt() >= m_trackmin.qOverPt && offlineTrack.getQOverPt() <= m_trackmax.qOverPt
      && offlineTrack.getEta() >= m_trackmin.eta && offlineTrack.getEta() <= m_trackmax.eta
      && offlineTrack.getPhi() >= m_trackmin.phi && offlineTrack.getPhi() <= m_trackmax.phi
      && offlineTrack.getD0() >= m_trackmin.d0 && offlineTrack.getD0() <= m_trackmax.d0
      && offlineTrack.getZ0() >= m_trackmin.z0 && offlineTrack.getZ0() <= m_trackmax.z0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passQOverPt(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getQOverPt() >= m_trackmin.qOverPt && offlineTrack.getQOverPt() <= m_trackmax.qOverPt)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passEta(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getEta() >= m_trackmin.eta && offlineTrack.getEta() <= m_trackmax.eta)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passPhi(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getPhi() >= m_trackmin.phi && offlineTrack.getPhi() <= m_trackmax.phi)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passD0(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getD0() >= m_trackmin.d0 && offlineTrack.getD0() <= m_trackmax.d0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passZ0(const FPGATrackSimOfflineTrack& offlineTrack) const
{
  if (offlineTrack.getZ0() >= m_trackmin.z0 && offlineTrack.getZ0() <= m_trackmax.z0)
    return true;
  else
    return false;
}

bool FPGATrackSimEventSelectionSvc::passCuts(const FPGATrackSimTruthTrack& truthTrack) const
{
  // Want a version that allows us to modify the selection parameters here.
  // If m_trackmin and m_trackmax are unmodified from the region definitions
  // this should return exactly the same.
  FPGATrackSimTrackPars cur = truthTrack.getPars();
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
      if (cur[i] < m_trackmin[i]) {
        return false;
      }
      if (cur[i] > m_trackmax[i]) {
        return false;      
      }
    }

  ATH_MSG_DEBUG("Passing it. PDGid = " << truthTrack.getPDGCode() << "; barcode = " << truthTrack.getBarcode());
  return true;  
}

bool FPGATrackSimEventSelectionSvc::passMatching(FPGATrackSimTrack const & track) const
{
  if (track.getBarcode() == 0 || track.getBarcode() == std::numeric_limits<long>::max()) return false;
  if (track.getEventIndex() == std::numeric_limits<long>::max()) return false;
  if (track.getQOverPt() == 0) return false;

  return true;
}

bool FPGATrackSimEventSelectionSvc::passMatching(FPGATrackSimTruthTrack const & truthTrack) const
{
  if (truthTrack.getBarcode() == 0 || truthTrack.getQ() == 0) return false;
  if ((!m_allowHighBarcode) && m_st != SampleType::skipTruth && HepMC::is_simulation_particle(truthTrack.getBarcode())) return false;
  if (!passCuts(truthTrack)) return false;

  return true;
}

bool FPGATrackSimEventSelectionSvc::selectEvent(FPGATrackSimEventInputHeader* eventHeader) const
{
  if (m_st == SampleType::skipTruth)
    return true;
  else if (m_st == SampleType::singleElectrons || m_st == SampleType::singleMuons || m_st == SampleType::singlePions){
    const auto& truthTracks = eventHeader->optional().getTruthTracks();
    return checkTruthTracks(truthTracks);
  }
  else if (m_st == SampleType::LLPs) {
    const auto& truthTracks = eventHeader->optional().getTruthTracks();
    // Maybe change this later - could be we want LRT selection in all cases but I suspect not
    if (m_LRT) return checkTruthTracksLRT(truthTracks);
    else return checkTruthTracks(truthTracks);
  }
  else {
    ATH_MSG_DEBUG("selectEvent(): Error with sampleType property");
    return false;
  }
}

bool FPGATrackSimEventSelectionSvc::selectEvent(FPGATrackSimLogicalEventInputHeader* eventHeader) const
{
  if (m_st == SampleType::skipTruth)
    return true;
  else if (m_st == SampleType::singleElectrons || m_st == SampleType::singleMuons || m_st == SampleType::singlePions){
    const auto& truthTracks = eventHeader->optional().getTruthTracks();
    return checkTruthTracks(truthTracks);
  }
  else if (m_st == SampleType::LLPs) {
    const auto& truthTracks = eventHeader->optional().getTruthTracks();
    // Maybe change this later - could be we want LRT selection in all cases but I suspect not
    if (m_LRT) return checkTruthTracksLRT(truthTracks);
    else return checkTruthTracks(truthTracks);
  }  
  else {
    ATH_MSG_DEBUG("selectEvent(): Error with sampleType property");
    return false;
  }

}

const FPGATrackSimRegionSlices* FPGATrackSimEventSelectionSvc::getRegions()
{
  if (!m_regions) createRegions();
  return m_regions;
}

void FPGATrackSimEventSelectionSvc::createRegions()
{
  if (!m_regions)
    {
      ATH_MSG_INFO("Creating the slices object");
      MsgStream cmsg(msgSvc(), "FPGATrackSimRegionSlices");
      cmsg.setLevel(msg().level()); // cause AthMessaging is stupid and doesn't have this function
      m_regions = new FPGATrackSimRegionSlices(m_regions_path.value());
    }
}

bool FPGATrackSimEventSelectionSvc::checkTruthTracks(const std::vector<FPGATrackSimTruthTrack>& truthTracks) const
{  
// find at least one track in the region
  bool good=false;
  for (auto track : truthTracks){
    if(m_regions->inRegion(m_regionID, track)){      
      good=true;
      if (std::abs(track.getPDGCode()) != static_cast<int>(m_st)) {
	      ATH_MSG_WARNING("selectEvent(): TruthTrack PDGCode != sampleType");
	      good=false;
      } 
      else {    
	    ATH_MSG_DEBUG("selectEvent(): found one truth track, in region "
		      <<getRegionID() <<"; track pars: "<< truthTracks.front().getPars());
	    break;
      }
    }
    else {
      ATH_MSG_DEBUG("selectEvent(): found one truth track over "<<truthTracks.size()<<", out of region "
		    <<getRegionID() <<"; track pars: "<< truthTracks.front().getPars());
    }
  }
  return good;
  
}

bool FPGATrackSimEventSelectionSvc::checkTruthTracksLRT(const std::vector<FPGATrackSimTruthTrack>& truthTracks) const
{
  ATH_MSG_DEBUG("selectEvent(): Checking truth tracks with LRT requirements"); 
  // Ideally we would make this flexible to use either of two conditions
  // 1) there is a truth track that passes our region criteria with our modified selections, or
  // 2) there is a truth track that passes all but the eta and phi requirements, but has
  // the majority of its associated hits in the region.

  // Check for 1)
  for (const auto& truthtrack : truthTracks ) {
    // If we specified a PDG ID selection, it can go here - but by default this is false
    if ((m_LRT_pdgID != 0) && (m_LRT_pdgID != truthtrack.getPDGCode())) {
      if (passCuts(truthtrack)) ANA_MSG_DEBUG("Skipping an otherwise passing track due to wrong PDGID: " << truthtrack.getPDGCode());
      continue;
    }
    if (passCuts(truthtrack)) return true;
  }

  // Check for 2)
  // TODO someday.


  // Otherwise return false
  return false;
}
