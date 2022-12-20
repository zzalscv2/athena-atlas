
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PFlowByVertexPseudoJetGetter.h

#ifndef JETRECTOOLS_PFLOWBYVERTEXPSEUDOJETGETTER_H
#define JETRECTOOLS_PFLOWBYVERTEXPSEUDOJETGETTER_H

////////////////////////////////////////////
/// \class PFlowByVertexPseudoJetGetter
///
/// PseudoJetGetter for pflow, vertex-by-vertex
///
/// Properties:
///  UseNeutral - If true, the neutral component of pflow is used
///  UseCharged - If true, the charged component of pflow is used
///  UseChargedPV - If true, require charged particles are associated with PV
///  UseChargedPUsideband - If true, require charged particles are in PV sideband
///  UniqueChargedVertex - If true, each charged PFO is only assigned to a single vertex
/// \author S. Schramm
//////////////////////////////////////////////////

#include "JetRec/PseudoJetGetter.h"
#include "AsgTools/ToolHandle.h"

class PFlowByVertexPseudoJetGetter : public PseudoJetGetter {
  ASG_TOOL_CLASS(PFlowByVertexPseudoJetGetter, IPseudoJetGetter)
public:
  PFlowByVertexPseudoJetGetter(const std::string &name);

  virtual int appendTo(PseudoJetVector& psjs, const LabelIndex* pli) const;


protected:

  bool m_useCharged;        /// Flag indicating to use charged particles at all
  bool m_useNeutral;        /// Flag indicating to use neutral particles at all
  bool m_useChargedPV;        /// Flag indicating to use charged particles only from PU z0 window
  bool m_useChargedPUsideband;        /// Flag indicating to use charged particles only from PU z0 sideband
  bool m_uniqueVertex;      /// Flag indicating to only assign a given charged PFO to a single vertex

  virtual jet::IConstituentUserInfo*
  buildCUI(const xAOD::IParticle* ppar, jet::IConstituentUserInfo::Index idx,
           const LabelIndex* pli) const;

private:
    

};

#endif
