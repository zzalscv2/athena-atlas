/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/FEI3SimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief FEI3 simulation
 */

#ifndef PIXELDIGITIZATION_FEI3SimTool_H
#define PIXELDIGITIZATION_FEI3SimTool_H

#include "FrontEndSimTool.h"
#include "InDetRawData/PixelRDO_Collection.h" //typedef

class SiChargedDiodeCollection;
class SiTotalCharge;
class PixelModuleData;

namespace CLHEP{
  class HepRandomEngine;
}

class FEI3SimTool: public FrontEndSimTool {
public:
  FEI3SimTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~FEI3SimTool();
  virtual void process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine);
private:
  FEI3SimTool();

  int relativeBunch2009(const double threshold, const double intimethreshold, const SiTotalCharge& totalCharge,
                        CLHEP::HepRandomEngine* rndmEngine) const;
  int relativeBunch2015(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
                        CLHEP::HepRandomEngine* rndmEngine) const;
  int relativeBunch2018(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
                        CLHEP::HepRandomEngine* rndmEngine) const;
  int relativeBunch2022(const SiTotalCharge& totalCharge, const double tot, int barrel_ec, int layer_disk, int moduleID, 
                        CLHEP::HepRandomEngine* rndmEngine) const;
};

#endif // PIXELDIGITIZATION_FEI3SimTool_H
