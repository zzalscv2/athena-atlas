/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************
 **********************************************************************/
#include "TrigEgammaAnalysisTools/EfficiencyTool.h"

#include "xAODEventInfo/EventInfo.h"

#include "string"
#include <algorithm>
#include "boost/algorithm/string.hpp"
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
//**********************************************************************

EfficiencyTool::
EfficiencyTool( const std::string& myname )
: TrigEgammaAnalysisBaseTool(myname) {
    m_detailedHists=false;
}

//**********************************************************************
StatusCode EfficiencyTool::childInitialize(){


    return StatusCode::SUCCESS;
}

StatusCode EfficiencyTool::childBook()
{

    return StatusCode::SUCCESS;
}
StatusCode EfficiencyTool::childExecute()
{

    return StatusCode::SUCCESS;
}

StatusCode EfficiencyTool::childFinalize(){
  return StatusCode::SUCCESS;
}

void EfficiencyTool::analyseIsEMLH(const xAOD::Electron *eg, const std::string pid/*, const std::bitset<4> reco*/){

    const std::string name1="IsEmLHFail"+pid;
    const std::string pidword="isEMLH"+pid;
    if(eg){
        unsigned int isem = eg->selectionisEM(pidword);
        //float isolation=getIsolation_ptcone20(eg)/eg->pt();
        for(int ii=0;ii<11;ii++){
            if ( (isem>>ii) & 0x1 )
                hist1(name1)->Fill(ii+0.5);
        }
    }
}

/************************************************************************************************************************************************    
 * 32-bit isEM cut-based electron ID as defined here
 * https://svnweb.cern.ch/trac/atlasoff/browser/PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools/trunk/python/TrigEGammaPIDdefs.py
 ClusterEtaRange_Electron        =  0"""  @brief cluster eta range """
 ConversionMatch_Electron        =  1""" @brief matching to photon (not necessarily conversion--the name is historical) """
 ClusterHadronicLeakage_Electron =  2""" @brief cluster leakage into the hadronic calorimeter """
 ClusterMiddleEnergy_Electron    =  3""" @brief energy in 2nd sampling (e.g E277>0) """ 
 ClusterMiddleEratio37_Electron  =  4""" @brief energy ratio in 2nd sampling (e.g E237/E277) """
 ClusterMiddleEratio33_Electron  =  5""" @brief energy ratio in 2nd sampling (e.g E233/E237) """
 ClusterMiddleWidth_Electron     =  6""" @brief width in the second sampling (e.g Weta2) """
 ClusterBackEnergyFraction_Electron = 7""" @brief energy fraction in the third layer """ 
 ClusterStripsEratio_Electron    =  8""" @brief fraction of energy found in 1st sampling (NB: not used in fact for electrons)"""
 ClusterStripsDeltaEmax2_Electron = 9""" @brief energy of 2nd maximum in 1st sampling ~e2tsts1/(1000+const_lumi*et) """
 ClusterStripsDeltaE_Electron    = 10""" @brief difference between 2nd maximum and 1st minimum in strips (e2tsts1-emins1) """
 ClusterStripsWtot_Electron      = 11""" @brief shower width in 1st sampling """
 ClusterStripsFracm_Electron     = 12""" @brief shower shape in shower core 1st sampling """
 ClusterStripsWeta1c_Electron    = 13""" @brief shower width weighted by distance from the maximum one """
 Empty                           = 14 Empty bit
 ClusterStripsDEmaxs1_Electron   = 15""" @brief difference between max and 2nd max in strips """
 TrackBlayer_Electron            = 16""" @brief B layer hit """
 TrackPixel_Electron             = 17""" @brief number of Pixel hits """
 TrackSi_Electron                = 18""" @brief number of Pixel and SCT hits """
 TrackA0_Electron                = 19""" @brief distance of closet approach """
 TrackMatchEta_Electron          = 20""" @brief eta difference between cluster and extrapolated track in the 1st sampling """
 TrackMatchPhi_Electron          = 21""" @brief phi difference between cluster and extrapolated track in the 2nd sampling """
 TrackMatchEoverP_Electron       = 22""" @brief energy-momentum match """
 TrackTRTeProbabilityHT_Electron   = 23""" @brief Cut on the TRT eProbabilityHT_Electron """
 TrackTRThits_Electron           = 24""" @brief number of TRT hits """
 TrackTRTratio_Electron          = 25""" @brief ratio of high to all TRT hits for isolated electrons """
 TrackTRTratio90_Electron        = 26""" @brief ratio of high to all TRT hits for non-isolated electrons """   
 TrackA0Tight_Electron           = 27""" @brief distance of closet approach for tight selection """
 TrackMatchEtaTight_Electron     = 28""" @brief eta difference between cluster and extrapolated track in the 1st sampling for tight selection """
 Isolation_Electron              = 29""" @brief isolation """
 ClusterIsolation_Electron       = 30""" @brief calorimetric isolation """
 TrackIsolation_Electron         = 31""" @brief tracker isolation """
 **************************************************************************************************************************************************/
