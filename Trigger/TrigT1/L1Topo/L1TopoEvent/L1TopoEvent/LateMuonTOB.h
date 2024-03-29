// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#ifndef L1TopoEvent__LateMuonTOB
#define L1TopoEvent__LateMuonTOB

#include <iostream>

#include "L1TopoEvent/BaseTOB.h"
#include "L1TopoEvent/Heap.h"

namespace TCS {
    
   class LateMuonTOB : public BaseTOB {
   public:
      
      static unsigned int nBitsEt() { return g_nBitsEt; }
      static unsigned int nBitsIsolation() { return g_nBitsIsolation; }
      static unsigned int nBitsEta() { return g_nBitsEta; }
      static unsigned int nBitsPhi() { return g_nBitsPhi; }


      // default constructor
      LateMuonTOB(uint32_t roiWord = 0, const std::string& tobName = "LateMuonTOB");
      
      // constructor with individual values
      LateMuonTOB(unsigned int et, unsigned int isolation, int eta, unsigned int phi, uint32_t roiWord = 0, const std::string& tobName = "LateMuonTOB");

      // constructor with initial values
      LateMuonTOB(const LateMuonTOB & latemuon);

      // destructor
      virtual ~LateMuonTOB();

      // accessors
      unsigned int Et() const { return m_Et; }
      unsigned int isolation() const { return m_isolation; }
      int eta() const { return m_eta; }
      unsigned int phi() const { return m_phi; }

      int bw2or3() const { return m_bw2or3; }
      int innerCoin() const { return m_innerCoin; }
      int goodMF() const { return m_goodMF; }
      int charge() const { return m_charge; }
      int is2cand() const { return m_is2cand; }

      double EtDouble() const { return m_EtDouble; }
      double EtaDouble() const { return m_etaDouble; }
      double PhiDouble() const { return m_phiDouble; }
      
      inline bool isTGC() const {return m_isTGC;} // Additonal flag that identifies muons from TGC

      // setters
      void setEt(unsigned int et) { m_Et = sizeCheck(et, nBitsEt()); }
      void setIsolation(unsigned int et) { m_isolation = sizeCheck(et, nBitsIsolation()); }
      void setEta(int eta) { m_eta = sizeCheck(eta, nBitsEta()); }
      void setPhi(unsigned int phi) { m_phi = sizeCheck(phi, nBitsPhi()); }
      
      void setBW2or3(int bw2or3) { m_bw2or3 = bw2or3; }
      void setInnerCoin(int innerCoin) { m_innerCoin = innerCoin; }
      void setGoodMF(int goodMF) { m_goodMF = goodMF; }
      void setCharge(int charge) { m_charge = charge; }
      void setIs2cand(int is2cand) { m_is2cand = is2cand; }
      void setIsTGC(int isTGC) { m_isTGC = isTGC; }

      void setEtDouble(double et) { m_EtDouble = et; }
      void setEtaDouble(double eta) { m_etaDouble = eta; }
      void setPhiDouble(double phi) { m_phiDouble = phi; }

      inputTOBType_t tobType() const { return LATEMUON; }
      
      // memory management
      static LateMuonTOB* createOnHeap(const LateMuonTOB& cl);
      static void clearHeap();
      static const Heap<TCS::LateMuonTOB>& heap() { return fg_heap; }

      virtual void print(std::ostream &o) const;

   private:
      static const unsigned int g_nBitsEt;
      static const unsigned int g_nBitsIsolation;
      static const unsigned int g_nBitsEta;
      static const unsigned int g_nBitsPhi;
      
      unsigned int m_Et {0};
      unsigned int m_isolation {0};
      int m_eta {0};
      unsigned int m_phi {0};

      int m_bw2or3{0};
      int m_innerCoin{0};
      int m_goodMF{0};
      int m_charge{0};
      int m_is2cand{0};
      int m_isTGC{0};

      double m_EtDouble {0};
      double m_etaDouble {0};
      double m_phiDouble {0};

      static thread_local Heap<TCS::LateMuonTOB> fg_heap;
   };
}

#endif
