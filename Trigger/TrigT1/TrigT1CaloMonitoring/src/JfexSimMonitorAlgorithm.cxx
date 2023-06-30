/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "JfexSimMonitorAlgorithm.h"

JfexSimMonitorAlgorithm::JfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator) {}

StatusCode JfexSimMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("Initializing JfexSimMonitorAlgorithm algorithm with name: "<< name());
    ATH_MSG_DEBUG("Package Name "<< m_Grouphist);
    ATH_CHECK(m_monTool.retrieve());
    ATH_MSG_DEBUG("Logging errors to " << m_monTool.name() << " monitoring tool");


    ATH_MSG_DEBUG("m_data_key_jJ "   << m_data_key_jJ   );
    ATH_MSG_DEBUG("m_data_key_jLJ "  << m_data_key_jLJ  );
    ATH_MSG_DEBUG("m_data_key_jTau " << m_data_key_jTau );
    ATH_MSG_DEBUG("m_data_key_jEM "  << m_data_key_jEM  );
    ATH_MSG_DEBUG("m_data_key_jXE "  << m_data_key_jXE  );
    ATH_MSG_DEBUG("m_data_key_jTE "  << m_data_key_jTE  );    

    ATH_MSG_DEBUG("m_simu_key_jJ "   << m_simu_key_jJ   );
    ATH_MSG_DEBUG("m_simu_key_jLJ "  << m_simu_key_jLJ  );
    ATH_MSG_DEBUG("m_simu_key_jTau " << m_simu_key_jTau );
    ATH_MSG_DEBUG("m_simu_key_jEM "  << m_simu_key_jEM  );
    ATH_MSG_DEBUG("m_simu_key_jXE "  << m_simu_key_jXE  );
    ATH_MSG_DEBUG("m_simu_key_jTE "  << m_simu_key_jTE  );    
    

    // we initialise all the containers
    ATH_CHECK( m_data_key_jJ.initialize()   );
    ATH_CHECK( m_data_key_jLJ.initialize()  );
    ATH_CHECK( m_data_key_jTau.initialize() );
    ATH_CHECK( m_data_key_jEM.initialize()  );
    ATH_CHECK( m_data_key_jXE.initialize()  );
    ATH_CHECK( m_data_key_jTE.initialize()  );

    ATH_CHECK( m_simu_key_jJ.initialize()   );
    ATH_CHECK( m_simu_key_jLJ.initialize()  );
    ATH_CHECK( m_simu_key_jTau.initialize() );
    ATH_CHECK( m_simu_key_jEM.initialize()  );
    ATH_CHECK( m_simu_key_jXE.initialize()  );
    ATH_CHECK( m_simu_key_jTE.initialize()  );
    
    ATH_CHECK( m_jFexTowerKey.initialize()  );
    

    // TOBs may come from trigger bytestream - renounce from scheduler
    renounce(m_data_key_jJ);
    renounce(m_data_key_jLJ);
    renounce(m_data_key_jTau);
    renounce(m_data_key_jEM);
    renounce(m_data_key_jXE);
    renounce(m_data_key_jTE);  
    

    return AthMonitorAlgorithm::initialize();
}

