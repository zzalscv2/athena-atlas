// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#ifndef GENERICTOB_H
#define GENERICTOB_H

#include <iostream>
#include <string>

#include "L1TopoEvent/BaseTOB.h"
#include "L1TopoEvent/JetTOB.h"
#include "L1TopoEvent/gJetTOB.h"
#include "L1TopoEvent/jTauTOB.h"
#include "L1TopoEvent/eTauTOB.h"
#include "L1TopoEvent/jLargeRJetTOB.h"
#include "L1TopoEvent/gLargeRJetTOB.h"
#include "L1TopoEvent/jJetTOB.h"
#include "L1TopoEvent/ClusterTOB.h"
#include "L1TopoEvent/eEmTOB.h"
#include "L1TopoEvent/jEmTOB.h"
#include "L1TopoEvent/cTauTOB.h"
#include "L1TopoEvent/MuonTOB.h"
#include "L1TopoEvent/LateMuonTOB.h"
#include "L1TopoEvent/MuonNextBCTOB.h"
#include "L1TopoEvent/MetTOB.h"


// TODO implement sizecheck lile in ClusterTOB

namespace TCS {
   
   class GenericTOB : public BaseTOB {
   public:

      // default constructor
      GenericTOB(uint32_t roiWord = 0);

      // constructor from individual values
      GenericTOB(unsigned int Et, int eta, int phi, uint32_t roiWord = 0);

      // copy constructor
      GenericTOB(const GenericTOB & other);

      // constructor from jet
      GenericTOB(const JetTOB & jet, JetTOB::JetSize jetSize);

      // constructor from jtaus
      GenericTOB(const jTauTOB & tau);

      // constructor from jEm
      GenericTOB(const jEmTOB & jem);

      // constructor from large r jet
      GenericTOB(const jLargeRJetTOB & jet);

      // constructor from large r gjet
      GenericTOB(const gLargeRJetTOB & jet);

      // constructor from small r jet
      GenericTOB(const jJetTOB & jet);

      // constructor from small r gjet
      GenericTOB(const gJetTOB & jet);

      // constructor from cluster
      GenericTOB(const ClusterTOB & cluster);

      // constructor from eEm
      GenericTOB(const eEmTOB & eem);

      // constructor from eTau
      GenericTOB(const eTauTOB & etau);

      // constructor from eTau
      GenericTOB(const cTauTOB & ctau);

      // constructor from muon
      GenericTOB(const MuonTOB & muon);
      
      // constructor from lateMuon
      GenericTOB(const LateMuonTOB & lateMuon);

      // constructor from muonNextBC
      GenericTOB(const MuonNextBCTOB & muonNextBC);

      // constructor from met
      GenericTOB(const MetTOB & met);

      // destructor
      ~GenericTOB();

      static GenericTOB* createOnHeap(const GenericTOB &);
      static void clearHeap();

      static const Heap<TCS::GenericTOB>& heap() { return fg_heap; }

   public:
      unsigned int Et() const { return m_Et; }
      unsigned int EtWide() const { return m_EtWide; }
      unsigned int EtNarrow() const { return m_EtNarrow; }

      int Ex() const { return m_Ex; }
      int Ey() const { return m_Ey; }

      int eta() const { return m_eta; }
      int phi() const { return m_phi; }

      //eEm
      unsigned int Reta() const { return m_reta; }
      unsigned int Rhad() const { return m_rhad; }
      unsigned int Wstot() const { return m_wstot; }

      //eTau
      unsigned int rcore() const { return m_rcore; }

      // See definitions at TrigT1Interfaces/MuCTPIL1TopoCandidate.h 
      int bw2or3() const { return m_bw2or3; }
      int innerCoin() const { return m_innerCoin; }
      int goodMF() const { return m_goodMF; }
      int charge() const { return m_charge; }
      int is2cand() const { return m_is2cand; }
      
      double EtDouble() const { return m_EtDouble; }
      double etaDouble() const { return m_etaDouble; }
      double phiDouble() const { return m_phiDouble; }

      //jtau isolation 
      unsigned int jtauiso() const { return m_jtauiso; }

      virtual void print(std::ostream &o) const;

      void setTobType(inputTOBType_t tobType) { m_tobType = tobType; }

      inputTOBType_t tobType() const { return m_tobType; }

   private:
      unsigned int m_Et { 0 };
      unsigned int m_EtNarrow { 0 };
      unsigned int m_EtWide { 0 };

      int m_Ex { 0 };
      int m_Ey { 0 };

      int m_eta { 0 };
      int m_phi { 0 };

      int m_bw2or3 { 0 };
      int m_innerCoin { 0 };
      int m_goodMF { 0 };
      int m_charge { 0 };
      int m_is2cand { 0 };

      double m_EtDouble { 0 };
      double m_etaDouble { 0 };
      double m_phiDouble { 0 };

      unsigned int m_reta {0};
      unsigned int m_rhad {0};
      unsigned int m_wstot {0};

      unsigned int m_jtauiso {0};
      unsigned int m_rcore {0};
      
      inputTOBType_t   m_tobType { NONE };

      static thread_local  Heap<TCS::GenericTOB> fg_heap;
   };  
}

#endif
