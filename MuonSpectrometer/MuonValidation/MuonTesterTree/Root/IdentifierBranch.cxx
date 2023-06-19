/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/IdentifierBranch.h>
#ifndef XAOD_ANALYSIS
namespace MuonVal {

MuonIdentifierBranch::MuonIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonTesterBranch(tree, grp_name) {}

bool MuonIdentifierBranch::fill(const EventContext&) { return true; }
bool MuonIdentifierBranch::init() { return m_idHelperSvc.retrieve().isSuccess(); }
const Muon::IMuonIdHelperSvc* MuonIdentifierBranch::idHelperSvc() const { return m_idHelperSvc.get(); }

void MuonIdentifierBranch::push_back(const Identifier& id) {
    m_stationIndex.push_back(m_idHelperSvc->stationName(id));
    m_stationEta.push_back(m_idHelperSvc->stationEta(id));
    m_stationPhi.push_back(m_idHelperSvc->stationPhi(id));
}
///###############################################################
///                 MdtIdentifierBranch
///###############################################################
MdtIdentifierBranch::MdtIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void MdtIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const MdtIdHelper& idHelper{idHelperSvc()->mdtIdHelper()};
    m_multiLayer.push_back(idHelper.multilayer(id));
    m_tubeLayer.push_back(idHelper.tubeLayer(id));
    m_tube.push_back(idHelper.tube(id));
}
///###############################################################
///                 RpcIdentifierBranch
///###############################################################
RpcIdentifierBranch::RpcIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void RpcIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const RpcIdHelper& idHelper{idHelperSvc()->rpcIdHelper()};
    m_gasGap.push_back(idHelper.gasGap(id));
    m_doubletR.push_back(idHelper.doubletR(id));
    m_doubletZ.push_back(idHelper.doubletZ(id));
    m_doubletPhi.push_back(idHelper.doubletPhi(id));
    m_measuresPhi.push_back(idHelper.measuresPhi(id));
    m_strip.push_back(idHelper.strip(id));
}

///###############################################################
///                 CscIdentifierBranch
///###############################################################
CscIdentifierBranch::CscIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void CscIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const CscIdHelper& idHelper{idHelperSvc()->cscIdHelper()};
    m_chamberLayer.push_back(idHelper.chamberLayer(id));
    m_wireLayer.push_back(idHelper.wireLayer(id));
    m_measuresPhi.push_back(idHelper.measuresPhi(id));
    m_strip.push_back(idHelper.strip(id));
}

///###############################################################
///                 TgcIdentifierBranch
///###############################################################
TgcIdentifierBranch::TgcIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void TgcIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const TgcIdHelper& idHelper{idHelperSvc()->tgcIdHelper()};
    m_gasgap.push_back(idHelper.gasGap(id));
    m_measuresPhi.push_back(idHelper.measuresPhi(id));
    m_channel.push_back(idHelper.channel(id));
}

///###############################################################
///                 sTgcIdentifierBranch
///###############################################################
sTgcIdentifierBranch::sTgcIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void sTgcIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const sTgcIdHelper& idHelper{idHelperSvc()->stgcIdHelper()};
    m_gas_gap.push_back(idHelper.gasGap(id));
    m_multiplet.push_back(idHelper.multilayer(id));
    m_channel_type.push_back(idHelper.channelType(id));
    m_channel.push_back(idHelper.channel(id));
}

///###############################################################
///                 MmIdentifierBranch
///###############################################################
MmIdentifierBranch::MmIdentifierBranch(MuonTesterTree& tree, const std::string& grp_name) : MuonIdentifierBranch(tree, grp_name) {}

void MmIdentifierBranch::push_back(const Identifier& id) {
    MuonIdentifierBranch::push_back(id);
    const MmIdHelper& idHelper{idHelperSvc()->mmIdHelper()};
    m_gas_gap.push_back(idHelper.gasGap(id));
    m_multiplet.push_back(idHelper.multilayer(id));
    m_channel.push_back(idHelper.channel(id));
}
}
#endif