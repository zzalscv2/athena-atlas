#ifndef MUONEVENTTPCNV_NSW_PADTRIGGERDATA_P1_H
#define MUONEVENTTPCNV_NSW_PADTRIGGERDATA_P1_H

#include <vector>

namespace Muon {
using uint32_vt = std::vector<uint32_t>;
struct NSW_PadTriggerData_p1 {

    uint32_t m_sourceid{0};
    uint32_t m_flags{0};
    uint32_t m_ec{0};
    uint32_t m_fragid{0};
    uint32_t m_secid{0};
    uint32_t m_spare{0};
    uint32_t m_orbit{0};
    uint32_t m_bcid{0};
    uint32_t m_l1id{0};
    uint32_t m_hit_n{0};
    uint32_t m_pfeb_n{0};
    uint32_t m_trigger_n{0};
    uint32_t m_bcid_n{0};
    uint32_vt m_hit_relbcid{};
    uint32_vt m_hit_pfeb{};
    uint32_vt m_hit_tdschannel{};
    uint32_vt m_hit_vmmchannel{};
    uint32_vt m_hit_vmm{};
    uint32_vt m_hit_padchannel{};
    uint32_vt m_pfeb_addr{};
    uint32_vt m_pfeb_nchan{};
    uint32_vt m_pfeb_disconnected{};
    uint32_vt m_trigger_bandid{};
    uint32_vt m_trigger_phiid{};
    uint32_vt m_trigger_relbcid{};
    uint32_vt m_bcid_rel{};
    uint32_vt m_bcid_status{};
    uint32_vt m_bcid_multzero{};

};
} // namespace Muon

#endif // MUONEVENTTPCNV_NSW_PADTRIGGERDATA_P1_H
