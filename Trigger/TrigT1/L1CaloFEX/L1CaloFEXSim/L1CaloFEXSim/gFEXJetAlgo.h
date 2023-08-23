/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJetAlgo - JetFinder algorithm for gFEX
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXJetAlgo_H
#define gFEXJetAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXJetAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/gFEXJetTOB.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"



namespace LVL1 {

  typedef  std::array<std::array<int, 6>, 32> gTowersJetEngine;
  typedef  std::array<std::array<int, 4>, 32> gTowersPartialSums;

  class gFEXJetAlgo : public AthAlgTool, virtual public IgFEXJetAlgo {

  public:
    /** Constructors */
    gFEXJetAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    virtual void pileUpCalculation(gTowersCentral &twrs, int rhoThreshold_Max, int rhoThreshold_Min, int inputScale,  int &PUCp) const override;
    
    virtual std::vector<std::unique_ptr<gFEXJetTOB>> largeRfinder(const gTowersCentral& Atwr, const gTowersCentral& Btwr,
                                                                  const gTowersForward& CNtwr, const gTowersForward& CPtwr,
                                                                  int pucA, int pucB, int gLJ_seedThrA, int gLJ_seedThrB, 
                                                                  int gJ_ptMinToTopoCounts1, int gJ_ptMinToTopoCounts2, 
                                                                  int jetThreshold, int gLJ_ptMinToTopoCounts1, int gLJ_ptMinToTopoCounts2,
                                                                  std::array<uint32_t, 7> & ATOB1_dat, std::array<uint32_t, 7> & ATOB2_dat,
                                                                  std::array<uint32_t, 7> & BTOB1_dat, std::array<uint32_t, 7> & BTOB2_dat) const override;

  private:

    virtual void RemotePartialAB(const gTowersCentral& twrs, gTowersPartialSums & lps, gTowersPartialSums & rps) const;

    virtual void RemotePartialCN(const gTowersForward& twrs, gTowersPartialSums & rps) const;

    virtual void RemotePartialCP(const gTowersForward& twrs, gTowersPartialSums & lps) const;

    virtual void singleAB(const gTowersCentral& twrs, gTowersCentral & FPGAsum) const;

    virtual void gBlockAB(const gTowersCentral& twrs, gTowersCentral & gBlkSum) const;

    virtual void gBlockMax2(const gTowersCentral& gBlkSum, int BjetColumn, int localColumn, std::array<int, 3> & gBlockV, std::array<int, 3> & gBlockEta, std::array<int, 3> & gBlockPhi) const;

    virtual void gBlockMax192(const gTowersJetEngine& gBlkSum, std::array<int, 3> & gBlockVp, std::array<int, 3> & gBlockEtap, std::array<int, 3> & gBlockPhip, int index) const;

    virtual void addRemoteRin(gTowersCentral &jets, const gTowersPartialSums &partial) const;

    virtual void addRemoteLin(gTowersCentral &jets, const gTowersPartialSums &partial) const;


    virtual void pileUpCorrectionAB(gTowersCentral &jets, int puc) const;

    virtual void gJetVetoAB( gTowersCentral &twrs ,int jet_threshold ) const;

    virtual void gBlockVetoAB(gTowersCentral &twrs, const gTowersCentral& blocks, int seed_threshold) const;

    virtual void jetOutAB(const gTowersCentral& jets, const gTowersCentral& blocks, int seedThreshold,
                           std::array<int, 32> &jetOutL, std::array<int, 32> &etaIndL,
                           std::array<int, 32> &jetOutR, std::array<int, 32> &etaIndR) const;

    virtual void gJetTOBgen(const std::array<int, FEXAlgoSpaceDefs::ABCrows>& jetOut,
			     const std::array<int, FEXAlgoSpaceDefs::ABCrows>& etaInd,
                             int TOBnum, int jetThreshold, std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBs,
                             std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBv,
                             std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBeta,
                             std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBphi ) const;

    SG::ReadHandleKey<LVL1::gTowerContainer> m_gFEXJetAlgo_gTowerContainerKey {this, "MyGTowers", "gTowerContainer", "Input container for gTowers"};
    
  };

} // end of namespace


#endif
