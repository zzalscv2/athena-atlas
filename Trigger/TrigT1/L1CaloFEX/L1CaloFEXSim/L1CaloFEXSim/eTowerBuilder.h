/*
    Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ETOWERBUILDER_H
#define ETOWERBUILDER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IeTowerBuilder.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerContainer.h"

class CaloIdManager;

namespace LVL1 {

class eTowerBuilder: public AthAlgTool, virtual public IeTowerBuilder {

 public:
  eTowerBuilder(const std::string& type,const std::string& name,const IInterface* parent);
  virtual ~eTowerBuilder() = default;

  virtual void init(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void execute(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void reset() const override;

 private:

  virtual void BuildEMBeTowers(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void BuildTRANSeTowers(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void BuildEMEeTowers(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void BuildHECeTowers(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void BuildAllTowers(std::unique_ptr<eTowerContainer> & eTowerContainerRaw) const override;
  virtual void BuildSingleTower(std::unique_ptr<eTowerContainer> & eTowerContainerRawRaw,float eta, float phi, float keybase, int posneg) const override;

};

} // end of LVL1 namespace
#endif
