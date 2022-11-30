/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_MMART_H_
#define _MUON_NSW_MMART_H_

#include <stdint.h>
#include <vector>
#include <exception>


namespace Muon
{
  namespace nsw
  {
    class MMARTPacket
    {
    public:

      MMARTPacket (std::vector<uint32_t> payload);
      virtual ~MMARTPacket () = default;
      uint32_t art_BCID () const {return m_art_BCID;};
      uint32_t art_pipeID () const {return m_art_pipeID;};
      uint32_t art_fiberID () const {return m_art_fiberID;};
      uint64_t art_VMMmap () const {return m_art_VMMmap;};
      const std::vector<uint32_t>& art_ARTs () const {return m_art_ARTs;};
      const std::vector<std::pair<uint8_t,uint16_t>>& channels () const {return m_channels;}

    private:
      uint32_t m_art_BCID;
      uint32_t m_art_pipeID;
      uint32_t m_art_fiberID;
      uint64_t m_art_VMMmap;
      //remember ART 7 is first, ART 0 is last!
      std::vector<uint32_t> m_art_ARTs;
      //processed values
      std::vector< std::pair<uint8_t,uint16_t> > m_channels; //(layer,channel) //using this type since RDOs have already this struct

      uint8_t getLayer () const {return (int)(m_art_fiberID/4.)+1 + (m_art_pipeID*2);};
      std::vector<std::tuple<uint8_t,uint8_t,uint8_t>> VMMmapToHits ();

      int getBoardPosition ( int board );
      int getVMMChannelPosition ( int boardPosition, int vmm, int ch );

    };
  }
}

#endif // _MUON_NSW_MMART_H_
