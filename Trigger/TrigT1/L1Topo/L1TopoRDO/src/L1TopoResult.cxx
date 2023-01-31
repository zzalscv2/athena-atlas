/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoRDO/L1TopoResult.h"
#include "L1TopoRDO/Helpers.h"


namespace L1Topo {

  L1TopoResult::L1TopoResult():
    m_status(true)
  {}

  L1TopoResult::L1TopoResult(const DataVector<xAOD::L1TopoRawData> &container):
    m_status(true)
  {
    this->decode(container);
  }

  L1TopoResult::~L1TopoResult()
  {}

  void L1TopoResult::decode(const DataVector<xAOD::L1TopoRawData> &container)
  {    
    m_overflows.reset();
    m_decisions.reset();
    m_l1topoROD.clear();
    m_l1topoFPGA.clear();
    for(const xAOD::L1TopoRawData* l1topo_raw : container) {
      const std::vector<uint32_t>& dataWords = l1topo_raw->dataWords();
      size_t nWords = dataWords.size();
      if (nWords!=50) {
	m_status = false;
	return;
      }else {
	m_status = true;
      }
      uint32_t rodTrailer2 = dataWords[--nWords];
      uint32_t rodTrailer1 = dataWords[--nWords];

      m_l1topoROD.emplace_back(new L1Topo::L1TopoROD(rodTrailer1, rodTrailer2));


      for (size_t i = nWords; i --> 0;) {
	if ((i+1)%8==0) {
	  uint32_t fpgaTrailer2 = dataWords[i];
	  uint32_t fpgaTrailer1 = dataWords[--i];

	  m_l1topoFPGA.emplace_back(new L1Topo::L1TopoFPGA(fpgaTrailer1, fpgaTrailer2));

	}
	else {
	  if (m_l1topoFPGA.back()->topoNumber() != 1) {
	    i-=3;
	    uint32_t overflowWord = dataWords[--i];
	    uint32_t triggerWord = dataWords[--i];
	    for (size_t iBit=0;iBit<32;iBit++) {
	      uint32_t topo = m_l1topoFPGA.back()->topoNumber();
	      uint32_t fpga = m_l1topoFPGA.back()->fpgaNumber();
	      unsigned int index = L1Topo::triggerBitIndexPhase1(topo, fpga, iBit);
	      m_overflows[index] = (overflowWord>>iBit)&1;
	      m_decisions[index] = (triggerWord>>iBit)&1;
	    }
	  } // Decision & overflow
	  else {
	    if (m_l1topoFPGA.back()->fpgaNumber() == 1) {
	      m_topo1opt3.reset(); m_topo1opt2.reset();
	      for (size_t k=0;k<3;k++) {
		uint32_t word = dataWords[i];
		for (size_t iBit=0;iBit<32;iBit++) {
		  size_t index = iBit+32*(2-k);
		  m_topo1opt3[index] = (word>>iBit)&1;
		}
		i--;
	      }
	      for (size_t k=0;k<3;k++) {
		uint32_t word = dataWords[i];
		for (size_t iBit=0;iBit<32;iBit++) {
		  size_t index = iBit+32*(2-k);
		  m_topo1opt2[index] = (word>>iBit)&1;
		}
		i--;
	      }
	      i++;
	    } // multiplicity 2-3
	    else {
	      m_topo1opt1.reset(); m_topo1opt0.reset();
	      for (size_t k=0;k<3;k++) {
		uint32_t word = dataWords[i];
		for (size_t iBit=0;iBit<32;iBit++) {
		  size_t index = iBit+32*(2-k);
		  m_topo1opt1[index] = (word>>iBit)&1;
		}
		i--;
	      }
	      for (size_t k=0;k<3;k++) {
		uint32_t word = dataWords[i];
		for (size_t iBit=0;iBit<32;iBit++) {
		  size_t index = iBit+32*(2-k);
		  m_topo1opt0[index] = (word>>iBit)&1;
		}
		i--;
	      }
	      i++;
	    
	    } // multiplicity 0-1
	    
	  } // end of multiplicity
	} // end of decoding
      } // end of nwords
    } // end of container

  }
  
} // namespace L1Topo
