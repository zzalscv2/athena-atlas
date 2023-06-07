/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMLoadVariables.h"
#include "AtlasHepMC/MagicNumbers.h"

MMLoadVariables::MMLoadVariables(StoreGateSvc* evtStore, const MuonGM::MuonDetectorManager* detManager, const MmIdHelper* idhelper):
   AthMessaging(Athena::getMessageSvc(), "MMLoadVariables") {
      m_evtStore = evtStore;
      m_detManager = detManager;
      m_MmIdHelper = idhelper;
}

StatusCode MMLoadVariables::getMMDigitsInfo(const McEventCollection *truthContainer,
                                            const TrackRecordCollection* trackRecordCollection,
                                            const MmDigitContainer *nsw_MmDigitContainer,
                                            std::map<std::pair<int,unsigned int>,std::vector<digitWrapper> >& entries,
                                            std::map<std::pair<int,unsigned int>,std::map<hitData_key,hitData_entry> >& Hits_Data_Set_Time,
                                            std::map<std::pair<int,unsigned int>,evInf_entry>& Event_Info,
                                            std::map<std::string, std::shared_ptr<MMT_Parameters> > &pars,
                                            histogramDigitVariables &histDigVars) const {
      //*******Following MuonPRD code to access all the variables**********
      std::vector<ROOT::Math::PtEtaPhiEVector> truthParticles, truthParticles_ent, truthParticles_pos;
      std::vector<int> pdg;
      std::vector<ROOT::Math::XYZVector> vertex;
      float phiEntry_tmp    = 0;
      float phiPosition_tmp = 0;
      float etaEntry_tmp    = 0;
      float etaPosition_tmp = 0;
      int pdg_tmp           = 0;
      ROOT::Math::XYZVector vertex_tmp(0.,0.,0.);
 
      ROOT::Math::PtEtaPhiEVector thePart, theInfo;
      auto MuEntry_Particle_n = (trackRecordCollection!=nullptr)?trackRecordCollection->size():0;
      int j=0; // iteration of particle entries
      if( truthContainer != nullptr ){
      for(const auto it : *truthContainer) {
        const HepMC::GenEvent *subEvent = it;
#ifdef HEPMC3
        for(const auto& particle : subEvent->particles()){
#else
        for(const auto pit : subEvent->particle_range()){
          const HepMC::GenParticle *particle = pit;
#endif
          const HepMC::FourVector momentum = particle->momentum();
          int barcodeparticle = HepMC::barcode(particle);
          if( HepMC::generations(barcodeparticle) < 1 && std::abs(particle->pdg_id())==13){
            thePart.SetCoordinates(momentum.perp(),momentum.eta(),momentum.phi(),momentum.e());
            if(trackRecordCollection!=nullptr){
            for(const auto & mit : *trackRecordCollection ) {
              const CLHEP::Hep3Vector mumomentum = mit.GetMomentum();
              const CLHEP::Hep3Vector muposition = mit.GetPosition();
              if(!trackRecordCollection->empty() && barcodeparticle ==mit.GetBarCode()) {
                pdg_tmp         = particle->pdg_id();
                phiEntry_tmp    = mumomentum.getPhi();
                etaEntry_tmp    = mumomentum.getEta();
                phiPosition_tmp = muposition.getPhi();
                etaPosition_tmp = muposition.getEta();
              }
            }//muentry loop
            } // trackRecordCollection is not null
#ifdef HEPMC3
            vertex_tmp = subEvent->vertices().front()->position();
#else
            int l=0;
            for(const auto vit : subEvent->vertex_range())
            {
              if(l!=0){break;}//get first vertex of iteration, may want to change this
              l++;
              const HepMC::GenVertex *vertex1 = vit;
              const HepMC::FourVector& position = vertex1->position();
              vertex_tmp.SetXYZ(position.x(),position.y(),position.z());
            }//end vertex loop
#endif
          }
          j++;

            if(thePart.Pt() > 0. && HepMC::generations(barcodeparticle) < 1){
              bool addIt = true;
              for(unsigned int ipart=0; ipart < truthParticles.size(); ipart++){
                if( std::abs(thePart.Pt()-truthParticles[ipart].Pt()) < 0.001 ||
                    std::abs(thePart.Eta()-truthParticles[ipart].Eta()) < 0.001 ||
                    std::abs(xAOD::P4Helpers::deltaPhi(thePart.Phi(), truthParticles[ipart].Phi())) < 0.001 ||
                    std::abs(thePart.E()-truthParticles[ipart].E()) < 0.001 ) addIt = false;
              }
              if(addIt){
                truthParticles.push_back(thePart);
                //new stuff
                vertex.push_back(vertex_tmp);
                pdg.push_back(pdg_tmp);
                truthParticles_ent.push_back(ROOT::Math::PtEtaPhiEVector(momentum.perp(),etaEntry_tmp   ,phiEntry_tmp   ,momentum.e()));
                truthParticles_pos.push_back(ROOT::Math::PtEtaPhiEVector(momentum.perp(),etaPosition_tmp,phiPosition_tmp,momentum.e()));
              }
            }

        } //end particle loop
      } //end truth container loop (should be only 1 container per event)
      } // if truth container is not null
      const auto& ctx = Gaudi::Hive::currentContext();
      int event = ctx.eventID().event_number();

      int TruthParticle_n = j;
      evFit_entry fit;
      fit.athena_event=event;

      unsigned int digit_particles = 0;
      for(auto digitCollectionIter : *nsw_MmDigitContainer) {
        // a digit collection is instanciated for each container, i.e. holds all digits of a multilayer
        const MmDigitCollection* digitCollection = digitCollectionIter;
        // loop on all digits inside a collection, i.e. multilayer
        std::vector<digitWrapper> entries_tmp;

        for (const auto item:*digitCollection) {
            // get specific digit and identify it
            const MmDigit* digit = item;
            Identifier id = digit->identify();
            if (!m_MmIdHelper->is_mm(id)) continue;

            std::string stName   = m_MmIdHelper->stationNameString(m_MmIdHelper->stationName(id));
            int stationName      = m_MmIdHelper->stationName(id);
            int stationEta       = m_MmIdHelper->stationEta(id);
            int stationPhi       = m_MmIdHelper->stationPhi(id);
            int multiplet        = m_MmIdHelper->multilayer(id);
            int gas_gap          = m_MmIdHelper->gasGap(id);
            int channel          = m_MmIdHelper->channel(id);

            const MuonGM::MMReadoutElement* rdoEl = m_detManager->getMMReadoutElement(id);

            std::vector<float>  time          = digit->stripTimeForTrigger();
            std::vector<float>  charge        = digit->stripChargeForTrigger();
            std::vector<int>    stripPosition = digit->stripPositionForTrigger();
            std::vector<int>    MMFE_VMM = digit->MMFE_VMM_idForTrigger();
            std::vector<int>    VMM = digit->VMM_idForTrigger();

            bool isValid = false;
            histDigVars.NSWMM_dig_stationName.push_back(stName);
            histDigVars.NSWMM_dig_stationEta.push_back(stationEta);
            histDigVars.NSWMM_dig_stationPhi.push_back(stationPhi);
            histDigVars.NSWMM_dig_multiplet.push_back(multiplet);
            histDigVars.NSWMM_dig_gas_gap.push_back(gas_gap);
            histDigVars.NSWMM_dig_channel.push_back(channel);

            std::vector<double> localPosX;
            std::vector<double> localPosY;
            std::vector<double> globalPosX;
            std::vector<double> globalPosY;
            std::vector<double> globalPosZ;

            int nstrip = 0; //counter of the number of firing strips
            for (const auto &i: stripPosition) {

	      isValid = false; //reset
              // take strip index form chip information
              int cr_strip = i;
              localPosX.push_back (0.);
              localPosY.push_back (0.);
              globalPosX.push_back(0.);
              globalPosY.push_back(0.);
              globalPosZ.push_back(0.);
              ++nstrip;

              Identifier cr_id = m_MmIdHelper->channelID(stationName, stationEta, stationPhi, multiplet, gas_gap, cr_strip, isValid);
              if (!isValid) {
                ATH_MSG_WARNING("MicroMegas digitization: failed to create a valid ID for (chip response) strip n. " << cr_strip
                               << "; associated positions will be set to 0.0.");
              } else {
                  // asking the detector element to get local position of strip
                  Amg::Vector2D cr_strip_pos(0., 0.);
                  if ( !rdoEl->stripPosition(cr_id,cr_strip_pos) ) {
                    ATH_MSG_WARNING("MicroMegas digitization: failed to associate a valid local position for (chip response) strip n. " << cr_strip
                                   << "; associated positions will be set to 0.0.");
                  } else {
                    localPosX[nstrip-1] = cr_strip_pos.x();
                    localPosY[nstrip-1] = cr_strip_pos.y();
                  }

                  // asking the detector element to transform this local to the global position
                  Amg::Vector3D cr_strip_gpos(0., 0., 0.);
                  rdoEl->surface(cr_id).localToGlobal(cr_strip_pos, Amg::Vector3D(0., 0., 0.), cr_strip_gpos);
                  globalPosX[nstrip-1] = cr_strip_gpos[0];
                  globalPosY[nstrip-1] = cr_strip_gpos[1];
                  globalPosZ[nstrip-1] = cr_strip_gpos[2];
              }
            }//end of strip position loop

            //NTUPLE FILL DIGITS
            histDigVars.NSWMM_dig_time.push_back(time);
            histDigVars.NSWMM_dig_charge.push_back(charge);
            histDigVars.NSWMM_dig_stripPosition.push_back(stripPosition);
            histDigVars.NSWMM_dig_stripLposX.push_back(localPosX);
            histDigVars.NSWMM_dig_stripLposY.push_back(localPosY);
            histDigVars.NSWMM_dig_stripGposX.push_back(globalPosX);
            histDigVars.NSWMM_dig_stripGposY.push_back(globalPosY);
            histDigVars.NSWMM_dig_stripGposZ.push_back(globalPosZ);
            if(globalPosY.empty()) continue;

            if (!time.empty()) entries_tmp.push_back(
              digitWrapper(digit, stName, -1.,
                           ROOT::Math::XYZVector(-999, -999, -999),
                           ROOT::Math::XYZVector(localPosX[0], localPosY[0], -999),
                           ROOT::Math::XYZVector(globalPosX[0], globalPosY[0], globalPosZ[0] )
                          ) );
        } //end iterator digit loop

        if (!entries_tmp.empty()) {
          std::vector<std::string> stNames;
          for(const auto &dW : entries_tmp) stNames.push_back(dW.stName);
          if(std::all_of(stNames.begin(), stNames.end(), [&] (const std::string & name) { return name == stNames[0]; })) {
            entries[std::make_pair(event,digit_particles)]=entries_tmp;
            digit_particles++;
          }
          stNames.clear();
        }
      } // end digit container loop

      for(unsigned int i=0; i<truthParticles.size(); i++) {
        evInf_entry particle_info(event, pdg[i],
                    truthParticles[i].E(), truthParticles[i].Pt(),
                    truthParticles[i].Eta(), truthParticles_pos[i].Eta(), truthParticles_ent[i].Eta(),
                    truthParticles[i].Phi(), truthParticles_pos[i].Phi(), truthParticles_ent[i].Phi(),
                    truthParticles[i].Theta(), truthParticles_pos[i].Theta(), truthParticles_ent[i].Theta(), truthParticles_ent[i].Theta()-truthParticles_pos[i].Theta(),
                    TruthParticle_n,MuEntry_Particle_n,vertex[i]);
        particle_info.NUV_bg_preVMM = 0;
        Event_Info[std::make_pair(event,i)] = particle_info;
      }


      //Loop over entries, which has digitization info for each event
      unsigned int ient=0;
      for (auto it=entries.begin(); it!=entries.end(); it++){

        /* Identifying the wedge from digits:
         * now the digit is associated with the corresponding station, so lambda function can be exploited to check if they are all the same
         */
        double tru_phi = -999, tru_theta = -999;
        std::pair<int, unsigned int> pair (event,ient);
        auto tru_it = Event_Info.find(pair);
        if (tru_it != Event_Info.end()) {
          tru_phi = tru_it->second.phi_pos;
          tru_theta = tru_it->second.theta_pos;
        }

        std::string station = it->second[0].stName;
        bool uvxxmod=(pars[station]->setup.compare("xxuvuvxx")==0);

        std::map<hitData_key,hitData_entry> hit_info;
        std::vector<hitData_key> keys;

        //Now we need to loop on digits
        for (const auto &dW : it->second) {
          Identifier tmpID      = dW.id();
          int thisMultiplet     = m_MmIdHelper->multilayer( tmpID );
          int thisGasGap        = m_MmIdHelper->gasGap( tmpID );
          int thisTime          = dW.digit->stripTimeForTrigger()[0];
          int thisCharge        = 2; //dW.digit->stripChargeForTrigger().at(0);
          int thisStripPosition = dW.digit->stripPositionForTrigger()[0];
          double thisLocalPosX  = dW.strip_lpos.X();
          int thisVMM           = dW.digit->VMM_idForTrigger()[0];
          int thisMMFE_VMM      = dW.digit->MMFE_VMM_idForTrigger()[0];
          int thisStationEta    = m_MmIdHelper->stationEta( tmpID );
          int thisStationPhi    = m_MmIdHelper->stationPhi( tmpID );
          int thisPlane = (thisMultiplet-1)*4+thisGasGap-1;
          int BC_id = std::ceil( thisTime / 25. );
          ROOT::Math::XYZVector mazin_check(
            dW.strip_gpos.X(),
            dW.strip_gpos.Y(),
            dW.strip_gpos.Z()
            );

          ROOT::Math::XYZVector athena_rec(dW.strip_gpos);
          ROOT::Math::XYZVector recon(athena_rec.Y(),-athena_rec.X(),athena_rec.Z());

          if(uvxxmod){
            xxuv_to_uvxx(recon,thisPlane,pars[station]);
          }

          //We're doing everything by the variable known as "athena_event" to reflect C++ vs MATLAB indexing
          int btime=(event+1)*10+(BC_id-1);
          int special_time = thisTime + (event+1)*100;

          hitData_entry hit_entry(event,
                               thisTime,
                               thisCharge,
                               thisVMM,
                               thisMMFE_VMM,
                               thisPlane,
                               thisStripPosition,
                               thisStationEta,
                               thisStationPhi,
                               thisMultiplet,
                               thisGasGap,
                               thisLocalPosX,
                               tru_theta,
                               tru_phi,
                               true,
                               btime,
                               special_time,
                               mazin_check,
                               mazin_check);

          hit_info[hit_entry.entry_key()]=hit_entry;
          ATH_MSG_DEBUG("Filling hit_info slot: ");
          if (msgLvl(MSG::DEBUG)){
            hit_entry.entry_key().print();
          }
          keys.push_back(hit_entry.entry_key());
        }//end digit wrapper loop

        if (tru_it != Event_Info.end()) {
          tru_it->second.N_hits_preVMM=hit_info.size();
          tru_it->second.N_hits_postVMM=0;
        }

        std::vector<bool>plane_hit(pars[station]->setup.size(),false);

        for(std::map<hitData_key,hitData_entry>::iterator it=hit_info.begin(); it!=hit_info.end(); ++it){
          int plane=it->second.plane;
          plane_hit[plane]=true;
          if (tru_it != Event_Info.end()) tru_it->second.N_hits_postVMM++;
        }
        Hits_Data_Set_Time[std::make_pair(event,ient)] = hit_info;
        ient++;
      }
    return StatusCode::SUCCESS;
  }

  double MMLoadVariables::phi_shift(double athena_phi,const std::string& wedgeType, int stationPhi) const{
    float n = 2*(stationPhi-1);
    if(wedgeType=="Small") n+=1;
    float sectorPi = n*M_PI/8.;
    if(n>8) sectorPi = (16.-n)*M_PI/8.;

    if(n<8)       return (athena_phi-sectorPi);
    else if(n==8) return (athena_phi + (athena_phi >= 0? -1:1)*sectorPi);
    else if(n>8)  return (athena_phi+sectorPi);
    else return athena_phi;

  }
  void MMLoadVariables::xxuv_to_uvxx(ROOT::Math::XYZVector& hit,const int plane, std::shared_ptr<MMT_Parameters> par) const{
    if(plane<4)return;
    else if(plane==4)hit_rot_stereo_bck(hit, par);//x to u
    else if(plane==5)hit_rot_stereo_fwd(hit, par);//x to v
    else if(plane==6)hit_rot_stereo_fwd(hit, par);//u to x
    else if(plane==7)hit_rot_stereo_bck(hit, par);//v to x
  }

  void MMLoadVariables::hit_rot_stereo_fwd(ROOT::Math::XYZVector& hit, std::shared_ptr<MMT_Parameters> par)const{
    double degree=M_PI/180.0*(par->stereo_degree);
    bool striphack = false;
    if(striphack) hit.SetY(hit.Y()*cos(degree));
    else{
      double xnew=hit.X()*std::cos(degree)+hit.Y()*std::sin(degree),ynew=-hit.X()*std::sin(degree)+hit.Y()*std::cos(degree);
      hit.SetX(xnew);hit.SetY(ynew);
    }
  }

  void MMLoadVariables::hit_rot_stereo_bck(ROOT::Math::XYZVector& hit, std::shared_ptr<MMT_Parameters> par)const{
    double degree=-M_PI/180.0*(par->stereo_degree);
    bool striphack = false;
    if(striphack) hit.SetY(hit.Y()*std::cos(degree));
    else{
      double xnew=hit.X()*std::cos(degree)+hit.Y()*std::sin(degree),ynew=-hit.X()*std::sin(degree)+hit.Y()*std::cos(degree);
      hit.SetX(xnew);hit.SetY(ynew);
    }
  }

  int 
  MMLoadVariables::Get_VMM_chip(int strip) const{  //Not Finished... Rough
    int strips_per_VMM = 64;
    return ceil(1.*strip/strips_per_VMM);
  }

  int 
  MMLoadVariables::strip_number(int station, int plane, int spos, std::shared_ptr<MMT_Parameters> par)const{
    if (station<=0||station>par->n_stations_eta) {
      int base_strip = 0;
      return base_strip;
    }
    if (plane<0||plane>(int)par->setup.size()) {
      int base_strip = 0;

      return base_strip;
    }
   
    int base_strip=spos;
    return base_strip;
  }