void EfficiencyTool::analyseIsEM(const xAOD::Electron *eg, const std::string pid,const std::bitset<4> reco){
    //{"ShowerShape","TrkClus","Track","TRT","Track+TRT","TrkClus+Trk+TRT","Isolation","IsEM + Iso","Track+Cluster","Track Only","Cluster","PhotonCluster","No Object","Some object","Unknown","Total"};
    const std::string name1="IsEmFail"+pid;
    const std::string name2="IneffIsEm"+pid;
    const std::string pidword="isEM"+pid;
    // reco; //Electron, cluster, track, photon
    if(eg){
        bool cluster=false;
        bool trkclus=false;
        bool trk=false;
        bool trt=false;
        bool failisem=false; 
        bool failiso=false;

        unsigned int isem = eg->selectionisEM(pidword);
        float isolation=getIsolation_ptcone20(eg)/eg->pt();
        if(isolation>0.1) failiso=true;
        for(int ii=0;ii<29;ii++){
            if ( (isem>>ii) & 0x1 ){
                failisem=true;
                hist1(name1)->Fill(ii+0.5);
                if(ii <= 15)
                    if ( (isem>>ii) & 0x1 ) cluster=true;
                if((ii >=16 && ii <= 19) || ii == 27)
                    if ( (isem>>ii) & 0x1 ) trk=true;
                if(ii == 20 || ii == 21 || ii == 22 || ii == 28)
                    if ( (isem>>ii) & 0x1 ) trkclus=true;
                if(ii >= 23 && ii <= 26)
                    if ( (isem>>ii) & 0x1 ) trt=true;
            }
        }
        if(failiso) hist1(name1)->Fill(31+0.5);

        bool onlyclus= !trkclus && !trk && !trt && cluster && !failiso;
        bool onlytrkclus= trkclus && !trk && !trt && !cluster && !failiso;
        bool onlytrk= !trkclus && trk && !trt && !cluster && !failiso;
        bool trktrt= !trkclus && trk && trt && !cluster && !failiso;
        bool onlytrt= !trkclus && !trk && trt && !cluster && !failiso;
        bool trkclustrktrt= trkclus && trk && trt && !cluster && !failiso;
        bool onlyiso= !trkclus && !trk && !trt && !cluster && failiso;

        if(onlyclus) hist1(name2)->Fill(0.5,1); // 0.5
        else if(onlytrkclus) hist1(name2)->Fill(1.5,1);
        else if(onlytrk) hist1(name2)->Fill(2.5,1);
        else if(onlytrt) hist1(name2)->Fill(3.5,1);
        else if(trktrt) hist1(name2)->Fill(4.5,1);
        else if(trkclustrktrt) hist1(name2)->Fill(5.5,1);
        else if(onlyiso) hist1(name2)->Fill(6.5,1);
        else if(failisem || failiso) hist1(name2)->Fill(7.5,1); //Something failed
        else hist1(name2)->Fill(13.5,1); // Unknown -- some combination failed
    }
    else if(!reco.test(0) && reco.test(1) && reco.test(2)) hist1(name2)->Fill(8.5,1); //Track and Cluster
    else if(!reco.test(0) && !reco.test(1) && reco.test(2)) hist1(name2)->Fill(9.5,1); // No Cluster w/ track
    else if(!reco.test(0) && reco.test(1) && !reco.test(2)) hist1(name2)->Fill(10.5,1); //Cluster Only
    else if(!reco.test(0) && !reco.test(1) && !reco.test(2) && !reco.test(3)) hist1(name2)->Fill(12.5,1); // No object in RoI
    else if(reco.test(1)||reco.test(2)||reco.test(3)) hist1(name2)->Fill(13.5,1);
    else hist1(name2)->Fill(14.5,1);
    
    if(reco.test(3) && reco.test(1)) hist1(name2)->Fill(11.5,1); //Has cluster and photon --> should be same as cluster only?
    hist1(name2)->Fill(15.5,1);
}

