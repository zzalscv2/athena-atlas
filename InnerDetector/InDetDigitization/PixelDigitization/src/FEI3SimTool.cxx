/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "FEI3SimTool.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

FEI3SimTool::FEI3SimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  FrontEndSimTool(type, name, parent) {
}

FEI3SimTool::~FEI3SimTool() = default;

StatusCode FEI3SimTool::initialize() {
  CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("FEI3SimTool::initialize()");
  return StatusCode::SUCCESS;
}

StatusCode FEI3SimTool::finalize() {
  ATH_MSG_DEBUG("FEI3SimTool::finalize()");
  return StatusCode::SUCCESS;
}

void FEI3SimTool::process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                          CLHEP::HepRandomEngine* rndmEngine) {
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  if (p_design->getReadoutTechnology() != InDetDD::PixelReadoutTechnology::FEI3) {
    return;
  }

  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  Identifier moduleID = pixelId->wafer_id(chargedDiodes.element()->identify());

  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
  int moduleIndex = pixelId->eta_module(chargedDiodes.element()->identify());

  if (abs(barrel_ec) != m_BarrelEC) {
    return;
  }

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<PixelModuleData> moduleDataHandle(m_moduleDataKey, ctx);
  const PixelModuleData *moduleData = *moduleDataHandle;
  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;

  // Add cross-talk
  CrossTalk(moduleData->getCrossTalk(barrel_ec, layerIndex), chargedDiodes);

  if (m_doNoise) {
    // Add thermal noise
    ThermalNoise(moduleData->getThermalNoise(barrel_ec, layerIndex), chargedDiodes, rndmEngine);

    // Add random noise
    RandomNoise(chargedDiodes, moduleData, calibData, rndmEngine);
  }

  // Add random diabled pixels
  RandomDisable(chargedDiodes, moduleData, rndmEngine); // FIXME How should we handle disabling pixels in Overlay jobs?

  for (SiChargedDiodeIterator i_chargedDiode = chargedDiodes.begin(); i_chargedDiode != chargedDiodes.end();
       ++i_chargedDiode) {
    // Merge ganged pixel
    InDetDD::SiCellId cellID = chargedDiodes.element()->cellIdFromIdentifier(chargedDiodes.getId(
                                                                               (*i_chargedDiode).first));
    const InDetDD::SiDetectorElement * siDetEl = static_cast<const InDetDD::SiDetectorElement *>(chargedDiodes.element());
    InDetDD::SiCellId gangedCell = siDetEl->gangedCell(cellID);
    Identifier gangedID = chargedDiodes.element()->identifierFromCellId(gangedCell);
    if (gangedCell.isValid()) {
      SiChargedDiode* gangedChargeDiode = chargedDiodes.find(gangedID);
      int phiGanged = pixelId->phi_index(gangedID);
      int phiThis = pixelId->phi_index(chargedDiodes.getId((*i_chargedDiode).first));

      if (gangedChargeDiode) { // merge charges
        bool maskGanged = ((phiGanged > 159) && (phiGanged < 168));
        bool maskThis = ((phiThis > 159) && (phiThis < 168));
        // mask the one ganged pixel that does not correspond to the readout electronics.
        // not really sure this is needed
        if (maskGanged && maskThis) {
          ATH_MSG_ERROR("FEI3SimTool: both ganged pixels are in the mask out region -> BUG!");
        }
        if (maskGanged) {
          (*i_chargedDiode).second.add(gangedChargeDiode->totalCharge()); // merged org pixel
          SiHelper::maskOut(*gangedChargeDiode, true);
        } else {
          gangedChargeDiode->add((*i_chargedDiode).second.totalCharge()); // merged org pixel
          SiHelper::maskOut((*i_chargedDiode).second, true);
        }
      }
    }
  }

  for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
       i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
    SiChargedDiode& diode = **i_chargedDiode;

    Identifier diodeID = chargedDiodes.getId(diode.diode());
    double charge = diode.charge();

    unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);

    // charge to ToT conversion
    double tot = calibData->getToT(type, moduleHash, FE, charge);

    // Apply analog threshold, timing simulation
    double th0 = calibData->getAnalogThreshold(type, moduleHash, FE);
    double ith0 = calibData->getInTimeThreshold(type, moduleHash, FE);

    double thrand1 = CLHEP::RandGaussZiggurat::shoot(rndmEngine);
    double thrand2 = CLHEP::RandGaussZiggurat::shoot(rndmEngine);
    double threshold = th0
                       + calibData->getAnalogThresholdSigma(type, moduleHash, FE) * thrand1
                       + calibData->getAnalogThresholdNoise(type, moduleHash, FE) * thrand2;
                       // This noise check is unaffected by digitizationFlags.doInDetNoise in
                       // 21.0 - see PixelCellDiscriminator.cxx in that branch
    double intimethreshold = (ith0 / th0) * threshold;

    if (charge > threshold) {
      int bunchSim = 0;
      if (diode.totalCharge().fromTrack()) {
        if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) == 2022) {
          bunchSim = relativeBunch2022(diode.totalCharge(), tot, barrel_ec, layerIndex, moduleIndex, moduleData, rndmEngine);
        } 
        else if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) == 2018) {
          bunchSim = relativeBunch2018(diode.totalCharge(), barrel_ec, layerIndex, moduleIndex, moduleData, rndmEngine);
        } 
        else if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) == 2015) {
          bunchSim = relativeBunch2015(diode.totalCharge(), barrel_ec, layerIndex, moduleIndex, moduleData, rndmEngine);
        } 
        else if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) == 2009) {
          bunchSim = relativeBunch2009(threshold, intimethreshold, diode.totalCharge(), moduleData, rndmEngine);
        }
      } 
      else {
        if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) > 0) {
          bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, moduleData->getNumberOfBCID(barrel_ec, layerIndex));
        }
      }

      if (bunchSim < 0 || bunchSim > moduleData->getNumberOfBCID(barrel_ec, layerIndex)) {
        SiHelper::belowThreshold(diode, true, true);
      } else {
        SiHelper::SetBunch(diode, bunchSim);
      }
    } else {
      SiHelper::belowThreshold(diode, true, true);
    }

    double totsig = calibData->getTotRes(moduleHash, FE, tot);
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot, totsig));

    if (nToT < 1) {
      nToT = 1;
    }

    if (nToT <= moduleData->getToTThreshold(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold(diode, true, true);
    }

    if (nToT >= moduleData->getFEI3Latency(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold(diode, true, true);
    }

    // Filter events
    if (SiHelper::isMaskOut(diode)) {
      continue;
    }
    if (SiHelper::isDisabled(diode)) {
      continue;
    }

    if (!m_pixelConditionsTool->isActive(moduleHash, diodeID, ctx)) {
      SiHelper::disabled(diode, true, true);
      continue;
    }

    int flag = diode.flag();
    int bunch = (flag >> 8) & 0xff;

    InDetDD::SiReadoutCellId cellId = diode.getReadoutCell();
    const Identifier id_readout = chargedDiodes.element()->identifierFromCellId(cellId);

    // Front-End simulation
    if (bunch >= 0 && bunch < moduleData->getNumberOfBCID(barrel_ec, layerIndex)) {
      rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch, 0, bunch));
    }

    // Duplication mechanism for FEI3 small hits :
    if (moduleData->getFEI3HitDuplication(barrel_ec, layerIndex)) {
      bool smallHitChk = false;
      if (nToT <= moduleData->getFEI3SmallHitToT(barrel_ec, layerIndex)) {
        smallHitChk = true;
      }

      if (smallHitChk && bunch > 0 && bunch <= moduleData->getNumberOfBCID(barrel_ec, layerIndex)) {
        rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch - 1, 0, bunch - 1));
      }
    }
  }
  }

