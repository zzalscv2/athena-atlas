/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_TRIGGER_MMMON_ELINK_H_
#define _MUON_NSW_TRIGGER_MMMON_ELINK_H_

#include <stdint.h>
#include <vector>
#include <exception>

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

namespace Muon
{
  namespace nsw
  {
    class NSWResourceId;

    class NSWTriggerElinkException;

    class NSWTriggerMMMonElink : public NSWTriggerElink
    {
     public:

      NSWTriggerMMMonElink (const uint32_t *bs, uint32_t remaining);
      virtual ~NSWTriggerMMMonElink () = default;


      uint32_t head_fragID () const {return m_head_fragID;};
      uint32_t head_sectID () const {return m_head_sectID;};
      uint32_t head_EC () const {return m_head_EC;};
      uint32_t head_flags () const {return m_head_flags;};
      uint32_t head_BCID () const {return m_head_BCID;};
      uint32_t head_orbit () const {return m_head_orbit;};
      uint32_t head_spare () const {return m_head_spare;};
      uint32_t L1ID () const {return m_L1ID;};
      uint32_t head_coincBCID () const {return m_head_coincBCID;};
      uint32_t head_regionCount () const {return m_head_regionCount;};
      uint32_t head_coincRegion () const {return m_head_coincRegion;};
      uint32_t head_reserved () const {return m_head_reserved;};
      const std::vector<uint32_t>& finder_streamID () const {return m_finder_streamID;}
      const std::vector<uint32_t>& finder_regionCount () const {return m_finder_regionCount;}
      const std::vector<uint32_t>& finder_triggerID () const {return m_finder_triggerID;};
      const std::vector<uint32_t>& finder_V1 () const {return m_finder_V1;};
      const std::vector<uint32_t>& finder_V0 () const {return m_finder_V0;};
      const std::vector<uint32_t>& finder_U1 () const {return m_finder_U1;};
      const std::vector<uint32_t>& finder_U0 () const {return m_finder_U0;};
      const std::vector<uint32_t>& finder_X3 () const {return m_finder_X3;};
      const std::vector<uint32_t>& finder_X2 () const {return m_finder_X2;};
      const std::vector<uint32_t>& finder_X1 () const {return m_finder_X1;};
      const std::vector<uint32_t>& finder_X0 () const {return m_finder_X0;};
      const std::vector<uint32_t>& fitter_streamID () const {return m_fitter_streamID;};
      const std::vector<uint32_t>& fitter_regionCount () const {return m_fitter_regionCount;};
      const std::vector<uint32_t>& fitter_triggerID () const {return m_fitter_triggerID;};
      const std::vector<uint32_t>& fitter_filler () const {return m_fitter_filler;};
      const std::vector<uint32_t>& fitter_mxG () const {return m_fitter_mxG;};
      const std::vector<uint32_t>& fitter_muG () const {return m_fitter_muG;};
      const std::vector<uint32_t>& fitter_mvG () const {return m_fitter_mvG;};
      const std::vector<uint32_t>& fitter_mxL () const {return m_fitter_mxL;};
      const std::vector<uint32_t>& fitter_mx_ROI () const {return m_fitter_mx_ROI;};
      const std::vector<uint32_t>& fitter_dTheta () const {return m_fitter_dTheta;};
      const std::vector<uint32_t>& fitter_zero () const {return m_fitter_zero;};
      const std::vector<uint32_t>& fitter_phiSign () const {return m_fitter_phiSign;};
      const std::vector<uint32_t>& fitter_phiBin () const {return m_fitter_phiBin;};
      const std::vector<uint32_t>& fitter_rBin () const {return m_fitter_rBin;};
      uint32_t trailer_CRC () const {return m_trailer_CRC;};

     private:

       uint32_t m_head_fragID;
       uint32_t m_head_sectID;
       uint32_t m_head_EC;
       uint32_t m_head_flags;
       uint32_t m_head_BCID;
       uint32_t m_head_orbit;
       uint32_t m_head_spare;
       uint32_t m_L1ID;
       uint32_t m_head_coincBCID;
       uint32_t m_head_regionCount;
       uint32_t m_head_coincRegion;
       uint32_t m_head_reserved;
       std::vector<uint32_t> m_finder_streamID;
       std::vector<uint32_t> m_finder_regionCount;
       std::vector<uint32_t> m_finder_triggerID;
       std::vector<uint32_t> m_finder_V1;
       std::vector<uint32_t> m_finder_V0;
       std::vector<uint32_t> m_finder_U1;
       std::vector<uint32_t> m_finder_U0;
       std::vector<uint32_t> m_finder_X3;
       std::vector<uint32_t> m_finder_X2;
       std::vector<uint32_t> m_finder_X1;
       std::vector<uint32_t> m_finder_X0;
       std::vector<uint32_t> m_fitter_streamID;
       std::vector<uint32_t> m_fitter_regionCount;
       std::vector<uint32_t> m_fitter_triggerID;
       std::vector<uint32_t> m_fitter_filler;
       std::vector<uint32_t> m_fitter_mxG;
       std::vector<uint32_t> m_fitter_muG;
       std::vector<uint32_t> m_fitter_mvG;
       std::vector<uint32_t> m_fitter_mxL;
       std::vector<uint32_t> m_fitter_mx_ROI;
       std::vector<uint32_t> m_fitter_dTheta;
       std::vector<uint32_t> m_fitter_zero;
       std::vector<uint32_t> m_fitter_phiSign;
       std::vector<uint32_t> m_fitter_phiBin;
       std::vector<uint32_t> m_fitter_rBin;
       uint32_t m_trailer_CRC;

    };
  }
}


#endif // _MUON_NSW_TRIGGER_MMMON_ELINK_H_
