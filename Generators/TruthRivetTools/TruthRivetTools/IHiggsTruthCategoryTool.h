/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Dual-use tool interface for Rivet routine for classifying MC events according to the Higgs template cross section categories
 * Authors: Jim Lacey (Carleton University)
 * <james.lacey@cern.ch,jlacey@physics.carleton.ca>
 */

#ifndef TRUTHRIVETTOOLS_IHIGGSTRUTHCATEGORYTOOL_H
#define TRUTHRIVETTOOLS_IHIGGSTRUTHCATEGORYTOOL_H 1

#include "AsgTools/IAsgTool.h"
#include "TLorentzVector.h"
#include "HiggsTemplateCrossSections.h"
#include "HepMC/GenEvent.h"

class IHiggsTruthCategoryTool : public virtual asg::IAsgTool {
 public:
  ASG_TOOL_INTERFACE( IHiggsTruthCategoryTool ) //declares the interface to athena
    virtual ~IHiggsTruthCategoryTool() {};
 public:
  virtual StatusCode initialize() = 0;
  virtual StatusCode finalize () = 0;  
  virtual HTXS::HiggsClassification getHiggsTruthCategoryObject(const HepMC::GenEvent& HepMCEvent, const HTXS::HiggsProdMode prodMode)=0;
};

#endif //> !HIGGSTRUTHCLASSIFIER_IHIGGSTRUTHCATEGORYTOOL_H