int FEI3SimTool::relativeBunch2009(const double threshold, const double intimethreshold,
                                   const SiTotalCharge& totalCharge,
                                   const PixelModuleData* moduleData,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  int BCID = 0;
  double myTimeWalkEff = 0.;
  double overdrive = intimethreshold - threshold;

  //my TimeWalk computation through PARAMETRIZATION (by Francesco De Lorenzi - Milan)
  //double curvature  =  7.6e7*overdrive-2.64e10;
  //double divergence = -1.6*overdrive+942 ;
  //double myTimeWalk    = curvature/(pow((totalCharge.charge()-divergence),2.5));

  //my TimeWalk computation through PARAMETRIZATION from 2009 cosmic data (by I. Ibragimov and D. Miller)
  double p1 = 20. / log(intimethreshold / overdrive);
  double p0 = p1 * log(1. - threshold / 100000.);

  double myTimeWalk = -p0 - p1 * log(1. - threshold / totalCharge.charge());

  myTimeWalkEff = myTimeWalk + myTimeWalk * 0.2 * CLHEP::RandGaussZiggurat::shoot(rndmEngine);

  double randomJitter =
    CLHEP::RandFlat::shoot(rndmEngine, (-moduleData->getTimeJitter(0, 1) / 2.0),
                           (moduleData->getTimeJitter(0, 1) / 2.0));

  //double G4Time	 = totalCharge.time();

  double G4Time = getG4Time(totalCharge);
  double timing = moduleData->getTimeOffset(0, 1) + myTimeWalkEff + randomJitter + G4Time;
  BCID = static_cast<int>(floor(timing / moduleData->getBunchSpace()));
  //ATH_MSG_DEBUG (  CTW << " , " << myTimeWalkEff << " , " << G4Time << " , " << timing << " , " << BCID );

  return BCID;
}

// This is the new parameterization based on the 2015 collision data.
int FEI3SimTool::relativeBunch2015(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
                                   const PixelModuleData* moduleData,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  /**
   * 2016.03.29  Soshi.Tsuno@cern.ch
   *
   * The time walk effect is directly tuned with timing scan data (collision) in 2015.
   *
   * See reference in the talk,
   * https://indico.cern.ch/event/516099/contributions/1195889/attachments/1252177/1846815/pixelOffline_timing_04.04.2016_soshi.pdf
   *
   * Ideally, it could be directly parameterized as a function of given ToT.
   * However, the ToT calibration was changed over 2015-2016, where newly calibrated ToT value was not available for
   *2016.
   * For instance, the b-layer charge tuning was changed from ToT30@MIP (2015) to ToT18@MIP (2016).
   * Thus the time walk effect needs to be parameterized with more universal value, that is, charge information.
   * But it was non-trivial because of the migration effect between the border in ToT.
   *
   * Here in 2015 version, we apply the threshold of the 60% total charge to get a certain ToT value,
   * which most describes the data timing structure.
   *
   * 60% working point tune-2
   */

  double prob = 0.0;
  if (barrel_ec == 0 && layer_disk == 1) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9349;
      }   // corresponds to ToT=4
      else if (totalCharge.charge() < 4150.0) {
        prob = 0.2520;
      }   //                ToT=5
      else if (totalCharge.charge() < 4600.0) {
        prob = 0.0308;
      }   //                ToT=6
      else if (totalCharge.charge() < 5250.0) {
        prob = 0.0160;
      }   //                ToT=7
      else if (totalCharge.charge() < 5850.0) {
        prob = 0.0104;
      }   //                ToT=8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0127;
      }   //                ToT=9
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9087;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.2845;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0504;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0198;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0141;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0122;
      }
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9060;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.2885;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0387;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0126;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0116;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0052;
      }
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8774;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.3066;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0449;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0188;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0169;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0096;
      }
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8725;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.2962;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0472;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0188;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0141;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0130;
      }
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8731;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.3443;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0686;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0243;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0139;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0089;
      }
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8545;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.2946;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.0524;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0218;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0218;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0191;
      }
    }
  }
  if (barrel_ec == 0 && layer_disk == 2) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9479;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6051;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2031;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0735;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0462;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0272;
      }
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9736;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6344;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2439;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1000;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0435;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0335;
      }
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9461;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6180;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.1755;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0647;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0476;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0470;
      }
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9542;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.5839;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.1899;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0604;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0576;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0285;
      }
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9233;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.5712;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.1633;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0796;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0612;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0384;
      }
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8994;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.5176;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.1626;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0698;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0416;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0382;
      }
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8919;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.5313;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.1585;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.0520;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0318;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0254;
      }
    }
  }
  if (barrel_ec == 0 && layer_disk == 3) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9182;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6744;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.3174;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1460;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.1001;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0587;
      }
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9255;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6995;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.3046;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1449;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0954;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0608;
      }
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9419;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.7380;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.3346;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1615;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0726;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0564;
      }
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9319;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6747;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2640;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1018;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0588;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0502;
      }
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.9276;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6959;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2859;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1214;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0776;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0387;
      }
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8845;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6270;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2798;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1209;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0706;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0703;
      }
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 4100.0) {
        prob = 0.8726;
      } else if (totalCharge.charge() < 4150.0) {
        prob = 0.6358;
      } else if (totalCharge.charge() < 4600.0) {
        prob = 0.2907;
      } else if (totalCharge.charge() < 5250.0) {
        prob = 0.1051;
      } else if (totalCharge.charge() < 5850.0) {
        prob = 0.0646;
      } else if (totalCharge.charge() < 6500.0) {
        prob = 0.0685;
      }
    }
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }

  int BCID =
    static_cast<int>(floor((G4Time +
                            moduleData->getTimeOffset(barrel_ec,
                                                      layer_disk) + timeWalk) / moduleData->getBunchSpace()));

  return BCID;
}