void EfficiencyTool::fillInefficiency(const std::string dir,const xAOD::Electron *selEF,const xAOD::Photon *selPh,const xAOD::CaloCluster *clus,const xAOD::TrackParticle *trk){
    cd(dir);
    ATH_MSG_DEBUG("REGTEST::Inefficiency");
    // Currently check the PID on the xAOD
    // xAOD PID for trigger needs validation!
    // First check for the nullptr
    // 36 bins for isEM
    // 11 bins for isEMLH

    // Fill Reco efficiency (find a nearby object)
    /*if(clus) hist1("eff_hltreco")->Fill("Cluster",1);
    else hist1("eff_hltreco")->Fill("Cluster",0);
    
    if(trk) hist1("eff_hltreco")->Fill("Track",1);
    else hist1("eff_hltreco")->Fill("Track",0);

    if(selPh) hist1("eff_hltreco")->Fill("Photon",1);
    else hist1("eff_hltreco")->Fill("Photon",0);

    if(selEF) hist1("eff_hltreco")->Fill("Electron",1);
    else hist1("eff_hltreco")->Fill("Electron",0);*/

    std::vector<std::string> pidnames {"Loose","Medium","Tight"};
   
    std::bitset<4> reco; //Electron, cluster, track, photon
    
    if(selEF!=nullptr) reco.set(0);
    if(clus!=nullptr) reco.set(1);
    if(trk!=nullptr) reco.set(2);
    if(selPh!=nullptr) reco.set(3);
    for(const auto name:pidnames){
        analyseIsEM(selEF,name,reco);
        analyseIsEMLH(selEF,name/*,reco*/);
    }
    
    float lastbinIsEM=hist1("IsEmFailTight")->GetNbinsX()-1;
    float lastbinIsEMLH=hist1("IsEmLHFailTight")->GetNbinsX()-1;
    for(const auto name:pidnames){
        hist1("IsEmFail"+name)->Fill(lastbinIsEM+0.5);
        hist1("IsEmLHFail"+name)->Fill(lastbinIsEMLH+0.5);
    }
  
    if(selPh==nullptr) ATH_MSG_DEBUG("fillIneffiency::No photon found!");
    if(selEF!=nullptr){
        ATH_MSG_DEBUG("REGTEST::Inefficiency Electron pt, eta, phi "<< selEF->pt() << " " << selEF->eta() << " " << selEF->phi());
    }
    else {
        if(trk==nullptr && clus!=nullptr){
            ATH_MSG_DEBUG("fillInefficiency::No Electron, nearby cluster"); 
            // No electron candidate but we have photon
            // Do something for hasCluster
            for(const auto name:pidnames){
                hist1("IsEmFail"+name)->Fill( (lastbinIsEM-3) + 0.5);
                hist1("IsEmLHFail"+name)->Fill( (lastbinIsEMLH-3) + 0.5);
            }
        }
        if(clus==nullptr && trk!=nullptr){
            ATH_MSG_DEBUG("fillInefficiency::No Electron, no cluster"); 
            // No electron candidate but we have photon
            // Do something for hasCluster
            for(const auto name:pidnames){
                hist1("IsEmFail"+name)->Fill( (lastbinIsEM-2) + 0.5);
                hist1("IsEmLHFail"+name)->Fill( (lastbinIsEMLH-2) + 0.5);
            }
        }
        if(clus==nullptr && trk==nullptr){
            ATH_MSG_DEBUG("fillInefficiency::No Electron, no cluster"); 
            // Unknown failure
            for(const auto name:pidnames){
                hist1("IsEmFail"+name)->Fill( (lastbinIsEM-1) + 0.5);
                hist1("IsEmLHFail"+name)->Fill( (lastbinIsEMLH-1) + 0.5);
            }
        }
    }
    if(clus!=nullptr) ATH_MSG_DEBUG("REGTEST::Inefficiency Cluster " << clus->et() << " " << clus->eta() << " " << clus->phi());
    if(trk!=nullptr) ATH_MSG_DEBUG("REGTEST::Inefficiency Track " << trk->pt() << " " << trk->eta() << " " << trk->phi());
    else ATH_MSG_DEBUG("REGTEST::Inefficiency No track");
}

