/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_SUBJETBUILDER_H
#define DITAUREC_SUBJETBUILDER_H

#include "DiTauToolBase.h"

#include "fastjet/tools/Filter.hh"

class SubjetBuilder : public DiTauToolBase {
 public:

  //-------------------------------------------------------------
  //! Constructor
  //-------------------------------------------------------------
  SubjetBuilder(const std::string& type,
		const std::string& name,
		const IInterface * parent);

  //-------------------------------------------------------------
  //! Destructor
  //-------------------------------------------------------------
  virtual ~SubjetBuilder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:

  float m_Rsubjet;
  float m_ptmin;

};

#endif  // DITAUREC_SUBJETBUILDER_H