int FEI3SimTool::relativeBunch2018(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
                                   const PixelModuleData* moduleData,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  /**
   * 2020.01.20  Minori.Fujimoto@cern.ch
   *
   * The time walk effect is directly tuned with timing scan data (collision) in 2017/18.
   * https://indico.cern.ch/event/880804/
   */

  double prob = 0.0;
  if (barrel_ec == 0 && layer_disk == 1) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.035;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.035;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.075;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.075;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.060;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.060;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.010;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 6480.0) {
        prob = 0.050;
      } //ToT=4
      else if (totalCharge.charge() < 6800.0) {
        prob = 0.008;
      } //ToT=5
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.010;
      } //ToT=6
      else if (totalCharge.charge() < 9000.0) {
        prob = 0.005;
      } //ToT=7
      else if (totalCharge.charge() < 10000.0) {
        prob = 0.001;
      } //ToT=8
      else if (totalCharge.charge() < 11000.0) {
        prob = 0.001;
      } //ToT=9
      else if (totalCharge.charge() < 12000.0) {
        prob = 0.001;
      } //ToT=10
      else if (totalCharge.charge() < 13000.0) {
        prob = 0.001;
      } //ToT=11
      else if (totalCharge.charge() < 14000.0) {
        prob = 0.001;
      } //ToT=12
    }
  }
  if (barrel_ec == 0 && layer_disk == 2) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.1012;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0350;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0250;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.0978;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0405;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0250;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.1012;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0392;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0250;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.1015;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0390;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0250;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.0977;
      } else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } else if (totalCharge.charge() < 5800.0) {
        prob = 0.0150;
      } //0.0284
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0150;
      } //0.0307
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.0966;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0369;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0256;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 5094.9) {
        prob = 0.1053;
      } //ToT = 6
      else if (totalCharge.charge() < 5100.0) {
        prob = 0.0500;
      } //ToT = 7
      else if (totalCharge.charge() < 5800.0) {
        prob = 0.0379;
      } //ToT = 8
      else if (totalCharge.charge() < 6500.0) {
        prob = 0.0252;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0200;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0150;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0100;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
  }
  if (barrel_ec == 0 && layer_disk == 3) {
    if (abs(moduleID) == 0) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1451;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0915;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0681;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0518;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 1) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1418;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0800;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0600;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0497;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 2) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1481;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0891;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0627;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0488;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 3) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1590;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0930;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0635;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0485;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 4) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1590;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.1214;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0776;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0387;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 5) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1518;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0874;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0603;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0460;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
    if (abs(moduleID) == 6) {
      if (totalCharge.charge() < 5055.0) {
        prob = 0.1461;
      } //ToT = 6
      else if (totalCharge.charge() < 5070.0) {
        prob = 0.0825;
      } //ToT = 7
      else if (totalCharge.charge() < 5700.0) {
        prob = 0.0571;
      } //ToT = 8
      else if (totalCharge.charge() < 6550.0) {
        prob = 0.0441;
      } //ToT = 9
      else if (totalCharge.charge() < 7000.0) {
        prob = 0.0300;
      } //ToT = 10
      else if (totalCharge.charge() < 7500.0) {
        prob = 0.0200;
      } //ToT = 11
      else if (totalCharge.charge() < 8200.0) {
        prob = 0.0200;
      } //ToT = 12
      else if (totalCharge.charge() < 9500.0) {
        prob = 0.0100;
      } //ToT = 13
    }
  }
  if (abs(barrel_ec) == 2 && layer_disk == 0) {
    if (totalCharge.charge() < 5550.0) {
      prob = 0.124;
    } //ToT = 6
    else if (totalCharge.charge() < 6000.0) {
      prob = 0.067;
    } //ToT = 7
    else if (totalCharge.charge() < 6400.0) {
      prob = 0.0005;
    } //ToT = 8
    else if (totalCharge.charge() < 6500.0) {
      prob = 0.002;
    } //ToT = 9
    else if (totalCharge.charge() < 6800.0) {
      prob = 0.040;
    } //ToT = 10
    else if (totalCharge.charge() < 7300.0) {
      prob = 0.031;
    } //ToT = 11
    else if (totalCharge.charge() < 7400.0) {
      prob = 0.040;
    }  //ToT = 12
    else if (totalCharge.charge() < 7500.0) {
      prob = 0.001;
    } //ToT = 13
  }
  if (abs(barrel_ec) == 2 && layer_disk == 1) {
    if (totalCharge.charge() < 5550.0) {
      prob = 0.124;
    } //ToT = 6
    else if (totalCharge.charge() < 6000.0) {
      prob = 0.067;
    } //ToT = 7
    else if (totalCharge.charge() < 6400.0) {
      prob = 0.0005;
    } //ToT = 8
    else if (totalCharge.charge() < 6500.0) {
      prob = 0.002;
    } //ToT = 9
    else if (totalCharge.charge() < 6800.0) {
      prob = 0.040;
    } //ToT = 10
    else if (totalCharge.charge() < 7300.0) {
      prob = 0.031;
    } //ToT = 11
    else if (totalCharge.charge() < 7400.0) {
      prob = 0.040;
    }  //ToT = 12
    else if (totalCharge.charge() < 7500.0) {
      prob = 0.001;
    } //ToT = 13
  }
  if (abs(barrel_ec) == 2 && layer_disk == 2) {
    if (totalCharge.charge() < 5400.0) {
      prob = 0.180;
    } //ToT=6
    else if (totalCharge.charge() < 5700.0) {
      prob = 0.067;
    } //ToT=7
    else if (totalCharge.charge() < 5701.0) {
      prob = 0.0005;
    } //ToT=8
    else if (totalCharge.charge() < 5702.0) {
      prob = 0.0005;
    } //ToT=9
    else if (totalCharge.charge() < 5800.0) {
      prob = 0.036;
    } //ToT=10
    else if (totalCharge.charge() < 6000.0) {
      prob = 0.031;
    } //ToT=11
    else if (totalCharge.charge() < 6500.0) {
      prob = 0.034;
    } //ToT=12
    else if (totalCharge.charge() < 7000.0) {
      prob = 0.001;
    } //ToT = 13
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }

  int BCID =
    static_cast<int>(floor((G4Time +
                            moduleData->getTimeOffset(barrel_ec,
                                                      layer_disk) + timeWalk) / moduleData->getBunchSpace()));

  return BCID;
}

