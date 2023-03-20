/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_STGTPPACKETS_H_
#define _MUON_NSW_STGTPPACKETS_H_

#include <stdint.h>
#include <vector>
#include <exception>
#include <array>

namespace Muon
{
  namespace nsw
  {
    class STGTPPadPacket
    {
    public:

      STGTPPadPacket (const std::vector<uint32_t>& payload);
      virtual ~STGTPPadPacket () = default;
      uint32_t BCID () const {return m_BCID;};
      uint32_t BandID (int num) const {return m_bandIDs.at(num);};
      uint32_t PhiID (int num) const {return m_phiIDs.at(num);};
      uint32_t PadIdleFlag () const {return m_idleFlag;};
    private:
      uint32_t m_BCID;
      std::array<uint32_t,4> m_bandIDs;
      std::array<uint32_t,4> m_phiIDs;
      uint32_t m_idleFlag;
      uint32_t m_coincWege;


    };

    class STGTPSegmentPacket
    {
    public:

       STGTPSegmentPacket(const std::vector<uint32_t>& payload);

       virtual ~STGTPSegmentPacket() = default;
       uint32_t LUT_ChoiceSelection() const { return m_lut_choice;}
       uint32_t NSW_SegmentSelector() const { return m_nsw_segment_selector;}
       uint32_t ValidSegmentSelector() const { return m_valid_segment_selector;}

       uint32_t Monitor_Segment7() const { return m_monitor_segment_7; }
       uint32_t Spare_Segment7() const { return m_spare_segment_7; }
       uint32_t LowRes_Segment7() const { return m_lowRes_segment_7; }
       uint32_t PhiRes_Segment7() const { return m_phiRes_segment_7; }
       uint32_t DTheta_Segment7() const { return m_dTheta_segment_7; }
       uint32_t PhiID_Segment7() const { return m_phiID_segment_7; }
       uint32_t RIndex_Segment7() const { return m_RIndex_segment_7; }

       uint32_t Monitor_Segment6() const { return m_monitor_segment_6; }
       uint32_t Spare_Segment6() const { return m_spare_segment_6; }
       uint32_t LowRes_Segment6() const { return m_lowRes_segment_6; }
       uint32_t PhiRes_Segment6() const { return m_phiRes_segment_6; }
       uint32_t DTheta_Segment6() const { return m_dTheta_segment_6; }
       uint32_t PhiID_Segment6() const { return m_phiID_segment_6; }
       uint32_t RIndex_Segment6() const { return m_RIndex_segment_6; }

       uint32_t Monitor_Segment5() const { return m_monitor_segment_5; }
       uint32_t Spare_Segment5() const { return m_spare_segment_5; }
       uint32_t LowRes_Segment5() const { return m_lowRes_segment_5; }
       uint32_t PhiRes_Segment5() const { return m_phiRes_segment_5; }
       uint32_t DTheta_Segment5() const { return m_dTheta_segment_5; }
       uint32_t PhiID_Segment5() const { return m_phiID_segment_5; }
       uint32_t RIndex_Segment5() const { return m_RIndex_segment_5; }

       uint32_t Monitor_Segment4() const { return m_monitor_segment_4; }
       uint32_t Spare_Segment4() const { return m_spare_segment_4; }
       uint32_t LowRes_Segment4() const { return m_lowRes_segment_4; }
       uint32_t PhiRes_Segment4() const { return m_phiRes_segment_4; }
       uint32_t DTheta_Segment4() const { return m_dTheta_segment_4; }
       uint32_t PhiID_Segment4() const { return m_phiID_segment_4; }
       uint32_t RIndex_Segment4() const { return m_RIndex_segment_4; }

       uint32_t Monitor_Segment3() const { return m_monitor_segment_3; }
       uint32_t Spare_Segment3() const { return m_spare_segment_3; }
       uint32_t LowRes_Segment3() const { return m_lowRes_segment_3; }
       uint32_t PhiRes_Segment3() const { return m_phiRes_segment_3; }
       uint32_t DTheta_Segment3() const { return m_dTheta_segment_3; }
       uint32_t PhiID_Segment3() const { return m_phiID_segment_3; }
       uint32_t RIndex_Segment3() const { return m_RIndex_segment_3; }

