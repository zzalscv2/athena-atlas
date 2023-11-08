/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/RpcCondDbAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"

// constructor
RpcCondDbAlg::RpcCondDbAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {
    declareProperty("isOnline", m_isOnline);
    declareProperty("isData", m_isData);
    declareProperty("isRun1", m_isRun1);
    declareProperty("PanelEfficiency", m_panelEfficiency = 0.50);
}

// Initialize
StatusCode RpcCondDbAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKey_folder_da_deadPanels.initialize(!m_readKey_folder_da_deadPanels.empty() && m_isData));
    ATH_CHECK(m_readKey_folder_da_offPanels.initialize(!m_readKey_folder_da_offPanels.empty() && m_isData));
    ATH_CHECK(m_readKey_folder_mc_deadElements.initialize(!m_readKey_folder_mc_deadElements.empty()));

    return StatusCode::SUCCESS;
}

// execute
StatusCode RpcCondDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    if (m_isOnline) {
        ATH_MSG_DEBUG("IsOnline is set to True; nothing to do!");
        return StatusCode::SUCCESS;
    }

    // launching Write Cond Handle
    SG::WriteCondHandle<RpcCondDbData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<RpcCondDbData> writeCdo{std::make_unique<RpcCondDbData>()};

    // Start with an infinite range and narrow it down as needed
    EventIDRange rangeW = IOVInfiniteRange::infiniteMixed();

    // data only
    if (m_isData) {
        ATH_CHECK(loadDataDeadPanels(rangeW, writeCdo.get(), ctx));
        ATH_CHECK(loadDataOffPanels(rangeW, writeCdo.get(), ctx));
    }
    // data and MC
    ATH_CHECK(loadMcElementStatus(rangeW, writeCdo.get(), ctx));

    if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record RpcCondDbData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

    return StatusCode::SUCCESS;
}

// loadDataDeadPanels
StatusCode RpcCondDbAlg::loadDataDeadPanels(EventIDRange& rangeW, RpcCondDbData* writeCdo, const EventContext& ctx) const {
    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey_folder_da_deadPanels, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (readCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // intersect validity range of thsi obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << range << ", range of output is " << rangeW);
    CondAttrListCollection::const_iterator itr;
    unsigned int chan_index = 0;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        unsigned int chanNum = readCdo->chanNum(chan_index);
        const std::string& sector_name = readCdo->chanName(chanNum);

        const coral::AttributeList& atr = itr->second;

        std::string panel_dead;
        std::string panel_reason;
        if (atr.size()) {
            panel_dead = *(static_cast<const std::string*>((atr["RpcDeadROPanelIds"]).addressOfData()));
            panel_reason = *(static_cast<const std::string*>((atr["RpcDeadROPanelReasons"]).addressOfData()));

            ATH_MSG_DEBUG("panel_dead " << panel_dead);
            ATH_MSG_DEBUG("panel_reason " << panel_reason);

            constexpr std::string_view delimiter{","};
            auto info_panel = CxxUtils::tokenize(panel_dead, delimiter);

            Identifier PanelId;

            for (auto & ch_tmp : info_panel) {
                PanelId =  CxxUtils::atoi(ch_tmp);
                ATH_MSG_DEBUG("info_panel " << ch_tmp << " " << PanelId);

                if (PanelId.get_compact()) {
                    ATH_MSG_DEBUG("DEADPANEL " << m_idHelperSvc->rpcIdHelper().show_to_string(PanelId));
                    Identifier atlasId = m_idHelperSvc->rpcIdHelper().panelID(PanelId);
                    std::stringstream ss;
                    ss << "panel_" << sector_name << "_" << m_idHelperSvc->rpcIdHelper().show_to_string(PanelId);
                    if (atlasId != 0) writeCdo->setDeadPanel(ss.str(), atlasId);
                    ATH_MSG_DEBUG("push-back");
                }
            }
        }
        chan_index++;
    }
    return StatusCode::SUCCESS;
}

