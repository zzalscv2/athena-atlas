/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ConcurrencyFlags.h"

#include "TrigT1NSWSimTools/MMTriggerTool.h"

namespace NSWL1 {

  MMTriggerTool::MMTriggerTool( const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type,name,parent),
    m_detManager(nullptr),
    m_MmIdHelper(nullptr),
    m_tree(nullptr)
  {
    declareInterface<NSWL1::IMMTriggerTool>(this);
  }

  StatusCode MMTriggerTool::initialize() {

    ATH_MSG_DEBUG( "initializing -- " << name() );

    ATH_MSG_DEBUG( name() << " configuration:");
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_mmDigitContainer.name() << m_mmDigitContainer.value());
    ATH_MSG_DEBUG(" " << std::setw(32) << std::setfill('.') << std::setiosflags(std::ios::left) << m_doNtuple.name() << ((m_doNtuple)? "[True]":"[False]")
                      << std::setfill(' ') << std::setiosflags(std::ios::right) );

    ATH_CHECK(m_keyMcEventCollection.initialize(m_isMC));
    ATH_CHECK(m_keyMuonEntryLayer.initialize(m_isMC));
    ATH_CHECK(m_keyMmDigitContainer.initialize());

    const IInterface* parent = this->parent();
    const INamedInterface* pnamed = dynamic_cast<const INamedInterface*>(parent);
    const std::string& algo_name = pnamed->name();
    if ( m_doNtuple ) {
      if (Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
        ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
        return StatusCode::FAILURE;
      }

      ATH_CHECK( m_incidentSvc.retrieve() );
      m_incidentSvc->addListener(this,IncidentType::BeginEvent);

      if ( algo_name=="NSWL1Simulation" ) {
        ITHistSvc* tHistSvc;
        ATH_CHECK( service("THistSvc", tHistSvc) );

        m_tree = nullptr;
        std::string ntuple_name = algo_name+"Tree";
        ATH_CHECK( tHistSvc->getTree(ntuple_name,m_tree) );
        ATH_MSG_DEBUG("Analysis ntuple succesfully retrieved");
        ATH_CHECK( this->book_branches() );
      }
    }

    //  retrieve the MuonDetectormanager
    ATH_CHECK( detStore()->retrieve( m_detManager ) );

    //  retrieve the Mm offline Id helper
    ATH_CHECK( detStore()->retrieve( m_MmIdHelper ) );

    //Calculate and retrieve wedge geometry, defined in MMT_struct
    const par_par standard = par_par(0.0009,4,4,0.0035,"xxuvxxuv",true);
    const par_par xxuvuvxx = par_par(0.0009,4,4,0.007,"xxuvuvxx",true,true); //.0035 for uv_tol before...
    const par_par xxuvuvxx_uvroads = par_par(0.0009,4,4,0.0035,"xxuvuvxx",true,true); //.0035 for uv_tol before...

    m_par_large = std::make_shared<MMT_Parameters>(xxuvuvxx,'L', m_detManager);
    m_par_small = std::make_shared<MMT_Parameters>(xxuvuvxx,'S', m_detManager);

