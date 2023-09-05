/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELDIGITIZATION_FrontEndSimTool_H
#define PIXELDIGITIZATION_FrontEndSimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
//
//for ToolHandles, ServiceHandles:
#include "InDetConditionsSummaryService/IInDetConditionsTool.h" //ToolHandle template param
#include "PixelReadoutGeometry/IPixelReadoutManager.h"  //SvcHandle template param
#include "PixelConditionsData/PixelModuleData.h"  //ReadCondHandleKey template param
#include "PixelConditionsData/PixelChargeCalibCondData.h"  //ReadCondHandleKey template param
#include "InDetRawData/PixelRDO_Collection.h" //typedef



class SiChargedDiodeCollection;
class SiTotalCharge;
//
namespace CLHEP{
  class HepRandomEngine;
}

static const InterfaceID IID_IFrontEndSimTool("FrontEndSimTool", 1, 0);

class FrontEndSimTool: public AthAlgTool, virtual public IAlgTool {
public:
  FrontEndSimTool(const std::string& type, const std::string& name, const IInterface* parent);
  
  static const InterfaceID& interfaceID() {return IID_IFrontEndSimTool;}

  virtual StatusCode initialize() override;
  
  virtual StatusCode finalize() override;
  virtual ~FrontEndSimTool() {}
  //
  virtual void process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine) = 0;

  void crossTalk(double crossTalk, SiChargedDiodeCollection& chargedDiodes) const;
  
  void thermalNoise(double thermalNoise, SiChargedDiodeCollection& chargedDiodes,
                    CLHEP::HepRandomEngine* rndmEngine) const;
                    
  void randomNoise(SiChargedDiodeCollection& chargedDiodes,
                   const PixelModuleData *moduleData,
                   const PixelChargeCalibCondData *chargeCalibData,
                   CLHEP::HepRandomEngine* rndmEngine) const;
  

  void randomDisable(SiChargedDiodeCollection& chargedDiodes,
                     const PixelModuleData *moduleData,
                     CLHEP::HepRandomEngine* rndmEngine) const;
 

private:
  FrontEndSimTool();
protected:
  static constexpr double m_bunchSpace{25.0};
  int m_numberOfBcid{1}; //assumed same for all positions
  ToolHandle<IInDetConditionsTool> m_pixelConditionsTool{
    this, "PixelConditionsSummaryTool", "PixelConditionsSummaryTool", "Tool to retrieve Pixel Conditions summary"
  };

  ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout{
    this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager"
  };

  SG::ReadCondHandleKey<PixelModuleData> m_moduleDataKey{
    this, "PixelModuleData", "PixelModuleData", "Pixel module data"
  };

  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey{
    this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"
  };

  Gaudi::Property<int> m_BarrelEC{
    this, "BarrelEC", 0, "Index of barrel or endcap"
  };

  Gaudi::Property<bool> m_doNoise{
    this, "DoNoise", true, "Flag of noise simulation"
  };
  
  Gaudi::Property<bool> m_cosmics{
    this, "Cosmics", false, "Is this for Cosmics simulation?"
  };

  double getG4Time(const SiTotalCharge& totalCharge) const;
};

#endif // PIXELDIGITIZATION_FrontEndSimTool_H