// loadDataOffPanels
StatusCode RpcCondDbAlg::loadDataOffPanels(EventIDRange& rangeW, RpcCondDbData* writeCdo, const EventContext& ctx) const {
    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey_folder_da_offPanels, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (readCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // intersect validity range of thsi obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << range << ", range of output is " << rangeW);

    CondAttrListCollection::const_iterator itr;
    unsigned int chan_index = 0;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        unsigned int chanNum = readCdo->chanNum(chan_index);
        const std::string& sector_name = readCdo->chanName(chanNum);

        const coral::AttributeList& atr = itr->second;

        std::string panel_off;
        std::string panel_reason;
        if (atr.size()) {
            panel_off = *(static_cast<const std::string*>((atr["RpcOffROPanelIds"]).addressOfData()));
            panel_reason = *(static_cast<const std::string*>((atr["RpcOffROPanelReasons"]).addressOfData()));

            ATH_MSG_DEBUG("panel_off " << panel_off);
            ATH_MSG_DEBUG("panel_reason " << panel_reason);

            constexpr std::string_view delimiter{","};
            auto info_panel = CxxUtils::tokenize(panel_off, delimiter);

            Identifier PanelId;

            for (auto & ch_tmp : info_panel) {
                PanelId = CxxUtils::atoi(ch_tmp);
                ATH_MSG_DEBUG("info_panel " << ch_tmp << " " << PanelId);

                if (PanelId.get_compact()) {
                    ATH_MSG_DEBUG("OFFPANEL " << m_idHelperSvc->rpcIdHelper().show_to_string(PanelId));
                    Identifier atlasId = m_idHelperSvc->rpcIdHelper().panelID(PanelId);
                    std::stringstream ss;
                    ss << "panel_" << sector_name << "_" << m_idHelperSvc->rpcIdHelper().show_to_string(PanelId);
                    if (atlasId != 0) writeCdo->setOffPanel(ss.str(), atlasId);
                    ATH_MSG_DEBUG("push-back");
                }
            }
        }
        chan_index++;
    }
    return StatusCode::SUCCESS;
}

