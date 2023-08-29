/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  MuonCluster.cxx
  Muon cluster finding, creates an RoI cluster for track finding
*/

//CLASS HEADER
#include "MuonCluster.h"

//ATHENA GAUDI & STOREGATE
#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"

//LVL1 ROIS
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

//MUON CLUSTER
#include "MuonCluster.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "CxxUtils/fpcompare.h"
#include "CxxUtils/phihelper.h"

//C++ LIBS
#include <cmath>
#include <algorithm>
#include <sstream>
#include <stdexcept>

using namespace std;

MuonCluster::MuonCluster(const std::string& name, ISvcLocator* svc)
  : AthReentrantAlgorithm(name, svc){

}

MuonCluster::~MuonCluster() {
}

StatusCode MuonCluster::initialize(){

    ATH_MSG_INFO("Parameters for MuonCluster:" << name());
    ATH_MSG_INFO("DeltaR : " << m_DeltaR);
    ATH_MSG_INFO("MuonCluLabel : " << m_featureLabel);

    ATH_MSG_DEBUG("Retrieve service StoreGateSvc");
    ATH_MSG_DEBUG("Timers are initializated here");

    ATH_CHECK( m_roiCollectionKey.initialize() );
    ATH_CHECK( m_outputCompositesKey.initialize() );
    ATH_CHECK( m_outputRoiDescriptorKey.initialize() );

    ATH_CHECK( m_muRoiClusEtaKey.initialize() );
    ATH_CHECK( m_muRoiClusPhiKey.initialize() );
    ATH_CHECK( m_muRoiClusNRoiKey.initialize() );

    if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode MuonCluster::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("MuonCluster::execute()");

    std::vector<double> RoiEta_mon;
    std::vector<double> RoiPhi_mon;
    std::vector<double> RoiZed_mon;

    auto CluEta = Monitored::Scalar<double>("CluEta", -99.);
    auto CluPhi = Monitored::Scalar<double>("CluPhi", -99.);
    auto CluNum = Monitored::Scalar<int>("NumRoi", 0);

    auto nL1RoIs = Monitored::Scalar<int>("nL1RoIs",-99);
    auto nRoIinClusters = Monitored::Scalar<int>("nRoIinClusters",-99);
    auto nClusters = Monitored::Scalar<int>("nClusters",-99);

    auto dPhi_cluSeed = Monitored::Scalar<float>("dPhiCluSeed", -99.);
    auto dEta_cluSeed = Monitored::Scalar<float>("dEtaCluSeed", -99.);
    auto dR_cluSeed = Monitored::Scalar<float>("dRCluSeed", -99.);

    auto dPhi_RoivecSeed = Monitored::Scalar<double>("dPhiRoiVecSeed", -99.);
    auto dEta_RoivecSeed = Monitored::Scalar<double>("dEtaRoiVecSeed", -99.);
    auto dR_RoivecSeed   = Monitored::Scalar<double>("dRRoiVecSeed", -99.);

    auto mon_roiEta = Monitored::Collection("RoiEta", RoiEta_mon);
    auto mon_roiPhi = Monitored::Collection("RoiPhi", RoiPhi_mon);

    auto t1         = Monitored::Timer("TIME_TrigAlg");
    auto t2         = Monitored::Timer("TIME_TrigAlg_Clustering");

    auto mon = Monitored::Group(m_monTool, mon_roiEta, mon_roiPhi,
                                CluEta, CluPhi, CluNum,
                                nL1RoIs, nRoIinClusters, nClusters,
                                dPhi_cluSeed, dR_cluSeed, dEta_cluSeed,
                                t1, t2);

    //Setup the composite container we will put the MuonRoICluster in
    auto trigCompColl = SG::makeHandle(m_outputCompositesKey, ctx);
    ATH_CHECK(
      trigCompColl.record(std::make_unique<xAOD::TrigCompositeContainer>(),std::make_unique<xAOD::TrigCompositeAuxContainer>())
    );

    //Setup Decorator Handlers
    SG::WriteDecorHandle<xAOD::TrigCompositeContainer, float> muRoiClusEta(m_muRoiClusEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::TrigCompositeContainer, float> muRoiClusPhi(m_muRoiClusPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> muRoiClusNRoi(m_muRoiClusNRoiKey, ctx);

    //Setup the RoI Descriptor container we will put the MuonRoIDescriptors in
    SG::WriteHandle<TrigRoiDescriptorCollection> trigDescColl = TrigCompositeUtils::createAndStoreNoAux(m_outputRoiDescriptorKey, ctx);

    //check to make sure we have all the input trigger elements!

    int n_cl=0;
    lvl1_muclu_roi muonClu[20] = {{0,0,0}};
    lvl1_muclu_roi muonClu0[20] = {{0,0,0}};

    auto roiCollectionHdl = SG::makeHandle(m_roiCollectionKey, ctx);
    auto roiCollection = roiCollectionHdl.get();

    if (roiCollection->size() < 2){ //should be the L1 Muon RoI container
        ATH_MSG_WARNING("Input TrigRoiDescriptorCollection isn't the correct size! Potential L1 menu inconsistency. Got " << roiCollection->size() << " RoIs");
        return StatusCode::SUCCESS;
    }

    nL1RoIs = roiCollection->size();
    nRoIinClusters = 0;
    for (const TrigRoiDescriptor *roi : *roiCollection)
    {
        if(n_cl>= kMAX_ROI) {
            ATH_MSG_WARNING("Too many L1 Muon RoIs: bailing out");
            break;
        }

        RoiEta_mon.push_back(roi->eta());
        RoiPhi_mon.push_back(roi->phi());
        lvl1_muclu_roi my_lvl1_clu_roi;
        my_lvl1_clu_roi.eta = roi->eta();
        my_lvl1_clu_roi.phi = roi->phi();
        my_lvl1_clu_roi.nroi = 0;
        muonClu[n_cl] = my_lvl1_clu_roi;
        muonClu0[n_cl] = my_lvl1_clu_roi;
        ++n_cl;
    }

    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("Accumulated " << n_cl << " ROI Directions: ");
      ATH_MSG_DEBUG("  [eta,  phi]");
      for (int unsigned i=0;i<RoiEta_mon.size();i++) {
        ATH_MSG_DEBUG("  [" << RoiEta_mon.at(i) << "," << RoiPhi_mon.at(i) << "]");
      }
    }

    // start the clustering

    t2.start();
    int n_itr = 0;
    for(int i_cl=0; i_cl<n_cl; ++i_cl) { // loop on cluster
        ATH_MSG_DEBUG("Initial RoI Coordinates: eta = " << muonClu0[i_cl].eta << ", phi = " << muonClu0[i_cl].phi);
        bool improvement = true;
        n_itr = 0;
        while(improvement){
            ++n_itr;
            double eta_avg=0.0;
            double cosPhi_avg=0.0;
            double sinPhi_avg=0.0;
            int n_in_clu = 0;
            for (int j_cl=0; j_cl<n_cl; ++j_cl) { // loop on cluster
                float deltaR = DeltaR(muonClu0[j_cl],muonClu[i_cl]);
                if(deltaR<m_DeltaR){
                    ATH_MSG_DEBUG("  Adding Following RoI: eta = " << muonClu0[j_cl].eta << ", phi = " << muonClu0[j_cl].phi);
                    ++n_in_clu;
                    if(n_itr==1){
                        muonClu[i_cl].eta = muonClu[i_cl].eta + (muonClu0[j_cl].eta-muonClu[i_cl].eta)/n_in_clu;
                        muonClu[i_cl].phi = CxxUtils::wrapToPi(muonClu[i_cl].phi + CxxUtils::wrapToPi(muonClu0[j_cl].phi-muonClu[i_cl].phi)/n_in_clu);
                    } else{
                        //to recalculate the average with all RoIs within a dR = 0.4 cone of the seed position
                        eta_avg += muonClu0[j_cl].eta;
                        cosPhi_avg += cos(muonClu0[j_cl].phi);
                        sinPhi_avg += sin(muonClu0[j_cl].phi);
                    }
                } // End of if on deltaR<m_DeltaR

            } // End of for loop over j_cl

            if(n_itr > 1){
                //set cluster position as average position of RoIs
                //This, coupled with the improvement=true/false below, makes an assumption that
                //improvement = false means same # RoIs in cluster, but never less (code had this before, too)
                muonClu[i_cl].eta = eta_avg/n_in_clu;
                muonClu[i_cl].phi = atan2(sinPhi_avg,cosPhi_avg);
            }

            //find the number of ROIs in the new cluster
            //if the number is the same as before,
            Int_t n_in_clu2=0;
            for (int j_cl=0; j_cl<n_cl; ++j_cl) { // loop on cluster
                float deltaR = DeltaR(muonClu0[j_cl],muonClu[i_cl]);
                if(deltaR<m_DeltaR){
                    ++n_in_clu2;
                }
            }

            ATH_MSG_DEBUG("Finding the number of Muon RoIs in the new Cluster....   " << n_in_clu2);
            if(n_in_clu2>muonClu[i_cl].nroi){
                muonClu[i_cl].nroi=n_in_clu2;
                improvement = true;
            } else  improvement = false;
        }//end while
    }
    t2.stop(); // Stop Clustering Timer

    ATH_MSG_DEBUG("Total Improvement Iterations = " << n_itr);


    // find the cluster with max number of rois
    int ncl_max = 0;
    int sel_cl = -1;
    int nRoisInClu = 0;
    for(int i_cl=0; i_cl<n_cl; ++i_cl) { // loop on cluster
        nRoisInClu += muonClu[i_cl].nroi;
        if(muonClu[i_cl].nroi>ncl_max){
            CluEta = muonClu[i_cl].eta;
            CluPhi = muonClu[i_cl].phi;
            CluNum = muonClu[i_cl].nroi;
            ncl_max = muonClu[i_cl].nroi;
            sel_cl = i_cl;
            ATH_MSG_DEBUG("  -- ncl_max loop: i_cl = " << i_cl << " with ncl_max = " << ncl_max);
        }
    }
    nRoIinClusters = nRoisInClu;
    nClusters = n_cl;

    // Should never happen (we checked above that roiCollection->size() >=2),
    // but otherwise we get warnings from cppcheck.
    if (sel_cl < 0) {
      return StatusCode::FAILURE;
    }

    dPhi_cluSeed = CxxUtils::wrapToPi(muonClu0[sel_cl].phi)-CxxUtils::wrapToPi(muonClu[sel_cl].phi);
    dEta_cluSeed = muonClu0[sel_cl].eta-muonClu[sel_cl].eta;
    dR_cluSeed   = DeltaR(muonClu0[sel_cl],muonClu[sel_cl]);

    ATH_MSG_DEBUG("RoI Cluster Coordinates: eta = " << CluEta << ", phi = " << CluPhi << ", nRoI = " << CluNum);
    ATH_MSG_DEBUG("Found the Cluster with the maximum number of RoIs....   " << ncl_max);
    // finished now debugging
    ATH_MSG_DEBUG("Create an output Collection seeded by the input");

    xAOD::TrigComposite *compClu = new xAOD::TrigComposite();
    try{
        trigCompColl->push_back(compClu);  //add muon RoI clusters to the composite container
    }catch(const std::exception& e){
        ATH_MSG_ERROR("Write of MuonRoICluster TrigCompositeContainer object into trigCompColl failed!");
        ATH_MSG_ERROR("Error Message: " << e.what());
        return StatusCode::FAILURE;
    }


    compClu->setName("Cluster");
    muRoiClusEta(*compClu) = static_cast<float>(CluEta);
    muRoiClusPhi(*compClu) = static_cast<float>(CluPhi);
    muRoiClusNRoi(*compClu) = static_cast<int>(CluNum);


    //create a TrigRoiDescriptor to send to ID tracking, to seed track-finding
    //only need to do this if the MuonCluster will pass the hypo!
    if( (static_cast<int>(CluNum) >= 3 && std::abs(static_cast<double>(CluEta)) < 1.0) || \
        (static_cast<int>(CluNum) >= 4 && std::abs(static_cast<double>(CluEta)) >= 1.0 && std::abs(static_cast<double>(CluEta)) <= 2.5) )
    {
        double phiHalfWidth = 0.35;
        double phiMinus = CxxUtils::wrapToPi(static_cast<double>(CluPhi)-phiHalfWidth);
        double phiPlus  = CxxUtils::wrapToPi(static_cast<double>(CluPhi)+phiHalfWidth);
        TrigRoiDescriptor* roiClus =  new TrigRoiDescriptor(static_cast<double>(CluEta), static_cast<double>(CluEta)-0.4, static_cast<double>(CluEta)+0.4,static_cast<double>(CluPhi), phiMinus, phiPlus);
        trigDescColl->push_back(roiClus);
    }

    //----------------------------------------------------------------
    // REGTEST
    //----------------------------------------------------------------
    ATH_MSG_DEBUG(" REGTEST \t Cluster with : " << static_cast<int>(CluNum) << " LVL1-Roi");
    ATH_MSG_DEBUG(" REGTEST \t Cluster Eta " <<  static_cast<double>(CluEta) << "  Cluster Phi " << static_cast<double>(CluPhi));
    //----------------------------------------------------------------

    return StatusCode::SUCCESS;
}

float MuonCluster::DeltaR(lvl1_muclu_roi p_roi,lvl1_muclu_roi q_roi) const{

    float delPhi = CxxUtils::wrapToPi((p_roi).phi-(q_roi).phi);
    float delEta = (p_roi).eta-(q_roi).eta;

    return(sqrt(delPhi*delPhi+delEta*delEta));
}
