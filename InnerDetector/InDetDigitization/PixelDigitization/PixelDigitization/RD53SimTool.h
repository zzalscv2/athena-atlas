/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/RD53SimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief RD53 simulation
 */

#ifndef PIXELDIGITIZATION_RD53SimTool_H
#define PIXELDIGITIZATION_RD53SimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "PixelDigitization/FrontEndSimTool.h"
#include "InDetRawData/PixelRDO_Collection.h" //typedef

class SiChargedDiodeCollection;

namespace CLHEP{
  class HepRandomEngine;
}

class RD53SimTool: public FrontEndSimTool {
public:
  RD53SimTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~RD53SimTool();
  virtual void process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine);
private:
  RD53SimTool();
  Gaudi::Property<bool> m_doTimeWalk {
    this, "DoTimeWalk",false,"include time-walk effects"
      };
  Gaudi::Property<int> m_overDrive {
    this, "OverDrive",150,"value of overdrive (in-time threshold - absolute threshold) in electrons"
      };
};

#endif // PIXELDIGITIZATION_RD53SimTool_H
