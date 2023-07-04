/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUAODSELECTOR_H
#define TAURECTOOLS_TAUAODSELECTOR_H

#include "tauRecTools/TauRecToolBase.h"

class TauAODSelector : public TauRecToolBase {
 
public:
  
  ASG_TOOL_CLASS2( TauAODSelector, TauRecToolBase, ITauToolBase )
  
  TauAODSelector(const std::string& name="TauAODSelector");
  
  virtual ~TauAODSelector() = default;
  
  virtual StatusCode execute(xAOD::TauJet& tau) const override;

private:

  // minimum tau pt below which taus are not written to AOD
  double m_min0pTauPt;
  double m_minTauPt;
  bool m_doEarlyStopping;

};

#endif // TAURECTOOLS_TAUAODSELECTOR_H