// loadMcElementStatus
StatusCode RpcCondDbAlg::loadMcElementStatus(EventIDRange& rangeW, RpcCondDbData* writeCdo, const EventContext& ctx) const {
    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey_folder_mc_deadElements, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (readCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // intersect validity range of thsi obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << range << ", range of output is " << rangeW);

    CondAttrListCollection::const_iterator itr;
    unsigned int chan_index = 0;
    unsigned int iFracDeadStrip = 0;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        CondAttrListCollection::ChanNum channum = itr->first;
        Identifier chamberId = Identifier(channum);

        std::string eff_panel, striplist, eff;

        eff_panel = *(static_cast<const std::string*>((atr["PanelRes"]).addressOfData()));
        striplist = *(static_cast<const std::string*>((atr["StripStatus"]).addressOfData()));

        ATH_MSG_DEBUG("-----------------------------entry #" << chan_index);
        ATH_MSG_DEBUG("channel ID = Panel ID " << channum << " as identif. = " << m_idHelperSvc->rpcIdHelper().show_to_string(chamberId));
        ATH_MSG_DEBUG("eff_panel load is " << eff_panel);
        ATH_MSG_DEBUG("striplist load is " << striplist << " " << striplist.size());

        // Efficiencies and Cluster Sizes
        constexpr std::string_view delimiter{" "};
        const auto info_panel = CxxUtils::tokenize(eff_panel, delimiter);

        int DBversion = CxxUtils::atoi(info_panel[0]);

        int npanelstrip = CxxUtils::atoi(info_panel[2]);

        double ProjectedTracks = CxxUtils::atof(info_panel[1]);
        writeCdo->setProjectedTrack(chamberId, ProjectedTracks);

        double Efficiency = CxxUtils::atof(info_panel[3]);
        writeCdo->setEfficiency(chamberId, Efficiency);

        if (Efficiency <= m_panelEfficiency) writeCdo->setLowEffPanel(chamberId);

        double GapEfficiency = CxxUtils::atof(info_panel[5]);
        writeCdo->setGapEfficiency(chamberId, GapEfficiency);

        double MeanClusterSize = CxxUtils::atof(info_panel[17]);
        writeCdo->setMeanClusterSize(chamberId, MeanClusterSize);

        if (DBversion > 2) {
            double FracClusterSize1 = CxxUtils::atof(info_panel[19]) + CxxUtils::atof(info_panel[20]) * 10000;
            writeCdo->setFracClusterSize1(chamberId, FracClusterSize1);

            double FracClusterSize2 = CxxUtils::atof(info_panel[21]) + CxxUtils::atof(info_panel[22]) * 10000;
            writeCdo->setFracClusterSize2(chamberId, FracClusterSize2);

            double FracClusterSize3 = CxxUtils::atof(info_panel[23]) + CxxUtils::atof(info_panel[24]) * 10000;
            writeCdo->setFracClusterSize3(chamberId, FracClusterSize3);
        } else {
            if (info_panel.size() > 20) {
                double FracClusterSize1 = CxxUtils::atof(info_panel[19]);
                writeCdo->setFracClusterSize1(chamberId, FracClusterSize1);

                double FracClusterSize2 = CxxUtils::atof(info_panel[20]);
                writeCdo->setFracClusterSize2(chamberId, FracClusterSize2);
            } else {
                writeCdo->setFracClusterSize1(chamberId, 0.6);
                writeCdo->setFracClusterSize2(chamberId, 0.2);
                ATH_MSG_DEBUG("Panel with incomplete info in the DB, size = " << info_panel.size() << " instead of required >20");
                ATH_MSG_DEBUG("PanelId = " << channum << " = " << m_idHelperSvc->rpcIdHelper().show_to_string(chamberId));
                ATH_MSG_DEBUG("Cluster Size 1 and 2 fractions are set to 0.6 and 0.2 for this chamber.");
            }
        }

        // strip status
        // n chars = #strips (status between 0--9)
        int countdeadstrip = 0;
        int countdeadstripinfidarea = 0;
        int countpanelstrip = 0;

        // update for the timing and error on timing
        // new info strip |status time error_on_time|
        constexpr std::string_view delimiter_strip{"|"};
        std::string strip_status_list = "";

        const auto info_strip= CxxUtils::tokenize(striplist, delimiter_strip);
        if (info_strip.size() > 1) {
            for (unsigned int i = 0; i < info_strip.size(); ++i) {
                const std::string &ch_strip2 = info_strip[i];

                constexpr std::string_view delimiter_strip2{" "};

                auto info_strip2 = CxxUtils::tokenize(ch_strip2, delimiter_strip2);

                double Time = CxxUtils::atof(info_strip2[1]);
                double SigmaTime = CxxUtils::atof(info_strip2[2]);
                const auto &strip_status = info_strip2[0];

                strip_status_list += strip_status;

                Identifier strip_id;
                CondAttrListCollection::ChanNum stripnum;
                stripnum = channum + i * 4;
                strip_id = channum + i * 4;

                ATH_MSG_DEBUG("strip " << strip_id << " has time " << Time << " and " << SigmaTime);

                writeCdo->setStripTime(strip_id, std::vector<double>{Time, SigmaTime});

                ATH_MSG_VERBOSE("strip #" << i + 1 << " strip_id " << stripnum << " expanded "
                                          << m_idHelperSvc->rpcIdHelper().show_to_string(strip_id));

                ++countpanelstrip;

                if (strip_status[0] == '0') {
                    writeCdo->setDeadStrip(strip_id);
                    ++countdeadstrip;

                    if (i > 1 && i < info_strip.size() - 2) {
                        // strip in the fiducial area for the efficiency measurement
                        ++countdeadstripinfidarea;
                    }
                }
            }
        } else {
            ATH_MSG_DEBUG("no timing info");

            for (unsigned int i = 0; i < striplist.size(); i++) {
                char part_strip = striplist[i];
                strip_status_list += part_strip;
                char ch_panel = part_strip;

                Identifier strip_id;
                CondAttrListCollection::ChanNum stripnum;
                stripnum = channum + i * 4;
                strip_id = channum + i * 4;

                ATH_MSG_VERBOSE("strip #" << i + 1 << " info_strip " << part_strip << " strip_id " << stripnum << " expanded "
                                          << m_idHelperSvc->rpcIdHelper().show_to_string(strip_id) << " panel = " << ch_panel);

                ++countpanelstrip;

                if (part_strip == '0') {
                    ++countdeadstrip;
                    writeCdo->setDeadStrip(strip_id);
                    if (i > 1 && i < striplist.size() - 2) {
                        // strip in the fiducial area for the efficiency measurement
                        ++countdeadstripinfidarea;
                    }
                }
            }
        }

        if (countpanelstrip != npanelstrip)
            ATH_MSG_WARNING(
                "WARNING (no side effects for this, just a reminder for a proper fix of the DB content) no matching strip number!!! "
                << countpanelstrip << " != " << npanelstrip << " Identifier: " << channum);

        float FracDeadStripMap = 0;
        if (countpanelstrip - 4 > 0) FracDeadStripMap = float(countdeadstripinfidarea) / float(countpanelstrip - 4);

        // store in the suitabel maps
        writeCdo->setDeadStripList(chamberId, strip_status_list);
        writeCdo->setFracDeadStrip(chamberId, FracDeadStripMap);
        ++iFracDeadStrip;

        std::stringstream ss;
        ss << "Size of RPC_PanelFracDeadStripMap " << iFracDeadStrip << "; in panel ";
        ss << channum << " FracDeadStri(in fid.area) " << FracDeadStripMap << " (incl. borders) ";
        if (countpanelstrip == 0)
            ss << "DIVISION BY ZERO IMPOSSIBLE";
        else
            ss << float(countdeadstrip) / float(countpanelstrip);
        ss << " nDeadStrips,InFidArea/nStrips " << countdeadstrip << ",";
        ss << countdeadstripinfidarea << "/" << countpanelstrip;
        ATH_MSG_DEBUG(ss.str());

        ATH_MSG_DEBUG("Efficiency is " << Efficiency << " and fraction is " << FracDeadStripMap << " and thus "
                                       << Efficiency - (0.99 - FracDeadStripMap));

        if (Efficiency - (0.99 - FracDeadStripMap) > 0.) {
            if (Efficiency < 0.995 || FracDeadStripMap > 0.01) {
                if (Efficiency > 0.005 || FracDeadStripMap < 0.99) {
                    std::stringstream msg;
                    msg << "WARNING: Inconsistent panel eff.=" << Efficiency << " and 0.99-dead_frac=" << 0.99 - FracDeadStripMap
                        << " nDeadStrips,InFidArea/nStrips " << countdeadstrip << "," << countdeadstripinfidarea << "/" << countpanelstrip
                        << " for panelId=" << m_idHelperSvc->rpcIdHelper().show_to_string(chamberId);
                    if (Efficiency - (0.99 - FracDeadStripMap) > 0.2)
                        msg << " difference >0.2";
                    else if (Efficiency - (0.99 - FracDeadStripMap) > 0.1)
                        msg << " difference >0.1";
                    else if (Efficiency - (0.99 - FracDeadStripMap) > 0.05)
                        msg << " difference >0.05";
                    else if (Efficiency - (0.99 - FracDeadStripMap) > 0.025)
                        msg << " difference >0.025";
                    else if (Efficiency - (0.99 - FracDeadStripMap) > 0.01)
                        msg << " difference >0.01";
                    else
                        msg << " difference >0 but <=0.01";
                    ATH_MSG_DEBUG(msg.str());
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
