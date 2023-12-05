/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauInfo.h"

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<int, int>& L1Phase1ThrMap_eTAU, const std::map<int, int>& L1Phase1ThrMap_jTAU)
    : m_trigger{trigger}
{
    std::vector<std::string> sections;
    boost::split(sections, trigger, boost::is_any_of("_"));

    std::regex tau_rgx("^(\\d*)tau(\\d+)$");
    std::regex elec_rgx("^(\\d*)e(\\d+)$");
    std::regex muon_rgx("^(\\d*)mu(\\d+)$");
    std::regex gamma_rgx("^(\\d*)g(\\d+)$");
    std::regex jet_rgx("^(\\d*)j(\\d+)$");
    std::regex met_rgx("^xe(\\d+)$");
    std::regex l1_rgx("^L1.*$");
    std::regex l1_tau_rgx("(\\d*)(|e|j|c)TAU(\\d+)(|L|M|H|IM)");
    std::regex topo_rgx("^.*(invm|dR|deta|dphi)AB.*$");
    std::vector<std::regex*> all_regexes = {&tau_rgx, &elec_rgx, &muon_rgx, &gamma_rgx, &jet_rgx, &met_rgx, &l1_rgx};

    std::regex tau_type_rgx("^(ptonly|tracktwoMVA|tracktwoMVABDT|tracktwoLLP|trackLRT)$");

    std::smatch match;
    std::regex_token_iterator<std::string::iterator> rend;

    // Check each leg
    std::vector<std::string> leg;
    for(size_t i = 0; i < sections.size(); i++) {
        leg.push_back(sections[i]); // Attach to the current leg

        //Match the beginning of a new leg, or the end of the chain
        if(i == sections.size() - 1 || (std::any_of(all_regexes.begin(), all_regexes.end(), [&sections, i](const std::regex* rgx) { return std::regex_match(sections[i+1], *rgx); }))) {
            // Process the previous leg, which starts with the item, multiplicity, and threshold
            if(std::regex_match(leg[0], match, tau_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                auto itr = find_if(leg.begin(), leg.end(), [tau_type_rgx](const std::string& s) { return std::regex_match(s, tau_type_rgx); });
                std::string type = itr != leg.end() ? *itr : "";

                for(size_t j = 0; j < multiplicity; j++) {
                    m_HLTThr.push_back(threshold);
                    m_types.push_back(type);
                }
            } else if(std::regex_match(leg[0], match, elec_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTElecThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, muon_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTMuonThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, gamma_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTGammaThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, jet_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTJetThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, met_rgx)) {
                unsigned int threshold = std::stoi(match[2].str());
                m_HLTMETThr.push_back(threshold);
            } else if(std::regex_match(leg[0], l1_rgx)) { // Treat the L1 items as a leg
                for(size_t j = 0; j < leg.size(); j++) {
                    if(std::regex_match(leg[j], topo_rgx)) continue; // Remove HLT topo sections, not part of the L1 item
                    m_L1Items.push_back(j == 0 ? leg[j].substr(2, leg[j].size()) : leg[j]);
                }
            }

            // Start a new leg
            leg = {};
        }
    }

    if(m_L1Items.size()) {
        // Build the full L1 string
        m_L1Item = m_L1Items[0];
        for(size_t j = 1; j < m_L1Items.size(); j++) m_L1Item += "_" + m_L1Items[j];

        // Get all individual L1 TAU items
        std::regex_token_iterator<std::string::iterator> rgx_iter(m_L1Item.begin(), m_L1Item.end(), l1_tau_rgx);
        while(rgx_iter != rend) {
            std::string s = *rgx_iter;
            std::regex_match(s, match, l1_tau_rgx);
            size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
            std::string item_type = match[2].str(); // e, j, c, or ""
            unsigned int threshold = std::stoi(match[3].str());
            
            // Correct the Phase 1 thresholds:
            if(item_type == "e" || item_type == "c") {
                threshold = L1Phase1ThrMap_eTAU.at(threshold);
            } else if(item_type == "j") {
                threshold = L1Phase1ThrMap_jTAU.at(threshold);
            }

            for(size_t j = 0; j < multiplicity; j++) {
                m_tauL1Items.push_back(s);
                m_tauL1Thr.push_back(threshold);
                m_tauL1Type.push_back(item_type + "TAU");
            }
            rgx_iter++;
        }

        m_L1Item = "L1" + m_L1Items[0];
    }
}