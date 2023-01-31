/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"

Muon::nsw::NSWTriggerMMMonElink::NSWTriggerMMMonElink (const uint32_t *bs, const uint32_t remaining):
  NSWTriggerElink (bs, remaining)
{
  // 2 felix header 32b words already decoded;
  uint pp = 2 * 32;

  //once format finalized, checking a minimum size

  //NB bit_slice(start, end) includes edges

  m_head_fragID =         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_fragID-1);        pp+=Muon::nsw::MMTPMON::size_head_fragID;
  m_head_sectID =         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_sectID-1);        pp+=Muon::nsw::MMTPMON::size_head_sectID;
  m_head_EC =             bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_EC-1);            pp+=Muon::nsw::MMTPMON::size_head_EC;
  m_head_flags =          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_flags-1);         pp+=Muon::nsw::MMTPMON::size_head_flags;
  m_head_BCID =           bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_BCID-1);          pp+=Muon::nsw::MMTPMON::size_head_BCID;
  m_head_orbit =          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_orbit-1);         pp+=Muon::nsw::MMTPMON::size_head_orbit;
  m_head_spare =          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_spare-1);         pp+=Muon::nsw::MMTPMON::size_head_spare;
  m_L1ID =                bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_L1ID-1);               pp+=Muon::nsw::MMTPMON::size_L1ID;
  m_head_coincBCID =      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_coincBCID-1);     pp+=Muon::nsw::MMTPMON::size_head_coincBCID;
  m_head_regionCount =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_regionCount-1);   pp+=Muon::nsw::MMTPMON::size_head_regionCount;
  m_head_coincRegion =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_coincRegion-1);   pp+=Muon::nsw::MMTPMON::size_head_coincRegion;
  m_head_reserved =       bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_head_reserved-1);      pp+=Muon::nsw::MMTPMON::size_head_reserved;

  while ( pp < (remaining-2) * sizeof(uint32_t) ){
    //NB here using sizes from the finder but stream header is identical
    uint32_t current_streamID =     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_streamID-1);    pp+=Muon::nsw::MMTPMON::size_finder_streamID;
    uint32_t current_regionCount =  bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_regionCount-1); pp+=Muon::nsw::MMTPMON::size_finder_regionCount;
    uint32_t current_triggerID =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_triggerID-1);   pp+=Muon::nsw::MMTPMON::size_finder_triggerID;

    if (current_streamID == 0b10110001){
      //finder
      m_finder_streamID.push_back(    current_streamID);
      m_finder_regionCount.push_back( current_regionCount);
      m_finder_triggerID.push_back(   current_triggerID);
      m_finder_V1.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_V1-1));          pp+=Muon::nsw::MMTPMON::size_finder_V1;
      m_finder_V0.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_V0-1));          pp+=Muon::nsw::MMTPMON::size_finder_V0;
      m_finder_U1.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_U1-1));          pp+=Muon::nsw::MMTPMON::size_finder_U1;
      m_finder_U0.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_U0-1));          pp+=Muon::nsw::MMTPMON::size_finder_U0;
      m_finder_X3.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_X3-1));          pp+=Muon::nsw::MMTPMON::size_finder_X3;
      m_finder_X2.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_X2-1));          pp+=Muon::nsw::MMTPMON::size_finder_X2;
      m_finder_X1.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_X1-1));          pp+=Muon::nsw::MMTPMON::size_finder_X1;
      m_finder_X0.push_back(          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_finder_X0-1));          pp+=Muon::nsw::MMTPMON::size_finder_X0;

    } else if (current_streamID == 0b10110010) {
      //fitter
      m_fitter_streamID.push_back(    current_streamID);
      m_fitter_regionCount.push_back( current_regionCount);
      m_fitter_triggerID.push_back(   current_triggerID);
      m_fitter_filler.push_back(      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_filler-1));      pp+=Muon::nsw::MMTPMON::size_fitter_filler;
      m_fitter_mxG.push_back(         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_mxG-1));         pp+=Muon::nsw::MMTPMON::size_fitter_mxG;
      m_fitter_muG.push_back(         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_muG-1));         pp+=Muon::nsw::MMTPMON::size_fitter_muG;
      m_fitter_mvG.push_back(         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_mvG-1));         pp+=Muon::nsw::MMTPMON::size_fitter_mvG;
      m_fitter_mxL.push_back(         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_mxL-1));         pp+=Muon::nsw::MMTPMON::size_fitter_mxL;
      m_fitter_mx_ROI.push_back(      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_mx_ROI-1));      pp+=Muon::nsw::MMTPMON::size_fitter_mx_ROI;
      m_fitter_dTheta.push_back(      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_dTheta-1));      pp+=Muon::nsw::MMTPMON::size_fitter_dTheta;
      m_fitter_zero.push_back(        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_zero-1));        pp+=Muon::nsw::MMTPMON::size_fitter_zero;
      m_fitter_phiSign.push_back(     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_phiSign-1));     pp+=Muon::nsw::MMTPMON::size_fitter_phiSign;
      m_fitter_phiBin.push_back(      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_phiBin-1));      pp+=Muon::nsw::MMTPMON::size_fitter_phiBin;
      m_fitter_rBin.push_back(        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_fitter_rBin-1));        pp+=Muon::nsw::MMTPMON::size_fitter_rBin;


    } else { 
      Muon::nsw::NSWTriggerElinkException e ("Stream ID in MMTP monitoring packet now recognized");
      throw e;
    }

    //warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding?
    m_trailer_CRC =         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPMON::size_trailer_CRC-1);        pp+=Muon::nsw::MMTPMON::size_trailer_CRC;

  }

}
