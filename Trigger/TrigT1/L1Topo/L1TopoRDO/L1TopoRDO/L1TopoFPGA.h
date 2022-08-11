/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TOPORDO_L1TOPOFPGA_H
#define L1TOPORDO_L1TOPOFPGA_H

#include <cstdint>
#include <iostream>


namespace L1Topo {
  
  class L1TopoFPGA {
  public:
    //! Construct from contents and decode trailers
    L1TopoFPGA(const uint32_t trailer1, const uint32_t trailer2);
    //! access methods
    uint32_t fpgaTrailer1() const;
    uint32_t fpgaTrailer2() const;
    size_t fpgaBlockSize() const;
    uint32_t topoNumber() const;
    uint32_t fpgaNumber() const;
    uint32_t numSlices() const;
    uint32_t sliceNumber() const;
    uint32_t crc() const;
    bool ct() const;
    bool sm() const;
    bool pe() const;
    bool lm() const;
    bool hm() const;
    bool pt() const;
      
  protected:
    //! method used by constructor to decode word 
    void decode();
  private:
    //! variables
    uint32_t m_fpgaTrailer1;
    uint32_t m_fpgaTrailer2;
    
    size_t m_fpgaBlockSize;
    
    uint32_t m_topoNumber;
    uint32_t m_fpgaNumber;
    uint32_t m_numSlices;
    uint32_t m_sliceNumber;
    uint32_t m_crc;
    
    bool m_ct;
    bool m_sm;
    bool m_pe;
    bool m_lm;
    bool m_hm;
    bool m_pt;
    
  };
  std::ostream& operator<<(std::ostream&, const L1TopoFPGA&);
  
} // namespace L1Topo

#endif // L1TOPORDO_L1TOPOFPGA_H
