/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONMONITORING_L2OVERLAPREMOVERMON_H
#define TRIGMUONMONITORING_L2OVERLAPREMOVERMON_H

#include "TrigMuonMonitorAlgorithm.h"
#include "xAODTrigMuon/L2StandAloneMuonContainer.h"

/*
This is a class for monitoring L2MuonSA.
 */
class L2OverlapRemoverMon : public TrigMuonMonitorAlgorithm{

 public:
  L2OverlapRemoverMon(const std::string& name, ISvcLocator* pSvcLocator );

 protected:
  virtual StatusCode fillVariablesPerChain(const EventContext &ctx, const std::string &chain) const override;

 private:

  /**
   * @brief Function that fills variables of L2OverlapRemover plots.
   * @see @c L2OverlapRemoverMon.icc for the implementation
   * @param chain Trigger chain provided in @cfillHistograms
   * @param trigstep trigger step
   */ 
  template<class T>
  StatusCode fillVariablesOverlapRemoverPlots(const std::string &chain, std::string &&trigstep) const;

  bool isOverlap(const std::string &chain, const ElementLink<xAOD::L2StandAloneMuonContainer>& muEL1, const ElementLink<xAOD::L2StandAloneMuonContainer>& muEL2) const;
  bool isOverlap(const std::string &chain, const ElementLink<xAOD::L2CombinedMuonContainer>& muEL1, const ElementLink<xAOD::L2CombinedMuonContainer>& muEL2) const;

  /**
   * @brief Function that choose best muon.
   * @param chain Trigger chain provided in @cfillHistograms
   * @param featureCont vector of LinkInfo
   * @param muResult vector of number assigned to each muon to make overlap judgment
   */ 
  StatusCode chooseBestMuon(const std::string &chain, const std::vector< TrigCompositeUtils::LinkInfo<xAOD::L2StandAloneMuonContainer> >& featureCont, const std::vector<unsigned int>& muResult) const;
  StatusCode chooseBestMuon(const std::string &chain, const std::vector< TrigCompositeUtils::LinkInfo<xAOD::L2CombinedMuonContainer> >& featureCont, const std::vector<unsigned int>& muResult) const;

  static float calcinvMass(double m1, double pt1, double eta1, double phi1, double m2, double pt2, double eta2, double phi2) ;
  static inline std::tuple<float,float,float> L2ORPosForMatchFunc(const xAOD::L2StandAloneMuon *trig);
  static inline std::tuple<float,float,float> L2ORPosForMatchFunc(const xAOD::L2CombinedMuon *trig);

  // these threshold is same as the L2OverlapRemover in TrigMufastHypoTool
  Gaudi::Property< float > m_dRSAThresBB {this, "DRThresBB", 0.05, "DR threshold of L2SA in barel and barel region"};
  Gaudi::Property< float > m_massSAThresBB {this, "MassThresBB", 0.20, "mass threshold of L2SA in barel and barel region"};
  Gaudi::Property< float > m_dRSAThresBE {this, "DRThresBE", 0.05, "DR threshold of L2SA in barel and barel region"};
  Gaudi::Property< float > m_massSAThresBE {this, "MassThresBE", 0.20, "mass threshold of L2SA in barel and endcap region"};
  Gaudi::Property< std::vector<float> > m_etaBins {this, "EtaBins", {0, 1.9, 2.1, 9.9}, "eta bins of DR and mass thresholds"};
  Gaudi::Property< std::vector<float> > m_dRSAThresEC {this, "DRThresEC", {0.06, 0.05, 0.05}, "DR threshold of L2SA in endcap and barel region"};
  Gaudi::Property< std::vector<float> > m_massSAThresEC {this, "MassThresEC", {0.20, 0.15, 0.10}, "mass threshold of L2SA in endcap and endcap region"};
  Gaudi::Property< std::vector<float> > m_dRCBThres {this, "DRThres", {0.06, 0.05, 0.05}, "DR threshold of L2CB"};
  Gaudi::Property< std::vector<float> > m_dRbySAThres {this, "dRbySAThres", {0.06, 0.05, 0.05}, "mufast DR threshold of L2CB"};
  Gaudi::Property< std::vector<float> > m_massCBThres {this, "MassThres", {0.20, 0.15, 0.10}, "mass threshold of L2CB"};

};

#include "L2OverlapRemoverMon.icc"

#endif //TRIGMUONMONITORING_L2OVERLAPREMOVERMON_H