int FEI3SimTool::relativeBunch2022(const SiTotalCharge& totalCharge, const double tot, int barrel_ec, int layer_disk, int moduleID,
                                   const PixelModuleData* moduleData,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  /**
   * 2023.03.07  Taken from timing scan data in 2022.
   */

  double prob = 0.0;
  if (barrel_ec==0 && layer_disk==1) {    // b-layer
    if (std::abs(moduleID)==0) {
      if      (tot<4.5)  { prob=0.0670; } // ToT=4
      else if (tot<5.5)  { prob=0.0311; } // ToT=5
      else if (tot<6.5)  { prob=0.0110; } // ToT=6
      else if (tot<7.5)  { prob=0.0098; } // ToT=7
      else if (tot<8.5)  { prob=0.0106; } // ToT=8
      else if (tot<9.5)  { prob=0.0072; } // ToT=9
      else if (tot<10.5) { prob=0.0061; } // ToT=10
      else if (tot<11.5) { prob=0.0072; } // ToT=11
      else if (tot<12.5) { prob=0.0089; } // ToT=12
    }
    else  if (std::abs(moduleID)==1) {
      if      (tot<4.5)  { prob=0.0275; } // ToT=4
      else if (tot<5.5)  { prob=0.0123; } // ToT=5
      else if (tot<6.5)  { prob=0.0099; } // ToT=6
      else if (tot<7.5)  { prob=0.0098; } // ToT=7
      else if (tot<8.5)  { prob=0.0084; } // ToT=8
      else if (tot<9.5)  { prob=0.0079; } // ToT=9
      else if (tot<10.5) { prob=0.0042; } // ToT=10
      else if (tot<11.5) { prob=0.0033; } // ToT=11
      else if (tot<12.5) { prob=0.0037; } // ToT=12
    }
    else  if (std::abs(moduleID)==2) {
      if      (tot<4.5)  { prob=0.0684; } // ToT=4
      else if (tot<5.5)  { prob=0.0275; } // ToT=5
      else if (tot<6.5)  { prob=0.0218; } // ToT=6
      else if (tot<7.5)  { prob=0.0142; } // ToT=7
      else if (tot<8.5)  { prob=0.0111; } // ToT=8
      else if (tot<9.5)  { prob=0.0077; } // ToT=9
      else if (tot<10.5) { prob=0.0068; } // ToT=10
      else if (tot<11.5) { prob=0.0062; } // ToT=11
      else if (tot<12.5) { prob=0.0051; } // ToT=12
    }
    else  if (std::abs(moduleID)==3) {
      if      (tot<4.5)  { prob=0.0536; } // ToT=4
      else if (tot<5.5)  { prob=0.0134; } // ToT=5
      else if (tot<6.5)  { prob=0.0101; } // ToT=6
      else if (tot<7.5)  { prob=0.0081; } // ToT=7
      else if (tot<8.5)  { prob=0.0074; } // ToT=8
      else if (tot<9.5)  { prob=0.0051; } // ToT=9
      else if (tot<10.5) { prob=0.0049; } // ToT=10
      else if (tot<11.5) { prob=0.0049; } // ToT=11
      else if (tot<12.5) { prob=0.0027; } // ToT=12
    }
    else  if (std::abs(moduleID)==4) {
      if      (tot<4.5)  { prob=0.0806; } // ToT=4
      else if (tot<5.5)  { prob=0.0340; } // ToT=5
      else if (tot<6.5)  { prob=0.0203; } // ToT=6
      else if (tot<7.5)  { prob=0.0225; } // ToT=7
      else if (tot<8.5)  { prob=0.0198; } // ToT=8
      else if (tot<9.5)  { prob=0.0121; } // ToT=9
      else if (tot<10.5) { prob=0.0095; } // ToT=10
      else if (tot<11.5) { prob=0.0069; } // ToT=11
      else if (tot<12.5) { prob=0.0049; } // ToT=12
    }
    else  if (std::abs(moduleID)==5) {
      if      (tot<4.5)  { prob=0.0736; } // ToT=4
      else if (tot<5.5)  { prob=0.0164; } // ToT=5
      else if (tot<6.5)  { prob=0.0143; } // ToT=6
      else if (tot<7.5)  { prob=0.0113; } // ToT=7
      else if (tot<8.5)  { prob=0.0091; } // ToT=8
      else if (tot<9.5)  { prob=0.0079; } // ToT=9
      else if (tot<10.5) { prob=0.0068; } // ToT=10
      else if (tot<11.5) { prob=0.0060; } // ToT=11
      else if (tot<12.5) { prob=0.0056; } // ToT=12
    }
    else  if (std::abs(moduleID)==6) {
      if      (tot<4.5)  { prob=0.1190; } // ToT=4
      else if (tot<5.5)  { prob=0.0275; } // ToT=5
      else if (tot<6.5)  { prob=0.0126; } // ToT=6
      else if (tot<7.5)  { prob=0.0125; } // ToT=7
      else if (tot<8.5)  { prob=0.0083; } // ToT=8
      else if (tot<9.5)  { prob=0.0076; } // ToT=9
      else if (tot<10.5) { prob=0.0064; } // ToT=10
      else if (tot<11.5) { prob=0.0055; } // ToT=11
      else if (tot<12.5) { prob=0.0060; } // ToT=12
    }
  }
  else if (barrel_ec==0 && layer_disk==2) {  // layer-1
    if (std::abs(moduleID)==0) {
      if      (tot<5.5)  { prob=0.3755; } // ToT=5
      else if (tot<6.5)  { prob=0.0841; } // ToT=6
      else if (tot<7.5)  { prob=0.0516; } // ToT=7
      else if (tot<8.5)  { prob=0.0390; } // ToT=8
      else if (tot<9.5)  { prob=0.0265; } // ToT=9
      else if (tot<10.5) { prob=0.0206; } // ToT=10
      else if (tot<11.5) { prob=0.0234; } // ToT=11
      else if (tot<12.5) { prob=0.0184; } // ToT=12
      else if (tot<13.5) { prob=0.0153; } // ToT=13
      else if (tot<14.5) { prob=0.0122; } // ToT=14
      else if (tot<15.5) { prob=0.0100; } // ToT=15
      else if (tot<16.5) { prob=0.0129; } // ToT=16
      else if (tot<17.5) { prob=0.0109; } // ToT=17
      else if (tot<18.5) { prob=0.0088; } // ToT=18
      else if (tot<19.5) { prob=0.0134; } // ToT=19
      else if (tot<20.5) { prob=0.0145; } // ToT=20
      else if (tot<21.5) { prob=0.0107; } // ToT=21
      else if (tot<22.5) { prob=0.0104; } // ToT=22
      else if (tot<23.5) { prob=0.0123; } // ToT=23
      else if (tot<24.5) { prob=0.0070; } // ToT=24
      else if (tot<25.5) { prob=0.0065; } // ToT=25
    }
    else if (std::abs(moduleID)==1) {
      if      (tot<5.5)  { prob=0.4535; } // ToT=5
      else if (tot<6.5)  { prob=0.1277; } // ToT=6
      else if (tot<7.5)  { prob=0.0626; } // ToT=7
      else if (tot<8.5)  { prob=0.0443; } // ToT=8
      else if (tot<9.5)  { prob=0.0341; } // ToT=9
      else if (tot<10.5) { prob=0.0288; } // ToT=10
      else if (tot<11.5) { prob=0.0270; } // ToT=11
      else if (tot<12.5) { prob=0.0224; } // ToT=12
      else if (tot<13.5) { prob=0.0227; } // ToT=13
      else if (tot<14.5) { prob=0.0196; } // ToT=14
      else if (tot<15.5) { prob=0.0145; } // ToT=15
      else if (tot<16.5) { prob=0.0163; } // ToT=16
      else if (tot<17.5) { prob=0.0128; } // ToT=17
      else if (tot<18.5) { prob=0.0129; } // ToT=18
      else if (tot<19.5) { prob=0.0126; } // ToT=19
      else if (tot<20.5) { prob=0.0144; } // ToT=20
      else if (tot<21.5) { prob=0.0118; } // ToT=21
      else if (tot<22.5) { prob=0.0091; } // ToT=22
      else if (tot<23.5) { prob=0.0131; } // ToT=23
      else if (tot<24.5) { prob=0.0120; } // ToT=24
      else if (tot<25.5) { prob=0.0119; } // ToT=25
    }
    else if (std::abs(moduleID)==2) {
      if      (tot<5.5)  { prob=0.5102; } // ToT=5
      else if (tot<6.5)  { prob=0.1059; } // ToT=6
      else if (tot<7.5)  { prob=0.0575; } // ToT=7
      else if (tot<8.5)  { prob=0.0411; } // ToT=8
      else if (tot<9.5)  { prob=0.0309; } // ToT=9
      else if (tot<10.5) { prob=0.0333; } // ToT=10
      else if (tot<11.5) { prob=0.0274; } // ToT=11
      else if (tot<12.5) { prob=0.0258; } // ToT=12
      else if (tot<13.5) { prob=0.0209; } // ToT=13
      else if (tot<14.5) { prob=0.0209; } // ToT=14
      else if (tot<15.5) { prob=0.0170; } // ToT=15
      else if (tot<16.5) { prob=0.0143; } // ToT=16
      else if (tot<17.5) { prob=0.0141; } // ToT=17
      else if (tot<18.5) { prob=0.0164; } // ToT=18
      else if (tot<19.5) { prob=0.0145; } // ToT=19
      else if (tot<20.5) { prob=0.0131; } // ToT=20
      else if (tot<21.5) { prob=0.0112; } // ToT=21
      else if (tot<22.5) { prob=0.0159; } // ToT=22
      else if (tot<23.5) { prob=0.0128; } // ToT=23
      else if (tot<24.5) { prob=0.0097; } // ToT=24
      else if (tot<25.5) { prob=0.0088; } // ToT=25
    }
    else if (std::abs(moduleID)==3) {
      if      (tot<5.5)  { prob=0.4122; } // ToT=5
      else if (tot<6.5)  { prob=0.1038; } // ToT=6
      else if (tot<7.5)  { prob=0.0567; } // ToT=7
      else if (tot<8.5)  { prob=0.0371; } // ToT=8
      else if (tot<9.5)  { prob=0.0288; } // ToT=9
      else if (tot<10.5) { prob=0.0268; } // ToT=10
      else if (tot<11.5) { prob=0.0211; } // ToT=11
      else if (tot<12.5) { prob=0.0238; } // ToT=12
      else if (tot<13.5) { prob=0.0223; } // ToT=13
      else if (tot<14.5) { prob=0.0166; } // ToT=14
      else if (tot<15.5) { prob=0.0161; } // ToT=15
      else if (tot<16.5) { prob=0.0175; } // ToT=16
      else if (tot<17.5) { prob=0.0129; } // ToT=17
      else if (tot<18.5) { prob=0.0091; } // ToT=18
      else if (tot<19.5) { prob=0.0136; } // ToT=19
      else if (tot<20.5) { prob=0.0126; } // ToT=20
      else if (tot<21.5) { prob=0.0133; } // ToT=21
      else if (tot<22.5) { prob=0.0087; } // ToT=22
      else if (tot<23.5) { prob=0.0082; } // ToT=23
      else if (tot<24.5) { prob=0.0077; } // ToT=24
      else if (tot<25.5) { prob=0.0078; } // ToT=25
    }
    else if (std::abs(moduleID)==4) {
      if      (tot<5.5)  { prob=0.4174; } // ToT=5
      else if (tot<6.5)  { prob=0.0976; } // ToT=6
      else if (tot<7.5)  { prob=0.0527; } // ToT=7
      else if (tot<8.5)  { prob=0.0403; } // ToT=8
      else if (tot<9.5)  { prob=0.0329; } // ToT=9
      else if (tot<10.5) { prob=0.0245; } // ToT=10
      else if (tot<11.5) { prob=0.0254; } // ToT=11
      else if (tot<12.5) { prob=0.0240; } // ToT=12
      else if (tot<13.5) { prob=0.0228; } // ToT=13
      else if (tot<14.5) { prob=0.0203; } // ToT=14
      else if (tot<15.5) { prob=0.0151; } // ToT=15
      else if (tot<16.5) { prob=0.0143; } // ToT=16
      else if (tot<17.5) { prob=0.0171; } // ToT=17
      else if (tot<18.5) { prob=0.0124; } // ToT=18
      else if (tot<19.5) { prob=0.0140; } // ToT=19
      else if (tot<20.5) { prob=0.0119; } // ToT=20
      else if (tot<21.5) { prob=0.0134; } // ToT=21
      else if (tot<22.5) { prob=0.0090; } // ToT=22
      else if (tot<23.5) { prob=0.0093; } // ToT=23
      else if (tot<24.5) { prob=0.0109; } // ToT=24
      else if (tot<25.5) { prob=0.0110; } // ToT=25
    }
    else if (std::abs(moduleID)==5) {
      if      (tot<5.5)  { prob=0.4079; } // ToT=5
      else if (tot<6.5)  { prob=0.0869; } // ToT=6
      else if (tot<7.5)  { prob=0.0441; } // ToT=7
      else if (tot<8.5)  { prob=0.0348; } // ToT=8
      else if (tot<9.5)  { prob=0.0308; } // ToT=9
      else if (tot<10.5) { prob=0.0249; } // ToT=10
      else if (tot<11.5) { prob=0.0221; } // ToT=11
      else if (tot<12.5) { prob=0.0216; } // ToT=12
      else if (tot<13.5) { prob=0.0211; } // ToT=13
      else if (tot<14.5) { prob=0.0213; } // ToT=14
      else if (tot<15.5) { prob=0.0184; } // ToT=15
      else if (tot<16.5) { prob=0.0163; } // ToT=16
      else if (tot<17.5) { prob=0.0167; } // ToT=17
      else if (tot<18.5) { prob=0.0143; } // ToT=18
      else if (tot<19.5) { prob=0.0125; } // ToT=19
      else if (tot<20.5) { prob=0.0111; } // ToT=20
      else if (tot<21.5) { prob=0.0124; } // ToT=21
      else if (tot<22.5) { prob=0.0139; } // ToT=22
      else if (tot<23.5) { prob=0.0148; } // ToT=23
      else if (tot<24.5) { prob=0.0104; } // ToT=24
      else if (tot<25.5) { prob=0.0074; } // ToT=25
    }
    else if (std::abs(moduleID)==6) {
      if      (tot<5.5)  { prob=0.4023; } // ToT=5
      else if (tot<6.5)  { prob=0.1047; } // ToT=6
      else if (tot<7.5)  { prob=0.0589; } // ToT=7
      else if (tot<8.5)  { prob=0.0439; } // ToT=8
      else if (tot<9.5)  { prob=0.0332; } // ToT=9
      else if (tot<10.5) { prob=0.0292; } // ToT=10
      else if (tot<11.5) { prob=0.0270; } // ToT=11
      else if (tot<12.5) { prob=0.0209; } // ToT=12
      else if (tot<13.5) { prob=0.0166; } // ToT=13
      else if (tot<14.5) { prob=0.0192; } // ToT=14
      else if (tot<15.5) { prob=0.0194; } // ToT=15
      else if (tot<16.5) { prob=0.0191; } // ToT=16
      else if (tot<17.5) { prob=0.0155; } // ToT=17
      else if (tot<18.5) { prob=0.0149; } // ToT=18
      else if (tot<19.5) { prob=0.0123; } // ToT=19
      else if (tot<20.5) { prob=0.0117; } // ToT=20
      else if (tot<21.5) { prob=0.0113; } // ToT=21
      else if (tot<22.5) { prob=0.0103; } // ToT=22
      else if (tot<23.5) { prob=0.0153; } // ToT=23
      else if (tot<24.5) { prob=0.0088; } // ToT=24
      else if (tot<25.5) { prob=0.0100; } // ToT=25
    }
  }
  else if (barrel_ec==0 && layer_disk==3) {  // layer-2
    if (std::abs(moduleID)==0) {
      if      (tot<5.5)  { prob=0.4812; } // ToT=5
      else if (tot<6.5)  { prob=0.1637; } // ToT=6
      else if (tot<7.5)  { prob=0.0945; } // ToT=7
      else if (tot<8.5)  { prob=0.0670; } // ToT=8
      else if (tot<9.5)  { prob=0.0579; } // ToT=9
      else if (tot<10.5) { prob=0.0451; } // ToT=10
      else if (tot<11.5) { prob=0.0317; } // ToT=11
      else if (tot<12.5) { prob=0.0249; } // ToT=12
      else if (tot<13.5) { prob=0.0191; } // ToT=13
      else if (tot<14.5) { prob=0.0270; } // ToT=14
      else if (tot<15.5) { prob=0.0227; } // ToT=15
      else if (tot<16.5) { prob=0.0190; } // ToT=16
      else if (tot<17.5) { prob=0.0168; } // ToT=17
      else if (tot<18.5) { prob=0.0205; } // ToT=18
      else if (tot<19.5) { prob=0.0162; } // ToT=19
      else if (tot<20.5) { prob=0.0179; } // ToT=20
      else if (tot<21.5) { prob=0.0228; } // ToT=21
      else if (tot<22.5) { prob=0.0169; } // ToT=22
      else if (tot<23.5) { prob=0.0136; } // ToT=23
      else if (tot<24.5) { prob=0.0089; } // ToT=24
      else if (tot<25.5) { prob=0.0198; } // ToT=25
    }
    else if (std::abs(moduleID)==1) {
      if      (tot<5.5)  { prob=0.5605; } // ToT=5
      else if (tot<6.5)  { prob=0.1837; } // ToT=6
      else if (tot<7.5)  { prob=0.1040; } // ToT=7
      else if (tot<8.5)  { prob=0.0654; } // ToT=8
      else if (tot<9.5)  { prob=0.0465; } // ToT=9
      else if (tot<10.5) { prob=0.0391; } // ToT=10
      else if (tot<11.5) { prob=0.0337; } // ToT=11
      else if (tot<12.5) { prob=0.0325; } // ToT=12
      else if (tot<13.5) { prob=0.0242; } // ToT=13
      else if (tot<14.5) { prob=0.0270; } // ToT=14
      else if (tot<15.5) { prob=0.0218; } // ToT=15
      else if (tot<16.5) { prob=0.0191; } // ToT=16
      else if (tot<17.5) { prob=0.0172; } // ToT=17
      else if (tot<18.5) { prob=0.0149; } // ToT=18
      else if (tot<19.5) { prob=0.0158; } // ToT=19
      else if (tot<20.5) { prob=0.0153; } // ToT=20
      else if (tot<21.5) { prob=0.0134; } // ToT=21
      else if (tot<22.5) { prob=0.0204; } // ToT=22
      else if (tot<23.5) { prob=0.0142; } // ToT=23
      else if (tot<24.5) { prob=0.0131; } // ToT=24
      else if (tot<25.5) { prob=0.0158; } // ToT=25
    }
    else if (std::abs(moduleID)==2) {
      if      (tot<5.5)  { prob=0.5681; } // ToT=5
      else if (tot<6.5)  { prob=0.1665; } // ToT=6
      else if (tot<7.5)  { prob=0.1006; } // ToT=7
      else if (tot<8.5)  { prob=0.0659; } // ToT=8
      else if (tot<9.5)  { prob=0.0478; } // ToT=9
      else if (tot<10.5) { prob=0.0417; } // ToT=10
      else if (tot<11.5) { prob=0.0330; } // ToT=11
      else if (tot<12.5) { prob=0.0343; } // ToT=12
      else if (tot<13.5) { prob=0.0310; } // ToT=13
      else if (tot<14.5) { prob=0.0271; } // ToT=14
      else if (tot<15.5) { prob=0.0244; } // ToT=15
      else if (tot<16.5) { prob=0.0169; } // ToT=16
      else if (tot<17.5) { prob=0.0191; } // ToT=17
      else if (tot<18.5) { prob=0.0223; } // ToT=18
      else if (tot<19.5) { prob=0.0211; } // ToT=19
      else if (tot<20.5) { prob=0.0170; } // ToT=20
      else if (tot<21.5) { prob=0.0153; } // ToT=21
      else if (tot<22.5) { prob=0.0193; } // ToT=22
      else if (tot<23.5) { prob=0.0194; } // ToT=23
      else if (tot<24.5) { prob=0.0146; } // ToT=24
      else if (tot<25.5) { prob=0.0141; } // ToT=25
    }
    else if (std::abs(moduleID)==3) {
      if      (tot<5.5)  { prob=0.5725; } // ToT=5
      else if (tot<6.5)  { prob=0.1725; } // ToT=6
      else if (tot<7.5)  { prob=0.0918; } // ToT=7
      else if (tot<8.5)  { prob=0.0529; } // ToT=8
      else if (tot<9.5)  { prob=0.0416; } // ToT=9
      else if (tot<10.5) { prob=0.0336; } // ToT=10
      else if (tot<11.5) { prob=0.0261; } // ToT=11
      else if (tot<12.5) { prob=0.0273; } // ToT=12
      else if (tot<13.5) { prob=0.0241; } // ToT=13
      else if (tot<14.5) { prob=0.0195; } // ToT=14
      else if (tot<15.5) { prob=0.0264; } // ToT=15
      else if (tot<16.5) { prob=0.0178; } // ToT=16
      else if (tot<17.5) { prob=0.0201; } // ToT=17
      else if (tot<18.5) { prob=0.0191; } // ToT=18
      else if (tot<19.5) { prob=0.0200; } // ToT=19
      else if (tot<20.5) { prob=0.0150; } // ToT=20
      else if (tot<21.5) { prob=0.0139; } // ToT=21
      else if (tot<22.5) { prob=0.0151; } // ToT=22
      else if (tot<23.5) { prob=0.0104; } // ToT=23
      else if (tot<24.5) { prob=0.0121; } // ToT=24
      else if (tot<25.5) { prob=0.0111; } // ToT=25
    }
    else if (std::abs(moduleID)==4) {
      if      (tot<5.5)  { prob=0.5377; } // ToT=5
      else if (tot<6.5)  { prob=0.1785; } // ToT=6
      else if (tot<7.5)  { prob=0.1089; } // ToT=7
      else if (tot<8.5)  { prob=0.0702; } // ToT=8
      else if (tot<9.5)  { prob=0.0474; } // ToT=9
      else if (tot<10.5) { prob=0.0476; } // ToT=10
      else if (tot<11.5) { prob=0.0392; } // ToT=11
      else if (tot<12.5) { prob=0.0308; } // ToT=12
      else if (tot<13.5) { prob=0.0318; } // ToT=13
      else if (tot<14.5) { prob=0.0290; } // ToT=14
      else if (tot<15.5) { prob=0.0244; } // ToT=15
      else if (tot<16.5) { prob=0.0262; } // ToT=16
      else if (tot<17.5) { prob=0.0228; } // ToT=17
      else if (tot<18.5) { prob=0.0163; } // ToT=18
      else if (tot<19.5) { prob=0.0158; } // ToT=19
      else if (tot<20.5) { prob=0.0153; } // ToT=20
      else if (tot<21.5) { prob=0.0185; } // ToT=21
      else if (tot<22.5) { prob=0.0157; } // ToT=22
      else if (tot<23.5) { prob=0.0165; } // ToT=23
      else if (tot<24.5) { prob=0.0112; } // ToT=24
      else if (tot<25.5) { prob=0.0095; } // ToT=25
    }
    else if (std::abs(moduleID)==5) {
      if      (tot<5.5)  { prob=0.5317; } // ToT=5
      else if (tot<6.5)  { prob=0.1626; } // ToT=6
      else if (tot<7.5)  { prob=0.0897; } // ToT=7
      else if (tot<8.5)  { prob=0.0606; } // ToT=8
      else if (tot<9.5)  { prob=0.0434; } // ToT=9
      else if (tot<10.5) { prob=0.0346; } // ToT=10
      else if (tot<11.5) { prob=0.0290; } // ToT=11
      else if (tot<12.5) { prob=0.0289; } // ToT=12
      else if (tot<13.5) { prob=0.0268; } // ToT=13
      else if (tot<14.5) { prob=0.0268; } // ToT=14
      else if (tot<15.5) { prob=0.0275; } // ToT=15
      else if (tot<16.5) { prob=0.0262; } // ToT=16
      else if (tot<17.5) { prob=0.0239; } // ToT=17
      else if (tot<18.5) { prob=0.0222; } // ToT=18
      else if (tot<19.5) { prob=0.0216; } // ToT=19
      else if (tot<20.5) { prob=0.0213; } // ToT=20
      else if (tot<21.5) { prob=0.0195; } // ToT=21
      else if (tot<22.5) { prob=0.0209; } // ToT=22
      else if (tot<23.5) { prob=0.0150; } // ToT=23
      else if (tot<24.5) { prob=0.0186; } // ToT=24
      else if (tot<25.5) { prob=0.0125; } // ToT=25
    }
    else if (std::abs(moduleID)==6) {
      if      (tot<5.5)  { prob=0.5822; } // ToT=5
      else if (tot<6.5)  { prob=0.1635; } // ToT=6
      else if (tot<7.5)  { prob=0.0925; } // ToT=7
      else if (tot<8.5)  { prob=0.0583; } // ToT=8
      else if (tot<9.5)  { prob=0.0498; } // ToT=9
      else if (tot<10.5) { prob=0.0434; } // ToT=10
      else if (tot<11.5) { prob=0.0303; } // ToT=11
      else if (tot<12.5) { prob=0.0267; } // ToT=12
      else if (tot<13.5) { prob=0.0253; } // ToT=13
      else if (tot<14.5) { prob=0.0247; } // ToT=14
      else if (tot<15.5) { prob=0.0219; } // ToT=15
      else if (tot<16.5) { prob=0.0223; } // ToT=16
      else if (tot<17.5) { prob=0.0195; } // ToT=17
      else if (tot<18.5) { prob=0.0141; } // ToT=18
      else if (tot<19.5) { prob=0.0118; } // ToT=19
      else if (tot<20.5) { prob=0.0207; } // ToT=20
      else if (tot<21.5) { prob=0.0169; } // ToT=21
      else if (tot<22.5) { prob=0.0121; } // ToT=22
      else if (tot<23.5) { prob=0.0117; } // ToT=23
      else if (tot<24.5) { prob=0.0118; } // ToT=24
      else if (tot<25.5) { prob=0.0102; } // ToT=25
    }
  }
  else if (std::abs(barrel_ec)==2) {  // Endcap
    if      (tot<5.5)  { prob=0.4896; } // ToT=5
    else if (tot<6.5)  { prob=0.1294; } // ToT=6
    else if (tot<7.5)  { prob=0.0684; } // ToT=7
    else if (tot<8.5)  { prob=0.0491; } // ToT=8
    else if (tot<9.5)  { prob=0.0379; } // ToT=9
    else if (tot<10.5) { prob=0.0350; } // ToT=10
    else if (tot<11.5) { prob=0.0315; } // ToT=11
    else if (tot<12.5) { prob=0.0275; } // ToT=12
    else if (tot<13.5) { prob=0.0251; } // ToT=13
    else if (tot<14.5) { prob=0.0246; } // ToT=14
    else if (tot<15.5) { prob=0.0221; } // ToT=15
    else if (tot<16.5) { prob=0.0186; } // ToT=16
    else if (tot<17.5) { prob=0.0182; } // ToT=17
    else if (tot<18.5) { prob=0.0190; } // ToT=18
    else if (tot<19.5) { prob=0.0176; } // ToT=19
    else if (tot<20.5) { prob=0.0133; } // ToT=20
    else if (tot<21.5) { prob=0.0127; } // ToT=21
    else if (tot<22.5) { prob=0.0107; } // ToT=22
    else if (tot<23.5) { prob=0.0113; } // ToT=23
    else if (tot<24.5) { prob=0.0096; } // ToT=24
    else if (tot<25.5) { prob=0.0086; } // ToT=25
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }
  int BCID = static_cast<int>(floor((G4Time+moduleData->getTimeOffset(barrel_ec,layer_disk)+timeWalk)/moduleData->getBunchSpace()));
  return BCID;
}