       uint32_t Monitor_Segment2() const { return m_monitor_segment_2; }
       uint32_t Spare_Segment2() const { return m_spare_segment_2; }
       uint32_t LowRes_Segment2() const { return m_lowRes_segment_2; }
       uint32_t PhiRes_Segment2() const { return m_phiRes_segment_2; }
       uint32_t DTheta_Segment2() const { return m_dTheta_segment_2; }
       uint32_t PhiID_Segment2() const { return m_phiID_segment_2; }
       uint32_t RIndex_Segment2() const { return m_RIndex_segment_2; }

       uint32_t Monitor_Segment1() const { return m_monitor_segment_1; }
       uint32_t Spare_Segment1() const { return m_spare_segment_1; }
       uint32_t LowRes_Segment1() const { return m_lowRes_segment_1; }
       uint32_t PhiRes_Segment1() const { return m_phiRes_segment_1; }
       uint32_t DTheta_Segment1() const { return m_dTheta_segment_1; }
       uint32_t PhiID_Segment1() const { return m_phiID_segment_1; }
       uint32_t RIndex_Segment1() const { return m_RIndex_segment_1; }

       uint32_t Monitor_Segment0() const { return m_monitor_segment_0; }
       uint32_t Spare_Segment0() const { return m_spare_segment_0; }
       uint32_t LowRes_Segment0() const { return m_lowRes_segment_0; }
       uint32_t PhiRes_Segment0() const { return m_phiRes_segment_0; }
       uint32_t DTheta_Segment0() const { return m_dTheta_segment_0; }
       uint32_t PhiID_Segment0() const { return m_phiID_segment_0; }
       uint32_t RIndex_Segment0() const { return m_RIndex_segment_0; }

       uint32_t BCID () const {return m_BCID;};
       uint32_t SectorID () const {return m_sectorID; }
       
    private:
       uint32_t m_lut_choice;
       uint32_t m_nsw_segment_selector;
       uint32_t m_valid_segment_selector;

       uint32_t m_monitor_segment_7;
       uint32_t m_spare_segment_7;
       uint32_t m_lowRes_segment_7;
       uint32_t m_phiRes_segment_7;
       uint32_t m_dTheta_segment_7;
       uint32_t m_phiID_segment_7;
       uint32_t m_RIndex_segment_7;

       uint32_t m_monitor_segment_6;
       uint32_t m_spare_segment_6;
       uint32_t m_lowRes_segment_6;
       uint32_t m_phiRes_segment_6;
       uint32_t m_dTheta_segment_6;
       uint32_t m_phiID_segment_6;
       uint32_t m_RIndex_segment_6;

       uint32_t m_monitor_segment_5;
       uint32_t m_spare_segment_5;
       uint32_t m_lowRes_segment_5;
       uint32_t m_phiRes_segment_5;
       uint32_t m_dTheta_segment_5;
       uint32_t m_phiID_segment_5;
       uint32_t m_RIndex_segment_5;

       uint32_t m_monitor_segment_4;
       uint32_t m_spare_segment_4;
       uint32_t m_lowRes_segment_4;
       uint32_t m_phiRes_segment_4;
       uint32_t m_dTheta_segment_4;
       uint32_t m_phiID_segment_4;
       uint32_t m_RIndex_segment_4;

       uint32_t m_monitor_segment_3;
       uint32_t m_spare_segment_3;
       uint32_t m_lowRes_segment_3;
       uint32_t m_phiRes_segment_3;
       uint32_t m_dTheta_segment_3;
       uint32_t m_phiID_segment_3;
       uint32_t m_RIndex_segment_3;

       uint32_t m_monitor_segment_2;
       uint32_t m_spare_segment_2;
       uint32_t m_lowRes_segment_2;
       uint32_t m_phiRes_segment_2;
       uint32_t m_dTheta_segment_2;
       uint32_t m_phiID_segment_2;
       uint32_t m_RIndex_segment_2;

       uint32_t m_monitor_segment_1;
       uint32_t m_spare_segment_1;
       uint32_t m_lowRes_segment_1;
       uint32_t m_phiRes_segment_1;
       uint32_t m_dTheta_segment_1;
       uint32_t m_phiID_segment_1;
       uint32_t m_RIndex_segment_1;

       uint32_t m_monitor_segment_0;
       uint32_t m_spare_segment_0; 
       uint32_t m_lowRes_segment_0;
       uint32_t m_phiRes_segment_0;
       uint32_t m_dTheta_segment_0;
       uint32_t m_phiID_segment_0;
       uint32_t m_RIndex_segment_0;

       uint32_t m_BCID;
       uint32_t m_sectorID; 
    };


  }
}

#endif // _MUON_NSW_STGPPACKETS_H_
