/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulation_MuonInputProvider
#define L1TopoSimulation_MuonInputProvider

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/LockedHandle.h"
#include "TrigT1Interfaces/MuCTPIL1Topo.h"
#include "TrigT1Result/RoIBResult.h"
#include "TrigT1Interfaces/MuCTPIToRoIBSLink.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"
#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"

#include "xAODTrigger/MuonRoI.h"

#include <vector>

namespace TrigConf
{
   class L1Menu;
} // namespace TrigConf

namespace TCS {
   class MuonTOB;
   class LateMuonTOB;
}

namespace LVL1 {

   class MuCTPIL1TopoCandidate;

   class MuonInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      MuonInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual StatusCode initialize() override;

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const override;

   private:
      TCS::MuonTOB createMuonTOB(const xAOD::MuonRoI & muonRoI, const std::vector<unsigned int> & rpcPtValues, const std::vector<unsigned int> & tgcPtValues) const;
      TCS::MuonTOB createMuonTOB(uint32_t roiword, const TrigConf::L1Menu *l1menu) const;
      TCS::MuonTOB createMuonTOB(const MuCTPIL1TopoCandidate & roi) const;
      TCS::LateMuonTOB createLateMuonTOB(const MuCTPIL1TopoCandidate & roi) const;
      /**
         @brief convert the 2-bit value from MuCTPIL1TopoCandidate::getptL1TopoCode() to an actual pt

         The muon TOB encodes pt values in 2 bits.
         A MuCTPIL1TopoCandidate provides the encoded 2-bit value with
         the function getptL1TopoCode().
         This function uses the information from the l1 trigger menu
         configuration to convert the threshold to an actual pt value.
         For more details, see ATR-16781.
      */
      unsigned int topoMuonPtThreshold(const MuCTPIL1TopoCandidate &mctpiCand) const;
      /* 
         @brief calculate the eta and phi L1Topo indices

         The exact eta and phi coordinates are rounded according to a particular L1Topo granularity
         Using product instead of division avoids unexpected rounding errors due to precision
         Also, LUTs for the firmware are built using Python 3.x numpy.round(), which is different from std::round()
         Input: x = eta/phi float values, g = inverse of eta/phi granularity
         Output: integer eta/phi L1Topo coordinates
      */
      int topoIndex(float x, int g) const;
      /* 
         @brief use L1Topo convention for muon flags (1 = true/positive, -1 = false/negative, 0 = undefined)
      */
      int topoFlag(bool flag) const;

      ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_recRPCRoiTool{this, "RecRpcRoiTool", "LVL1::TrigT1RPCRecRoiTool/TrigT1RPCRecRoiTool", "RPC RoI reconstruction tool"};
      ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_recTGCRoiTool{this, "RecTgcRoiTool", "LVL1::TrigT1TGCRecRoiTool/TrigT1TGCRecRoiTool", "TGC RoI reconstruction tool"};
      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey<L1MUINT::MuCTPIToRoIBSLink> m_muonROILocation { this, "MuonROILocation", LVL1MUCTPI::DEFAULT_MuonRoIBLocation, "Storegate key for the Muon ROIs" };
      SG::ReadHandleKey<ROIB::RoIBResult> m_roibLocation{ this, "ROIBResultLocation", ROIB::DEFAULT_RoIBRDOLocation, "Storegate key for the reading the ROIBResult" };
      SG::ReadHandleKey<LVL1::MuCTPIL1Topo> m_MuCTPItoL1TopoLocation { this, "locationMuCTPItoL1Topo", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation, "Storegate key for MuCTPItoL1Topo "};
      SG::ReadHandleKey<LVL1::MuCTPIL1Topo> m_MuCTPItoL1TopoLocationPlusOne { this, "locationMuCTPItoL1Topo1", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation, "Storegate key for MuCTPItoL1TopoPlusOne"};
      Gaudi::Property<uint16_t> m_MuonEncoding {this, "MuonEncoding", 0, "0=full granularity Mu ROIs, 1=MuCTPiToTopo granularity"};
      Gaudi::Property<std::string> m_MuonL1RoIKey {this, "MuonL1RoIKey", "", "Empty=Use Muctpi, LVL1MuonRoIs=Use reading from xAOD L1 RoI"};
 
   };
}

#endif
