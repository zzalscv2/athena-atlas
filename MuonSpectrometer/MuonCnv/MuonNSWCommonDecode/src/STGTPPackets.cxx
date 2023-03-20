/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>
#include <algorithm>
#include <tuple>

#include "MuonNSWCommonDecode/STGTPPackets.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"
#define PARSE_VAR(var,st,pp,siz) {\
var =  bit_slice<uint64_t,uint32_t>(st,pp,pp+siz-1);\
pp += siz;\
}

Muon::nsw::STGTPPadPacket::STGTPPadPacket (const std::vector<uint32_t>& packet){
  uint pp = 0;

  uint32_t bs[3]={packet[0],packet[1],packet[2]};
  m_coincWege =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_coincidence_wedge-1);     pp+= Muon::nsw::STGTPPad::size_coincidence_wedge;
  uint32_t phi_3 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_phiID_3-1);     pp+= Muon::nsw::STGTPPad::size_phiID_3;
  uint32_t phi_2 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_phiID_2-1);     pp+= Muon::nsw::STGTPPad::size_phiID_2;
  uint32_t phi_1 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_phiID_1-1);     pp+= Muon::nsw::STGTPPad::size_phiID_1;
  uint32_t phi_0 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_phiID_0-1);     pp+= Muon::nsw::STGTPPad::size_phiID_0;
  m_phiIDs[0]=phi_0;
  m_phiIDs[1]=phi_1;
  m_phiIDs[2]=phi_2;
  m_phiIDs[3]=phi_3;
  
  uint32_t band_3 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_bandID_3-1);     pp+= Muon::nsw::STGTPPad::size_bandID_3;
  uint32_t band_2 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_bandID_2-1);     pp+= Muon::nsw::STGTPPad::size_bandID_2;
  uint32_t band_1 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_bandID_1-1);     pp+= Muon::nsw::STGTPPad::size_bandID_1;
  uint32_t band_0 = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_bandID_0-1);     pp+= Muon::nsw::STGTPPad::size_bandID_0;
  m_bandIDs[0]=band_0;
  m_bandIDs[1]=band_1;
  m_bandIDs[2]=band_2;
  m_bandIDs[3]=band_3;

  m_BCID = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_BCID-1);     pp+= Muon::nsw::STGTPPad::size_BCID;

  pp+=Muon::nsw::STGTPPad::size_spare;
  m_idleFlag = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPPad::size_idleFlag-1);     pp+= Muon::nsw::STGTPPad::size_idleFlag; 

}

Muon::nsw::STGTPSegmentPacket::STGTPSegmentPacket(const std::vector<uint32_t>& packet)
{
  uint pp = 0;
  uint32_t bs[8]={packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7]};
  PARSE_VAR(m_lut_choice,bs,pp,Muon::nsw::STGTPSegments::size_lut_choice_selection);
  PARSE_VAR(m_nsw_segment_selector,bs,pp,Muon::nsw::STGTPSegments::size_nsw_segment_selector);
  PARSE_VAR(m_valid_segment_selector,bs,pp,Muon::nsw::STGTPSegments::size_valid_segment_selector);

  PARSE_VAR(m_monitor_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_monitor);
  PARSE_VAR(m_spare_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_spare);
  PARSE_VAR(m_lowRes_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_lowRes);
  PARSE_VAR(m_phiRes_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_phiRes);
  PARSE_VAR(m_dTheta_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_dTheta);
  PARSE_VAR(m_phiID_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_phiID);
  PARSE_VAR(m_RIndex_segment_7,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_7_rIndex);

  PARSE_VAR(m_monitor_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_monitor);
  PARSE_VAR(m_spare_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_spare);
  PARSE_VAR(m_lowRes_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_lowRes);
  PARSE_VAR(m_phiRes_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_phiRes);
  PARSE_VAR(m_dTheta_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_dTheta);
  PARSE_VAR(m_phiID_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_phiID);
  PARSE_VAR(m_RIndex_segment_6,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_6_rIndex);

  PARSE_VAR(m_monitor_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_monitor);
  PARSE_VAR(m_spare_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_spare);
  PARSE_VAR(m_lowRes_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_lowRes);
  PARSE_VAR(m_phiRes_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_phiRes);
  PARSE_VAR(m_dTheta_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_dTheta);
  PARSE_VAR(m_phiID_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_phiID);
  PARSE_VAR(m_RIndex_segment_5,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_5_rIndex);

  PARSE_VAR(m_monitor_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_monitor);
  PARSE_VAR(m_spare_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_spare);
  PARSE_VAR(m_lowRes_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_lowRes);
  PARSE_VAR(m_phiRes_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_phiRes);
  PARSE_VAR(m_dTheta_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_dTheta);
  PARSE_VAR(m_phiID_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_phiID);
  PARSE_VAR(m_RIndex_segment_4,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_4_rIndex);

  PARSE_VAR(m_monitor_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_monitor);
  PARSE_VAR(m_spare_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_spare);
  PARSE_VAR(m_lowRes_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_lowRes);
  PARSE_VAR(m_phiRes_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_phiRes);
  PARSE_VAR(m_dTheta_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_dTheta);
  PARSE_VAR(m_phiID_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_phiID);
  PARSE_VAR(m_RIndex_segment_3,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_3_rIndex);

  PARSE_VAR(m_monitor_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_monitor);
  PARSE_VAR(m_spare_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_spare);
  PARSE_VAR(m_lowRes_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_lowRes);
  PARSE_VAR(m_phiRes_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_phiRes);
  PARSE_VAR(m_dTheta_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_dTheta);
  PARSE_VAR(m_phiID_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_phiID);
  PARSE_VAR(m_RIndex_segment_2,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_2_rIndex);

  PARSE_VAR(m_monitor_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_monitor);
  PARSE_VAR(m_spare_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_spare);
  PARSE_VAR(m_lowRes_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_lowRes);
  PARSE_VAR(m_phiRes_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_phiRes);
  PARSE_VAR(m_dTheta_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_dTheta);
  PARSE_VAR(m_phiID_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_phiID);
  PARSE_VAR(m_RIndex_segment_1,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_1_rIndex); 
 
  PARSE_VAR(m_monitor_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_monitor);
  PARSE_VAR(m_spare_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_spare);
  PARSE_VAR(m_lowRes_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_lowRes);
  PARSE_VAR(m_phiRes_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_phiRes); 
  PARSE_VAR(m_dTheta_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_dTheta); 
  PARSE_VAR(m_phiID_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_phiID);
  PARSE_VAR(m_RIndex_segment_0,bs,pp,Muon::nsw::STGTPSegments::size_output_segment_0_rIndex);

  PARSE_VAR(m_BCID,bs,pp,Muon::nsw::STGTPSegments::size_bcid);
  PARSE_VAR(m_sectorID,bs,pp,Muon::nsw::STGTPSegments::size_sectorID);


}

