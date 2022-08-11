/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TOPORDO_L1TOPOROD_H
#define L1TOPORDO_L1TOPOROD_H

#include <cstdint>
#include <iostream>


namespace L1Topo {
  
  class L1TopoROD {
  public:
    //! Construct from contents and decode trailers
    L1TopoROD(const uint32_t trailer1, const uint32_t trailer2);
    //! access methods
    uint32_t rodTrailer1() const;
    uint32_t rodTrailer2() const;
    uint32_t shelf() const;
    uint32_t rod() const;
    uint32_t linkErrMap() const;
    uint32_t payloadLength() const;
    uint32_t crc() const;
    bool linkErrs() const;
    bool ct() const;
    bool pc() const;
    bool hc() const;
    bool pe() const;
    bool lm() const;
    bool hm() const;
    bool pt() const;
      
  protected:
    //! method used by constructor to decode word 
    void decode();
  private:
    //! variables
    uint32_t m_rodTrailer1;
    uint32_t m_rodTrailer2;
    
    uint32_t m_shelf;
    uint32_t m_rod;
    uint32_t m_linkErrMap;
    uint32_t m_payloadLength;
    
    uint32_t m_crc;
    bool m_linkErrs;
    bool m_ct;
    bool m_pc;
    bool m_hc;
    bool m_pe;
    bool m_lm;
    bool m_hm;
    bool m_pt;
  };
  std::ostream& operator<<(std::ostream&, const L1TopoROD&);
  
} // namespace L1Topo

#endif // L1TOPORDO_L1TOPOROD_H