    return StatusCode::SUCCESS;
  }

  StatusCode MMTriggerTool::runTrigger(Muon::NSW_TrigRawDataContainer* rdo, const bool do_MMDiamonds) const {
    const auto& ctx = Gaudi::Hive::currentContext();
    int event = ctx.eventID().event_number();
    ATH_MSG_DEBUG("********************************************************* EVENT NUMBER = " << event);

    //////////////////////////////////////////////////////////////
    //                                                          //
    // Load Variables From Containers into our Data Structures  //
    //                                                          //
    //////////////////////////////////////////////////////////////

    std::map<std::string, std::shared_ptr<MMT_Parameters> > pars;
    pars["MML"] = m_par_large;
    pars["MMS"] = m_par_small;
    MMLoadVariables load = MMLoadVariables(&(*(evtStore())), m_detManager, m_MmIdHelper);

    std::map<std::pair<int, unsigned int>,std::vector<digitWrapper> > entries;
    std::map<std::pair<int, unsigned int>,std::map<hitData_key,hitData_entry> > Hits_Data_Set_Time;
    std::map<std::pair<int, unsigned int>,evInf_entry> Event_Info;

    const McEventCollection* ptrMcEventCollection = nullptr;
    const TrackRecordCollection* ptrMuonEntryLayer = nullptr;
    if(m_isMC){
      SG::ReadHandle<McEventCollection> readMcEventCollection( m_keyMcEventCollection );
      if( !readMcEventCollection.isValid() ){
        ATH_MSG_ERROR("Cannot retrieve McEventCollection");
        return StatusCode::FAILURE;
      }
      ptrMcEventCollection = readMcEventCollection.cptr();
      SG::ReadHandle<TrackRecordCollection> readMuonEntryLayer( m_keyMuonEntryLayer );
      if( !readMuonEntryLayer.isValid() ){
        ATH_MSG_ERROR("Cannot retrieve MuonEntryLayer");
        return StatusCode::FAILURE;
      }
      ptrMuonEntryLayer = readMuonEntryLayer.cptr();
    }

    SG::ReadHandle<MmDigitContainer> readMmDigitContainer( m_keyMmDigitContainer );
    if( !readMmDigitContainer.isValid() ){
      ATH_MSG_ERROR("Cannot retrieve MmDigitContainer");
      return StatusCode::FAILURE;
    }
    histogramDigitVariables histDigVars;
    ATH_CHECK( load.getMMDigitsInfo( ptrMcEventCollection, ptrMuonEntryLayer, readMmDigitContainer.cptr(), entries, Hits_Data_Set_Time, Event_Info, pars, histDigVars) );
    if (m_doNtuple) this->fillNtuple(histDigVars);

    if (entries.empty()) {
      ATH_MSG_WARNING("No digits available for processing, exiting");
      Hits_Data_Set_Time.clear();
      Event_Info.clear();
      return StatusCode::SUCCESS;
    }

    std::unique_ptr<MMT_Diamond> diamond = std::make_unique<MMT_Diamond>(m_detManager);
    if (do_MMDiamonds) {
      diamond->setTrapezoidalShape(m_trapShape);
      diamond->setXthreshold(m_diamXthreshold);
      diamond->setUV(m_uv);
      diamond->setUVthreshold(m_diamUVthreshold);
      diamond->setRoadSize(m_diamRoadSize);
      diamond->setRoadSizeUpX(m_diamOverlapEtaUp);
      diamond->setRoadSizeDownX(m_diamOverlapEtaDown);
      diamond->setRoadSizeUpUV(m_diamOverlapStereoUp);
      diamond->setRoadSizeDownUV(m_diamOverlapStereoDown);
    }

      double trueta = -999., truphi = -999., trutheta = -999., trupt = -999., dt = -999., tpos = -999., ppos = -999., epos = -999., tent = -999., pent = -999., eent = -999.;
      // We need to extract truth info, if available
	for (unsigned int j=0; j<Event_Info.size(); j++) {
      std::pair<int, unsigned int> truth_event (event,j);
      if (!Event_Info.empty()) {
        auto tru_it = Event_Info.find(truth_event);
        if (tru_it != Event_Info.end()) {
          evInf_entry truth_info(tru_it->second);
          trutheta = truth_info.theta_ip; // truth muon at the IP
          truphi = truth_info.phi_ip;
          trueta = truth_info.eta_ip;
          trupt = truth_info.pt;
          tpos=truth_info.theta_pos; // muEntry position
          ppos=truth_info.phi_pos;
          epos=truth_info.eta_pos;
          tent=truth_info.theta_ent; // muEntry momentum
          pent=truth_info.phi_ent;
          eent=truth_info.eta_ent;
          dt=truth_info.dtheta;
          if (m_doNtuple) {
            m_trigger_trueEtaRange->push_back(trueta);
            m_trigger_truePtRange->push_back(trupt);
            m_trigger_trueThe->push_back(trutheta);
            m_trigger_truePhi->push_back(truphi);
            m_trigger_trueDth->push_back(dt); // theta_pos-theta_ent
            m_trigger_trueEtaPos->push_back(epos);
            m_trigger_trueThePos->push_back(tpos);
            m_trigger_truePhiPos->push_back(ppos);
            m_trigger_trueEtaEnt->push_back(eent);
            m_trigger_trueTheEnt->push_back(tent);
            m_trigger_truePhiEnt->push_back(pent);
          }
        }
      }
	}
	unsigned int particles = entries.rbegin()->first.second +1,  nskip=0;
	for (unsigned int i=0; i<particles; i++) {
	std::pair<int, unsigned int> pair_event (event,i);

      // Now let's switch to reco hits: firstly, extracting the station name we're working on...
      std::string station = "-";
      auto event_it = entries.find(pair_event);
      station = event_it->second[0].stName; // Station name is taken from the first digit! In MMLoadVariables there's a check to ensure all digits belong to the same station

      // Secondly, extracting the Phi of the station we're working on...
      int stationPhi = -999;
      digitWrapper dW = event_it->second[0];
      Identifier tmpID = dW.id();
      stationPhi = m_MmIdHelper->stationPhi(tmpID);

      // Finally, let's start with hits
      auto reco_it = Hits_Data_Set_Time.find(pair_event);
      if (reco_it != Hits_Data_Set_Time.end()) {
        if (reco_it->second.size() >= (diamond->getXthreshold()+diamond->getUVthreshold())) {
          if (do_MMDiamonds) {
            /*
             * Filling hits for each event: a new class, MMT_Hit, is called in
             * order to use both algorithms witghout interferences
             */
            diamond->createRoads_fillHits(i-nskip, reco_it->second, m_detManager, pars[station], stationPhi);
            if (m_doNtuple) {
              for(const auto &hit : reco_it->second) {
                m_trigger_VMM->push_back(hit.second.VMM_chip);
                m_trigger_plane->push_back(hit.second.plane);
                m_trigger_station->push_back(hit.second.station_eta);
                m_trigger_strip->push_back(hit.second.strip);
              }
              std::vector<double> slopes = diamond->getHitSlopes();
              for (const auto &s : slopes) m_trigger_RZslopes->push_back(s);
              slopes.clear();
            }
            diamond->resetSlopes();
            /*
             * Here we create roads with all MMT_Hit collected before (if any), then we save the results
             */
            diamond->findDiamonds(i-nskip, event);

            if (!diamond->getSlopeVector(i-nskip).empty()) {
              if (m_doNtuple) {
                m_trigger_diamond_ntrig->push_back(diamond->getSlopeVector(i-nskip).size());
                for (const auto &slope : diamond->getSlopeVector(i-nskip)) {
                  m_trigger_diamond_sector->push_back(diamond->getDiamond(i-nskip).sector);
                  m_trigger_diamond_stationPhi->push_back(diamond->getDiamond(i-nskip).stationPhi);
                  m_trigger_diamond_bc->push_back(slope.BC);
                  m_trigger_diamond_totalCount->push_back(slope.totalCount);
                  m_trigger_diamond_realCount->push_back(slope.realCount);
                  m_trigger_diamond_XbkgCount->push_back(slope.xbkg);
                  m_trigger_diamond_UVbkgCount->push_back(slope.uvbkg);
                  m_trigger_diamond_XmuonCount->push_back(slope.xmuon);
                  m_trigger_diamond_UVmuonCount->push_back(slope.uvmuon);
                  m_trigger_diamond_iX->push_back(slope.iRoad);
                  m_trigger_diamond_iU->push_back(slope.iRoadu);
                  m_trigger_diamond_iV->push_back(slope.iRoadv);
                  m_trigger_diamond_age->push_back(slope.age);
                  m_trigger_diamond_mx->push_back(slope.mx);
                  m_trigger_diamond_my->push_back(slope.my);
                  m_trigger_diamond_Uavg->push_back(slope.uavg);
                  m_trigger_diamond_Vavg->push_back(slope.vavg);
                  m_trigger_diamond_mxl->push_back(slope.mxl);
                  m_trigger_diamond_theta->push_back(slope.theta);
                  m_trigger_diamond_eta->push_back(slope.eta);
                  m_trigger_diamond_dtheta->push_back(slope.dtheta);
                  m_trigger_diamond_phi->push_back(slope.phi);
                  m_trigger_diamond_phiShf->push_back(slope.phiShf);
                }
              }

              // MM RDO filling below
              std::vector<int> slopeBC;
              for (const auto &slope : diamond->getSlopeVector(i-nskip)) slopeBC.push_back(slope.BC);
              std::sort(slopeBC.begin(), slopeBC.end());
              slopeBC.erase( std::unique(slopeBC.begin(), slopeBC.end()), slopeBC.end() );
              for (const auto &bc : slopeBC) {
                Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(diamond->getDiamond(i-nskip).stationPhi, diamond->getDiamond(i-nskip).side, bc);

                for (const auto &slope : diamond->getSlopeVector(i-nskip)) {
                  if (bc == slope.BC) {
                    Muon::NSW_TrigRawDataSegment* trigRawDataSegment = new Muon::NSW_TrigRawDataSegment();

                    // Phi-id - here use local phi (not phiShf)
                    uint8_t phi_id = 0;
                    if (slope.phi > m_phiMax || slope.phi < m_phiMin) trigRawDataSegment->setPhiIndex(phi_id);
                    else {
                      uint8_t nPhi = (1<<m_phiBits) -2; // To accomodate the new phi-id encoding prescription around 0
                      float phiSteps = (m_phiMax - m_phiMin)/nPhi;
                      for (uint8_t i=0; i<nPhi; i++) {
                        if ((slope.phi) < (m_phiMin+i*phiSteps)) {
                          phi_id = i;
                          break;
                        }
                      }
                      trigRawDataSegment->setPhiIndex(phi_id);
                    }
                    if (m_doNtuple) m_trigger_diamond_TP_phi_id->push_back(phi_id);

                    // R-id
                    double extrapolatedR = 7824.46*std::abs(std::tan(slope.theta)); // The Z plane is a fixed value, taken from SL-TP documentation
                    uint8_t R_id = 0;
                    if (extrapolatedR > m_rMax || extrapolatedR < m_rMin) trigRawDataSegment->setRIndex(R_id);
                    else {
                      uint8_t nR = (1<<m_rBits) -1;
                      float Rsteps = (m_rMax - m_rMin)/nR;
                      for (uint8_t j=0; j<nR; j++) {
                        if (extrapolatedR < (m_rMin+j*Rsteps)) {
                          R_id = j;
                          break;
                        }
                      }
                      trigRawDataSegment->setRIndex(R_id);
                    }
                    if (m_doNtuple) m_trigger_diamond_TP_R_id->push_back(R_id);

                    // DeltaTheta-id
                    uint8_t dTheta_id = 0;
                    if (slope.dtheta > m_dThetaMax || slope.dtheta < m_dThetaMin) trigRawDataSegment->setDeltaTheta(dTheta_id);
                    else {
                      uint8_t ndTheta = (1<<m_dThetaBits) -1;
                      float dThetaSteps = (m_dThetaMax - m_dThetaMin)/ndTheta;
                      for (uint8_t k=0; k<ndTheta; k++) {
                        if ((slope.dtheta) < (m_dThetaMin+k*dThetaSteps)) {
                          dTheta_id = k;
                          break;
                        }
                      }
                      trigRawDataSegment->setDeltaTheta(dTheta_id);
                    }
                    if (m_doNtuple) m_trigger_diamond_TP_dTheta_id->push_back(dTheta_id);

                    // Low R-resolution bit
                    trigRawDataSegment->setLowRes(slope.lowRes);

                    trigRawData->push_back(trigRawDataSegment);
                  }
                }
                rdo->push_back(trigRawData);
              }
              ATH_MSG_DEBUG("Filled MM RDO container now having size: " << rdo->size() << ". Clearing event information!");
            } else ATH_MSG_DEBUG("No output slopes to store");
          } else {
            //////////////////////////////////////////////////////////////
            //                                                          //
            //                Finder Applied Here                       //
            //                                                          //
            //////////////////////////////////////////////////////////////

            //Initialization of the finder: defines all the roads
            auto find = std::make_unique<MMT_Finder>(pars[station], 1);
            ATH_MSG_DEBUG(  "Number of Roads Configured " <<  find->get_roads()  );

            std::vector<hitData_entry> hitDatas;
            for (const auto &hit_it : reco_it->second) hitDatas.push_back(hit_it.second);
            std::map<std::pair<int,int>,finder_entry> hitBuffer;
            for (const auto &hit_it : reco_it->second) {
              find->fillHitBuffer( hitBuffer, hit_it.second.entry_hit(pars[station]), pars[station] ); // Hit object, Map (road,plane) -> Finder entry
  
              hitData_info hitInfo = hit_it.second.entry_hit(pars[station]).info;
              if (m_doNtuple) {
                m_trigger_VMM->push_back(hit_it.second.VMM_chip);
                m_trigger_plane->push_back(hit_it.second.plane);
                m_trigger_station->push_back(hit_it.second.station_eta);
                m_trigger_strip->push_back(hit_it.second.strip);
                m_trigger_slope->push_back(hitInfo.slope);
              }
            }
            if (reco_it->second.size() > 7 && m_doNtuple) {
              m_trigger_trueEtaRange->push_back(trueta);
              m_trigger_truePtRange->push_back(trupt);
              if (station == "MML") {
                m_trigger_large_trueEtaRange->push_back(trueta);
                m_trigger_large_truePtRange->push_back(trupt);
              }
              if (station == "MMS") {
                m_trigger_small_trueEtaRange->push_back(trueta);
                m_trigger_small_truePtRange->push_back(trupt);
              }
            }

            ////////////////////////////////////////////////////////////////
            ////                                                          //
            ////                 Fitter Applied Here                      //
            ////                                                          //
            ////////////////////////////////////////////////////////////////

            auto fit = std::unique_ptr<MMT_Fitter>(new MMT_Fitter());

            //First loop over the roads and planes and apply the fitter
            int fits_occupied = 0;
            const int nfit_max = 1;  //MOVE THIS EVENTUALLY
            int nRoads = find->get_roads();

            std::vector<evFit_entry> road_fits = std::vector<evFit_entry>(nRoads,evFit_entry());

            //Variables saved for Alex T. for hardware validation
            double mxl;
            double fillmxl = -999;
            double muGlobal;
            double mvGlobal;
            std::vector<std::pair<double,double> > mxmy;

            for (int iRoad = 0; iRoad < nRoads; iRoad++) {
              std::vector<bool> plane_is_hit;
              std::vector<Hit> track;

              //Check if there are hits in the buffer
              find->checkBufferForHits(  plane_is_hit, // Empty, To be filled by function.
                                         track,        // Empty, To be filled by function.
                                         iRoad,        // roadID
                                         hitBuffer,    // All hits. Map ( (road,plane) -> finder_entry  )
                                         pars[station] // Pointer to geometrical info class
                                      );

              //Look for coincidences
              int road_num = find->Coincidence_Gate(plane_is_hit, pars[station]);
              if (road_num > 0) {
                if (fits_occupied >= nfit_max) break;

                //Perform the fit -> calculate local, global X, UV slopes -> calculate ROI and TriggerTool signal (theta, phi, deltaTheta)
                evFit_entry candidate = fit->fit_event(event,track,hitDatas,fits_occupied,mxmy,mxl,mvGlobal,muGlobal,pars[station]);

                ATH_MSG_DEBUG( "THETA " << candidate.fit_theta << " PHI " << candidate.fit_phi << " DTH " << candidate.fit_dtheta );
                road_fits[iRoad] = candidate;
                fillmxl = mxl;
                fits_occupied++;
              }
              road_fits[iRoad].hcode = road_num;
            } //end roads

            //////////////////////////////////////////////////////////////
            //                                                          //
            //              Pass the ROI as Signal                      //
            //                                                          //
            //////////////////////////////////////////////////////////////

            if (road_fits.empty() and hitDatas.size() == 8) ATH_MSG_DEBUG( "TruthRF0 " << tpos     << " " << ppos   << " " << dt << " " << trueta );

            for (unsigned int i = 0; i < road_fits.size(); i++) {
              if (road_fits[i].fit_roi == 0 and hitDatas.size() == 8) {
                ATH_MSG_DEBUG( "TruthROI0 " << tpos     << " " << ppos   << " " << dt << " " << trueta );
              }
              if (road_fits[i].fit_roi > 0) {
                //For the future: how do we want these to pass on as the signal?  Some new data structure?
                double fitTheta      = road_fits[i].fit_theta;
                double fitPhi        = road_fits[i].fit_phi;
                double fitDeltaTheta = road_fits[i].fit_dtheta;

                //need a correction for the fitted phi, taken from phi_shift in MMLoadVariables.cxx (now it's local)
                int wedge = 0;
                if (station == "MML") wedge = 0;
                else if (station == "MMS") wedge = 1;
                float n = 2*(stationPhi-1) + wedge;
                float shift = n * M_PI/8.;
                if(n > 8) shift = (16.-n)*M_PI/8.;
                if(n < 8)       fitPhi = (fitPhi + shift);
                else if(n == 8) fitPhi = (fitPhi + (fitPhi >= 0 ? -1 : 1)*shift);
                else if(n > 8)  fitPhi = (fitPhi - shift);

                double fitEta = -1. * std::log(std::tan(fitTheta/2.)); //VALE: trueta was filled!!!!

                ATH_MSG_DEBUG( "Truth " << tpos     << " " << ppos   << " " << dt );
                ATH_MSG_DEBUG( "FIT!! " << fitTheta << " " << fitPhi << " " << fitDeltaTheta );
                if (m_doNtuple) {
                  m_trigger_fitThe->push_back(fitTheta);
                  m_trigger_fitPhi->push_back(fitPhi);
                  m_trigger_fitDth->push_back(fitDeltaTheta);

                  m_trigger_resThe->push_back(fitTheta-tpos);
                  m_trigger_resPhi->push_back(fitPhi-ppos);
                  m_trigger_resDth->push_back(fitDeltaTheta-dt);

                  m_trigger_mx->push_back(mxmy.front().first);
                  m_trigger_my->push_back(mxmy.front().second);
                  m_trigger_mxl->push_back(fillmxl);

                  m_trigger_mu->push_back(muGlobal);
                  m_trigger_mv->push_back(mvGlobal);

                  m_trigger_fitEtaRange->push_back(fitEta);
                  m_trigger_fitPtRange->push_back(trupt);
                  if (station == "MML") {
                    m_trigger_large_fitEtaRange->push_back(fitEta);
                    m_trigger_large_fitPtRange->push_back(trupt);
                  }
                  if (station == "MMS") {
                    m_trigger_small_fitEtaRange->push_back(fitEta);
                    m_trigger_small_fitPtRange->push_back(trupt);
                  }
                }
              }
            }
            hitDatas.clear();
          } // if-else Diamond clause
        } else {
          ATH_MSG_DEBUG( "Available hits are " << reco_it->second.size() << ", less than X+UV threshold, skipping" );
          nskip++;
        }
      } else {
          ATH_MSG_WARNING( "Empty hit map, skipping" );
          nskip++;
      }
    } // Main particle loop
    entries.clear();
    Hits_Data_Set_Time.clear();
    Event_Info.clear();
    if (do_MMDiamonds) diamond->clearEvent();

    return StatusCode::SUCCESS;
  }
}//end namespace