StatusCode JfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexMonitorAlgorithm::fillHistograms");
    
    SG::ReadHandle<xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexTowerKey, ctx};
    if(!jFexTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container found in storegate  "<< m_jFexTowerKey);
        return StatusCode::SUCCESS;
    }
    
    std::string inputTower = jFexTowerContainer->empty() ? "EmulatedTowers" : "DataTowers";
    
    /*************************/
    //        SR jets!
    /*************************/
    std::vector<std::array<float,5> > jJ_data_UNmatched;
    std::vector<std::array<float,5> > jJ_data_matched = tobMatching(m_data_key_jJ, m_simu_key_jJ, ctx, jJ_data_UNmatched);

    std::vector<std::array<float,5> > jJ_simu_UNmatched;
    std::vector<std::array<float,5> > jJ_simu_matched = tobMatching(m_simu_key_jJ, m_data_key_jJ, ctx, jJ_simu_UNmatched);

    if(jJ_simu_matched.size() != jJ_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHist("SimEqData" ,"jJ" ,inputTower ,0 ,jJ_data_matched  );
    fillHist("DataNoSim" ,"jJ" ,inputTower ,1 ,jJ_data_UNmatched);
    fillHist("SimNoData" ,"jJ" ,inputTower ,1 ,jJ_simu_UNmatched);
    
    
    /*************************/
    //        LR jets!
    /*************************/
    std::vector<std::array<float,5> > jLJ_data_UNmatched;
    std::vector<std::array<float,5> > jLJ_data_matched = tobMatching(m_data_key_jLJ, m_simu_key_jLJ, ctx, jLJ_data_UNmatched);

    std::vector<std::array<float,5> > jLJ_simu_UNmatched;
    std::vector<std::array<float,5> > jLJ_simu_matched = tobMatching(m_simu_key_jLJ, m_data_key_jLJ, ctx, jLJ_simu_UNmatched);

    if(jLJ_simu_matched.size() != jLJ_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHist("SimEqData" ,"jLJ" ,inputTower ,0 ,jLJ_data_matched );
    fillHist("DataNoSim" ,"jLJ" ,inputTower ,1 ,jLJ_data_UNmatched );
    fillHist("SimNoData" ,"jLJ" ,inputTower ,1 ,jLJ_simu_UNmatched );    
    
    
    /*************************/
    //        Taus!
    /*************************/
    std::vector<std::array<float,5> > jTau_data_UNmatched;
    std::vector<std::array<float,5> > jTau_data_matched = tobMatching(m_data_key_jTau, m_simu_key_jTau, ctx, jTau_data_UNmatched);

    std::vector<std::array<float,5> > jTau_simu_UNmatched;
    std::vector<std::array<float,5> > jTau_simu_matched = tobMatching(m_simu_key_jTau, m_data_key_jTau, ctx, jTau_simu_UNmatched);

    if(jTau_simu_matched.size() != jTau_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHist("SimEqData" ,"jTau" ,inputTower ,0 ,jTau_data_matched );
    fillHist("DataNoSim" ,"jTau" ,inputTower ,1 ,jTau_data_UNmatched );
    fillHist("SimNoData" ,"jTau" ,inputTower ,1 ,jTau_simu_UNmatched );    
    
    
    /*************************/
    //        EM!
    /*************************/
    std::vector<std::array<float,5> > jEM_data_UNmatched;
    std::vector<std::array<float,5> > jEM_data_matched = tobMatching(m_data_key_jEM, m_simu_key_jEM, ctx, jEM_data_UNmatched);

    std::vector<std::array<float,5> > jEM_simu_UNmatched;
    std::vector<std::array<float,5> > jEM_simu_matched = tobMatching(m_simu_key_jEM, m_data_key_jEM, ctx, jEM_simu_UNmatched);

    if(jEM_simu_matched.size() != jEM_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHist("SimEqData" ,"jEM" ,inputTower ,0 ,jEM_data_matched );
    fillHist("DataNoSim" ,"jEM" ,inputTower ,1 ,jEM_data_UNmatched );
    fillHist("SimNoData" ,"jEM" ,inputTower ,1 ,jEM_simu_UNmatched );    
    
    
    /*************************/
    //        jXE
    /*************************/
    std::vector<std::array<int,3> > jXE_data_UNmatched;
    std::vector<std::array<int,3> > jXE_data_matched = tobMatchingGlobals(m_data_key_jXE, m_simu_key_jXE, ctx, jXE_data_UNmatched);
    std::vector<std::array<int,3> > jXE_simu_UNmatched;
    std::vector<std::array<int,3> > jXE_simu_matched = tobMatchingGlobals(m_data_key_jXE, m_simu_key_jXE, ctx, jXE_simu_UNmatched);

    if(jXE_simu_matched.size() != jXE_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHistGlobals("SimEqData"   ,"jXE" ,inputTower ,0 ,jXE_data_matched );
    fillHistGlobals("SimDiffData" ,"jXE" ,inputTower ,1 ,jXE_data_UNmatched );

    
    /*************************/
    //        jTE
    /*************************/
    std::vector<std::array<int,3> > jTE_data_UNmatched;
    std::vector<std::array<int,3> > jTE_data_matched = tobMatchingGlobals(m_data_key_jTE, m_simu_key_jTE, ctx, jTE_data_UNmatched);
    std::vector<std::array<int,3> > jTE_simu_UNmatched;
    std::vector<std::array<int,3> > jTE_simu_matched = tobMatchingGlobals(m_data_key_jTE, m_simu_key_jTE, ctx, jTE_simu_UNmatched);

    if(jTE_simu_matched.size() != jTE_data_matched.size()) {
        ATH_MSG_WARNING(" Simulation TOB and Data TOB matching vector do not have the same size");
    }

    fillHistGlobals("SimEqData"   ,"jTE" ,inputTower ,0 ,jTE_data_matched );
    fillHistGlobals("SimDiffData" ,"jTE" ,inputTower ,1 ,jTE_data_UNmatched );
    
    return StatusCode::SUCCESS;
}


template <typename T> std::vector<std::array<float,5> >  JfexSimMonitorAlgorithm::tobMatching(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<float,5> > & unmatched ) const {

    bool isInValid = false;
    SG::ReadHandle<T> tobs1Cont{tobs1Key, ctx};
    if(!tobs1Cont.isValid()) {
        ATH_MSG_WARNING("No jFex container found in storegate with key "<< tobs1Key<< ". Skipping, is it Run2 data?");
        isInValid = true;
    }
    SG::ReadHandle<T> tobs2Cont{tobs2Key, ctx};
    if(!tobs2Cont.isValid()) {
        ATH_MSG_WARNING("No jFex container found in storegate with key "<< tobs2Key<< ". Skipping, is it Run2 data?");
        isInValid = true;
    }

    std::vector< std::array<float,5> > matched;
    
    if(isInValid) return matched;

    for(const auto tob1 : *tobs1Cont) {

        bool isMatched = false;
        auto word1 = tob1->tobWord();
        auto jfex1 = tob1->jFexNumber();
        auto fpga1 = tob1->fpgaNumber();

        for (auto tob2 : *tobs2Cont) {
            if(word1 == tob2->tobWord() && jfex1 == tob2->jFexNumber() && fpga1 == tob2->fpgaNumber() ) {
                std::array<float,5> tmp = {(float) word1, (float) jfex1, (float) fpga1, tob1->eta(), tob1->phi()};
                matched.push_back(tmp);
                isMatched = true;
                break;
            }
        }

        if(!isMatched) {
            std::array<float,5> tmp = {(float) word1, (float) jfex1, (float) fpga1, tob1->eta(), tob1->phi()};
            unmatched.push_back(tmp);
        }
    }

    return matched;
}


void JfexSimMonitorAlgorithm::fillHist(const std::string & pkg, const std::string & item, const std::string & input, const bool fillError, std::vector< std::array<float,5> > & elem ) const {

    auto jFexModule  = Monitored::Scalar<int>  ("jfex",  0);
    auto jFexFPGA    = Monitored::Scalar<int>  ("fpga",  0);
    auto jFexeta     = Monitored::Scalar<float>("eta" ,0.0);
    auto jFexphi     = Monitored::Scalar<float>("phi" ,0.0);
    
    auto jFexInput   = Monitored::Scalar< std::string >  ("input",input);
    auto jFexItem    = Monitored::Scalar< std::string >  ("item" , item);
    
    std::string package = m_Grouphist+"_"+pkg+"_"+item+"_"+input;
    
    for(const auto tob: elem) {

        jFexModule = std::get<1>(tob);
        jFexFPGA   = std::get<2>(tob);
        jFexeta    = std::get<3>(tob);
        jFexphi    = std::get<4>(tob);
        
        fill(package,jFexModule,jFexFPGA,jFexeta,jFexphi);
        
        if(fillError){
            fill(m_Grouphist,jFexInput,jFexItem);
            genError("Sim_"+input, "TOB");
        }
    }
}

template <typename T> std::vector<std::array<int,3> >  JfexSimMonitorAlgorithm::tobMatchingGlobals(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<int,3> > & unmatched ) const {

    SG::ReadHandle<T> tobs1Cont{tobs1Key, ctx};
    if(!tobs1Cont.isValid()) {
        ATH_MSG_ERROR("No jFex container found in storegate tob1 with key "<< tobs1Key);
    }
    SG::ReadHandle<T> tobs2Cont{tobs2Key, ctx};
    if(!tobs2Cont.isValid()) {
        ATH_MSG_ERROR("No jFex container found in storegate tob2 with key "<< tobs2Key);
    }

    std::vector< std::array<int,3> > matched;

    for(const auto tob1 : *tobs1Cont) {

        bool isMatched = false;
        auto word1 = tob1->tobWord();
        auto jfex1 = tob1->jFexNumber();
        auto fpga1 = tob1->fpgaNumber();

        for (auto tob2 : *tobs2Cont) {
            if((word1 == tob2->tobWord() && jfex1 == tob2->jFexNumber() && fpga1 == tob2->fpgaNumber()) or (word1 == 0 ) ) {
                std::array<int,3> tmp = { (int) word1, jfex1, fpga1};
                matched.push_back(tmp);
                isMatched = true;
                break;
            }
        }

        if(!isMatched) {
            std::array<int,3> tmp = {(int) word1, jfex1, fpga1};
            unmatched.push_back(tmp);
        }
    }

    return matched;
}


void JfexSimMonitorAlgorithm::fillHistGlobals(const std::string & pkg, const std::string & item, const std::string & input, const bool fillError, std::vector< std::array<int,3> > & elem ) const {

    auto jFexModule  = Monitored::Scalar<int>  ("jfex",  0);
    auto jFexFPGA    = Monitored::Scalar< std::string >  ("fpga",  "");
    
    auto jFexInput   = Monitored::Scalar< std::string >  ("input",input);
    auto jFexItem    = Monitored::Scalar< std::string >  ("item" , item);
    
    std::string fpga_name[4] = {"U1", "U2", "U3", "U4"};
    
    std::string package = m_Grouphist+"_"+pkg+"_"+item+"_"+input;
    
    for(const auto tob: elem) {

        jFexModule = std::get<1>(tob);
        jFexFPGA   = fpga_name[std::get<2>(tob)];
        
        fill(package,jFexModule,jFexFPGA);
        
        if(fillError){
            fill(m_Grouphist,jFexInput,jFexItem);
            genError("Sim_"+input, "global TOB");
        }
    }
}


void  JfexSimMonitorAlgorithm::genError(const std::string& location, const std::string& title) const {
    Monitored::Group(m_monTool,
                     Monitored::Scalar("genLocation",location.empty() ? std::string("UNKNOWN") : location),
                     Monitored::Scalar("genType",title.empty()    ? std::string("UNKNOWN") : title)
                    );
}
