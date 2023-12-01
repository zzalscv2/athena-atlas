/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "RD53SimTool.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibParameters.h" //for Thresholds
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetRawData/PixelRDO_Collection.h"

#include "SiDigitization/SiHelper.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"
#include "InDetRawData/Pixel1RawData.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

RD53SimTool::RD53SimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  FrontEndSimTool(type, name, parent) {
}

RD53SimTool::~RD53SimTool() = default;

StatusCode RD53SimTool::initialize() {
  CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("RD53SimTool::initialize()");

  return StatusCode::SUCCESS;
}

StatusCode RD53SimTool::finalize() {
  ATH_MSG_DEBUG("RD53SimTool::finalize()");
  return StatusCode::SUCCESS;
}

void RD53SimTool::process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                          CLHEP::HepRandomEngine* rndmEngine) {
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  if (p_design->getReadoutTechnology() != InDetDD::PixelReadoutTechnology::RD53) {
    return;
  }

  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  Identifier moduleID = pixelId->wafer_id(chargedDiodes.element()->identify());

  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());

  if (abs(barrel_ec) != m_BarrelEC) {
    return;
  }

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<PixelModuleData> moduleDataHandle(m_moduleDataKey, ctx);
  const PixelModuleData *moduleData = *moduleDataHandle;
  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;

  int overflowToT = calibData->getFEI4OverflowToT();

  std::vector<Pixel1RawData*> p_rdo_small_fei4;
  std::vector<int> row, col;
  

  // Add cross-talk
  crossTalk(moduleData->getCrossTalk(barrel_ec, layerIndex), chargedDiodes);

  if (m_doNoise) {
    // Add thermal noise
    thermalNoise(m_thermalNoise, chargedDiodes, rndmEngine);

    // Add random noise
    randomNoise(chargedDiodes, moduleData, calibData, rndmEngine);
  }

  // Add random diabled pixels
  randomDisable(chargedDiodes, moduleData, rndmEngine); // FIXME How should we handle disabling pixels in Overlay jobs?

  for (SiChargedDiodeIterator i_chargedDiode = chargedDiodes.begin(); i_chargedDiode != chargedDiodes.end();
       ++i_chargedDiode) {
    Identifier diodeID = chargedDiodes.getId((*i_chargedDiode).first);
    double charge = (*i_chargedDiode).second.charge();

    unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);

    // Apply analogue threshold, timing simulation
    const auto thresholds = calibData->getThresholds(type, moduleHash, FE);
    const int th0 = thresholds.value;
    const int sigma = thresholds.sigma;
    const int noise = thresholds.noise;
    double threshold = th0 +
                       sigma * CLHEP::RandGaussZiggurat::shoot(rndmEngine) +
                       noise * CLHEP::RandGaussZiggurat::shoot(rndmEngine); // This noise check is unaffected by digitizationFlags.doInDetNoise in 21.0 - see PixelCellDiscriminator.cxx in that branch

    if (charge > threshold) {
      int bunchSim = 0;
      if ((*i_chargedDiode).second.totalCharge().fromTrack()) {
        bunchSim =
          static_cast<int>(floor((getG4Time((*i_chargedDiode).second.totalCharge()) +
                                  m_timeOffset) / m_bunchSpace));
	
	//Timewalk implementation 
	if(m_doTimeWalk){
	  if(charge < (threshold + m_overDrive)){
	    const int timeWalk = 25; // Here it is assumed that the maximum value of timewalk is one bunch crossing (25ns)
	    bunchSim =
	      static_cast<int>(floor((getG4Time((*i_chargedDiode).second.totalCharge()) +
				      m_timeOffset+ timeWalk) / m_bunchSpace));
	  } 
	}
      } else {
        bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, m_numberOfBcid);
      }

      if (bunchSim < 0 || bunchSim > m_numberOfBcid) {
        SiHelper::belowThreshold((*i_chargedDiode).second, true, true);
      } else {
        SiHelper::SetBunch((*i_chargedDiode).second, bunchSim);
      }
    } else {
      SiHelper::belowThreshold((*i_chargedDiode).second, true, true);
    }

    // charge to ToT conversion
    double tot = calibData->getToT(type, moduleHash, FE, charge);
    double totsig = calibData->getTotRes(moduleHash, FE, tot);
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot, totsig));

    if (nToT < 1) {
      nToT = 1;
    }

    // RD53 HitDiscConfig
    if (nToT >= overflowToT) {
      nToT = overflowToT;
    }

    if (nToT <= moduleData->getToTThreshold(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold((*i_chargedDiode).second, true, true);
    }

    // Filter events
    if (SiHelper::isMaskOut((*i_chargedDiode).second)) {
      continue;
    }
    if (SiHelper::isDisabled((*i_chargedDiode).second)) {
      continue;
    }

    if (!m_pixelConditionsTool->isActive(moduleHash, diodeID, ctx)) {
      SiHelper::disabled((*i_chargedDiode).second, true, true);
      continue;
    }

    int flag = (*i_chargedDiode).second.flag();
    int bunch = (flag >> 8) & 0xff;

    InDetDD::SiReadoutCellId cellId = (*i_chargedDiode).second.getReadoutCell();
    const Identifier id_readout = chargedDiodes.element()->identifierFromCellId(cellId);

    // Front-End simulation
    if (bunch >= 0 && bunch < m_numberOfBcid) {
      Pixel1RawData* p_rdo = new Pixel1RawData(id_readout, nToT, bunch, 0, bunch);
      rdoCollection.push_back(p_rdo);
      p_rdo = nullptr;
    }
  }
}
