/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_MUENTRYVARIABLES_H
#define MUONPRDTEST_MUENTRYVARIABLES_H


#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
namespace MuonPRDTest{
  class MuEntryVariables : public PrdTesterModule {
  public:
      MuEntryVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
      bool fill(const EventContext& ctx) override final;
      bool declare_keys() override final;


  private:
        SG::ReadHandleKey<TrackRecordCollection> m_key{};
        ScalarBranch<unsigned int>& m_MuEntry_nParticles{parent().newScalar<unsigned int>("Muontry_numPart")};
        PtEtaPhiEBranch m_MuEntry_mom{parent(), "MuEntry_Particle"};
        ThreeVectorBranch m_MuEntry_pos{parent(), "MuEntry_Position"};
        VectorBranch<int>& m_MuEntry_particlePdg_id{parent().newVector<int>("MuEntry_Pdg")};
        VectorBranch<int>& m_MuEntry_particleBarcode{parent().newVector<int>("MuEntry_Barcode")};
  };
}
#endif // MUENTRYVARIABLES_H