void EfficiencyTool::inefficiency(const std::string basePath,const float etthr, 
        std::pair< const xAOD::Egamma*,const HLT::TriggerElement*> pairObj){
    ATH_MSG_DEBUG("INEFF::Start Inefficiency Analysis ======================= " << basePath);
    cd(basePath);
    // Inefficiency analysis
    float et=0.;
    const xAOD::Egamma* eg =pairObj.first;
    const HLT::TriggerElement *feat = pairObj.second; 
    if(xAOD::EgammaHelpers::isElectron(pairObj.first)){
        ATH_MSG_DEBUG("Offline Electron");
        const xAOD::Electron* el =static_cast<const xAOD::Electron*> (eg);
        et = getEt(el)/1e3;
    }
    else  et=eg->caloCluster()->et()/1e3;

    float eta = eg->eta();
    float phi = eg->phi();
    ATH_MSG_DEBUG("INEFF::Offline et, eta, phi " << et << " " << eta << " " << phi);
    
    const xAOD::Electron* selEF = nullptr;
    const xAOD::Photon* selPh = nullptr;
    const xAOD::CaloCluster* selClus = nullptr;
    const xAOD::TrackParticle* selTrk = nullptr;

    float dRmax=0.07;

    std::vector<std::string> pidnames {"Loose","Medium","Tight"};
    if(getAccept().getCutResult("HLT")){
        for(const auto name:pidnames){
            for(int ibin=0; ibin<hist1("IneffIsEm"+name)->GetNbinsX();ibin++)
                hist1("IneffIsEm"+name)->Fill(ibin+0.5,0);
        }
    }
    // Ensure L1 passes
    // And offline passes et cut
    if(getAccept().getCutResult("L1Calo") && et > etthr) {
        ATH_MSG_DEBUG("INEFF::Passed L1 and offline et");
        hist1("eff_triggerstep")->Fill("L2Calo",getAccept().getCutResult("L2Calo"));
        hist1("eff_triggerstep")->Fill("L2",getAccept().getCutResult("L2"));
        hist1("eff_triggerstep")->Fill("EFCalo",getAccept().getCutResult("EFCalo"));
        hist1("eff_triggerstep")->Fill("EFTrack",getAccept().getCutResult("EFTrack"));
        hist1("eff_triggerstep")->Fill("HLT",getAccept().getCutResult("HLT"));

        // Fill efficiency plot for HLT trigger steps
        if(!getAccept().getCutResult("HLT") && getAccept().getCutResult("EFCalo")){
            ATH_MSG_DEBUG("INEFF::Retrieve features for EF containers only ");
            ATH_MSG_DEBUG("INEFF::Retrieve EF Electron");
            const auto* EFEl = getFeature<xAOD::ElectronContainer>(feat);
            ATH_MSG_DEBUG("INEFF::Retrieve EF Photons");
            const auto* EFPh = getFeature<xAOD::PhotonContainer>(feat);
            ATH_MSG_DEBUG("INEFF::Retrieve EF Cluster");
            const auto* EFClus = getFeature<xAOD::CaloClusterContainer>(feat);
            //ATH_MSG_DEBUG("INEFF::Retrieve EF Trk");
            //const auto* L2Trk = getFeature<xAOD::TrackParticleContainer>(feat);
            //const auto* L2Trk = getFeature<xAOD::TrackParticleContainer>(feat,"InDetTrigTrackingxAODCnv_Electron_FTF");
            //const auto* EFIDTrk = getFeature<xAOD::TrackParticleContainer>(feat,"InDetTrigTrackingxAODCnv_Electron_EFID");
            const auto* EFTrkIDTrig = getFeature<xAOD::TrackParticleContainer>(feat,"InDetTrigTrackingxAODCnv_Electron_IDTrig");
            dRmax=0.15;
            // Find closest object for all objects
            selEF=closestObject<xAOD::Electron,xAOD::ElectronContainer>(pairObj,dRmax,false);
            selPh=closestObject<xAOD::Photon,xAOD::PhotonContainer>(pairObj,dRmax,false);
            selClus=closestObject<xAOD::CaloCluster,xAOD::CaloClusterContainer>(pairObj,dRmax,false);
            selTrk=closestObject<xAOD::TrackParticle,xAOD::TrackParticleContainer>(pairObj,dRmax,false,"InDetTrigTrackingxAODCnv_Electron_IDTrig");
            fillInefficiency(basePath,selEF,selPh,selClus,selTrk);
            if(EFClus==nullptr){
                hist1("eff_hltreco")->Fill("ClusterCont",0);
                hist1("eff_hltreco")->Fill("Cluster",0);
                hist1("eff_hltreco")->Fill("ClusterMatch",0);
            }
            else{
                hist1("eff_hltreco")->Fill("ClusterCont",1);
                if(EFClus->size() > 0){
                    hist1("eff_hltreco")->Fill("Cluster",1);
                    if(selClus) hist1("eff_hltreco")->Fill("ClusterMatch",1);
                    else hist1("eff_hltreco")->Fill("ClusterMatch",0);
                }
                else{
                    hist1("eff_hltreco")->Fill("Cluster",0);
                    hist1("eff_hltreco")->Fill("ClusterMatch",0);
                }
            }

            if(EFTrkIDTrig==nullptr){
                hist1("eff_hltreco")->Fill("TrackCont",0);
                hist1("eff_hltreco")->Fill("Track",0);
                hist1("eff_hltreco")->Fill("TrackMatch",0);
            }
            else{
                hist1("eff_hltreco")->Fill("TrackCont",1);
                if(EFTrkIDTrig->size() > 0){
                    hist1("eff_hltreco")->Fill("Track",1);
                    if(selTrk) hist1("eff_hltreco")->Fill("TrackMatch",1);
                    else hist1("eff_hltreco")->Fill("TrackMatch",0);
                }
                else{
                    hist1("eff_hltreco")->Fill("Track",0);
                    hist1("eff_hltreco")->Fill("TrackMatch",0);
                }
            }

            if(EFPh==nullptr){
                hist1("eff_hltreco")->Fill("PhotonCont",0);
                hist1("eff_hltreco")->Fill("Photon",0);
                hist1("eff_hltreco")->Fill("PhotonMatch",0);
            }
            else {
                hist1("eff_hltreco")->Fill("PhotonCont",1);
                if(EFPh->size() > 0){
                    hist1("eff_hltreco")->Fill("Photon",1);
                    if(selPh) hist1("eff_hltreco")->Fill("PhotonMatch",1);
                    else  hist1("eff_hltreco")->Fill("PhotonMatch",0);
                }
                else{ 
                    hist1("eff_hltreco")->Fill("Photon",0);
                    hist1("eff_hltreco")->Fill("PhotonMatch",0);
                }
            }

            if(EFEl==nullptr){
                hist1("eff_hltreco")->Fill("ElectronCont",0);
                hist1("eff_hltreco")->Fill("Electron",0);
                hist1("eff_hltreco")->Fill("ElectronMatch",0);
            }
            else {
                hist1("eff_hltreco")->Fill("ElectronCont",1);
                if(EFEl->size() > 0){
                    hist1("eff_hltreco")->Fill("Electron",1);
                    if(selEF) hist1("eff_hltreco")->Fill("ElectronMatch",1);
                    else hist1("eff_hltreco")->Fill("ElectronMatch",0);
                }
                else{
                    hist1("eff_hltreco")->Fill("Electron",0);
                    hist1("eff_hltreco")->Fill("ElectronMatch",0);
                }
            }

            
        }
    }
    ATH_MSG_DEBUG("End Inefficiency Analysis ======================= " << basePath);
}
void EfficiencyTool::fillEfficiency(const std::string dir,bool isPassed,const float etthr, const std::string pidword, const xAOD::Egamma *eg, bool fill2D){

    cd(dir);
    float et=0.;
    bool pid=true;
    ATH_MSG_DEBUG("Default pid " << pid << " te " << isPassed);
    if(xAOD::EgammaHelpers::isElectron(eg)){
        ATH_MSG_DEBUG("Offline Electron with pidword " << pidword);
        const xAOD::Electron* el =static_cast<const xAOD::Electron*> (eg);
        pid=el->auxdecor<bool>(pidword);
        ATH_MSG_DEBUG("Electron pid " << pid);
        et = getEt(el)/1e3;
    }
    else  et=eg->caloCluster()->et()/1e3;

    float eta = eg->caloCluster()->etaBE(2);
    float phi = eg->phi();
    float pt = eg->pt();
    float avgmu=getAvgMu();
    ATH_MSG_DEBUG("Mu " << avgmu << " " << getAvgOnlineMu() << " "  << getAvgOfflineMu()); 
    ATH_MSG_DEBUG("PID decision efficiency " << eg->auxdecor<bool>(pidword));
    if(pid){
        hist1("et")->Fill(et);
        hist1("pt")->Fill(pt);
        hist1("highet")->Fill(et);
        if(et > etthr+1.0){
            hist1("eta")->Fill(eta);
            hist1("phi")->Fill(phi);
            hist1("mu")->Fill(avgmu);
            if(fill2D){
                if(m_detailedHists) hist2("et_eta")->Fill(et,eta);
                hist2("coarse_et_eta")->Fill(et,eta);
            }
        }
        if(isPassed) {
            hist1("match_et")->Fill(et);
            hist1("match_pt")->Fill(pt);
            hist1("match_highet")->Fill(et);
            if(et > etthr+1.0){
                hist1("match_eta")->Fill(eta);
                hist1("match_phi")->Fill(phi);
                hist1("match_mu")->Fill(avgmu);
                if(fill2D){
                    if(m_detailedHists) hist2("match_et_eta")->Fill(et,eta);
                    hist2("match_coarse_et_eta")->Fill(et,eta);
                }

            }
            hist1("eff_et")->Fill(et,1);
            hist1("eff_pt")->Fill(pt,1);
            hist1("eff_highet")->Fill(et,1);
            if(et > etthr+1.0){
                hist1("eff_eta")->Fill(eta,1);
                hist1("eff_phi")->Fill(phi,1);
                hist1("eff_mu")->Fill(avgmu,1);
                if(fill2D){
                    if(m_detailedHists) hist2("eff_et_eta")->Fill(et,eta,1);
                    hist2("eff_coarse_et_eta")->Fill(et,eta,1);
                }
            }
        } // Passes Trigger selection
        else {
            hist1("eff_et")->Fill(et,0);
            hist1("eff_pt")->Fill(pt,0);
            hist1("eff_highet")->Fill(et,0);
            if(et > etthr+1.0){
                hist1("eff_eta")->Fill(eta,0);
                hist1("eff_phi")->Fill(phi,0);
                hist1("eff_mu")->Fill(avgmu,0);
                if(fill2D){
                    if(m_detailedHists) hist2("eff_et_eta")->Fill(et,eta,0);
                    hist2("eff_coarse_et_eta")->Fill(et,eta,0);
                }
            }
        } // Fails Trigger selection
    } // Passes offline pid, fill histograms
}

