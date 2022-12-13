/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TOPORESULT_H
#define L1TOPORESULT_H

#include "L1TopoRDO/L1TopoFPGA.h"
#include "L1TopoRDO/L1TopoROD.h"

//xAOD
#include "xAODTrigL1Calo/L1TopoRawData.h"

//Athena
#include "AthContainers/DataVector.h"

// System includes
#include <vector>
#include <bitset>

/*
 * This class is for decoding phase1 L1Topo hardware results.
 *
 * @author Anil Sonay
 */

namespace L1Topo {
  
  class L1TopoResult {

  private:
    /// Number of topo outputs
    static constexpr size_t s_nTopoOutputs{128};

  public:
    
    L1TopoResult();
    
    L1TopoResult(const DataVector<xAOD::L1TopoRawData> &container);
        
    ~L1TopoResult();

    const std::shared_ptr<L1Topo::L1TopoROD> &getROD(unsigned index) const {return m_l1topoROD[index];}
    const std::shared_ptr<L1Topo::L1TopoFPGA> &getFPGA(unsigned index) const {return m_l1topoFPGA[index];}
    const std::bitset<s_nTopoOutputs> &getDecisions() const {return m_decisions;}
    const std::bitset<s_nTopoOutputs> &getOverflows() const {return m_overflows;}
    const std::bitset<s_nTopoOutputs> &getTopo1Opt0() const {return m_topo1opt0;}
    const std::bitset<s_nTopoOutputs> &getTopo1Opt1() const {return m_topo1opt1;}
    const std::bitset<s_nTopoOutputs> &getTopo1Opt2() const {return m_topo1opt2;}
    const std::bitset<s_nTopoOutputs> &getTopo1Opt3() const {return m_topo1opt3;}
    bool getStatus() {return m_status;}
    unsigned getRODSize() {return m_l1topoROD.size();}
    unsigned getFPGASize() {return m_l1topoFPGA.size();}

    //! method used by constructor to decode xAOD 
    void decode(const DataVector<xAOD::L1TopoRawData> &container);

  private:

    std::vector<std::shared_ptr<L1Topo::L1TopoROD>> m_l1topoROD;
    std::vector<std::shared_ptr<L1Topo::L1TopoFPGA>> m_l1topoFPGA;
    std::bitset<s_nTopoOutputs> m_decisions;
    std::bitset<s_nTopoOutputs> m_overflows;
    std::bitset<s_nTopoOutputs> m_topo1opt0;
    std::bitset<s_nTopoOutputs> m_topo1opt1;
    std::bitset<s_nTopoOutputs> m_topo1opt2;
    std::bitset<s_nTopoOutputs> m_topo1opt3;

    bool m_status;
    
    
  };
} // namespace L1Topo

#endif // L1TOPORESULT_H