StatusCode EfficiencyTool::toolExecute(const std::string basePath,const TrigInfo info,
        std::vector<std::pair< const xAOD::Egamma*,const HLT::TriggerElement*>> pairObjs){
    if(m_tp) return StatusCode::SUCCESS;
    
    // Removing Prescale check, in reprocessing L1AfterPrescale always false
    //
    /*if(isPrescaled(info.trigName)){
        ATH_MSG_DEBUG(info.trigName << " prescaled, skipping");
        return StatusCode::SUCCESS; 
    }*/

    const std::string dir = basePath+"/"+info.trigName;
    const float etthr = info.trigThrHLT;
    const std::string pidword = info.trigPidDecorator;
    
    ATH_MSG_DEBUG("Efficiency for " << info.trigName << " " <<pidword);
    for(const auto pairObj : pairObjs){
        // Final cuts done here
        if(pairObj.first->type()==xAOD::Type::Electron){
            const xAOD::Electron* el = static_cast<const xAOD::Electron *> (pairObj.first);
            float et = getEt(el)/1e3;
            if(et < info.trigThrHLT-5.0) continue; // return StatusCode::SUCCESS;

        } // Offline object Electron
        else if(pairObj.first->type()==xAOD::Type::Photon){
            float et = getCluster_et(pairObj.first)/1e3;
            if(et < info.trigThrHLT-5.0) continue; // return StatusCode::SUCCESS;
        } // Offline photon
        ATH_MSG_DEBUG("Fill probe histograms");
        setAccept(pairObj.second,info); //Sets the trigger accepts
        if (pairObj.second!=nullptr) {
            // Inefficiency analysis
            if(!info.trigL1){
                if(pairObj.first->type()==xAOD::Type::Electron){
                    if(!pairObj.first->auxdecor<bool>(info.trigPidDecorator)) continue;
                    inefficiency(dir+"/Efficiency/HLT",etthr,pairObj);
                }
            }
        } // Features
        if(info.trigL1)
            this->fillEfficiency(dir+"/Efficiency/L1Calo",getAccept().getCutResult("L1Calo"),etthr,pidword,pairObj.first);
        else {
            this->fillEfficiency(dir+"/Efficiency/HLT",getAccept().getCutResult("HLT"),etthr,pidword,pairObj.first);
            this->fillEfficiency(dir+"/Efficiency/L2Calo",getAccept().getCutResult("L2Calo"),etthr,pidword,pairObj.first,m_detailedHists); 
            this->fillEfficiency(dir+"/Efficiency/L2",getAccept().getCutResult("L2"),etthr,pidword,pairObj.first,m_detailedHists);
            this->fillEfficiency(dir+"/Efficiency/EFCalo",getAccept().getCutResult("EFCalo"),etthr,pidword,pairObj.first,m_detailedHists);
            if(m_detailedHists){
                this->fillEfficiency(dir+"/Efficiency/L1Calo",getAccept().getCutResult("L1Calo"),etthr,pidword,pairObj.first);
                for(const auto pid : m_isemname) {
                    this->fillEfficiency(dir+"/Efficiency/HLT/"+pid,getAccept().getCutResult("HLT"),etthr,"is"+pid,pairObj.first);
                    if( pairObj.first->auxdecor<bool>("Isolated") ) fillEfficiency(dir+"/Efficiency/HLT/"+pid+"Iso",getAccept().getCutResult("HLT"),etthr,"is"+pid,pairObj.first);
                }
                for(const auto pid : m_lhname) {
                    this->fillEfficiency(dir+"/Efficiency/HLT/"+pid,getAccept().getCutResult("HLT"),etthr,"is"+pid,pairObj.first);
                    if( pairObj.first->auxdecor<bool>("Isolated") ) fillEfficiency(dir+"/Efficiency/HLT/"+pid+"Iso",getAccept().getCutResult("HLT"),etthr,"is"+pid,pairObj.first);
                }
            }
            ATH_MSG_DEBUG("Complete efficiency");
        }
    }
    return StatusCode::SUCCESS;
}
