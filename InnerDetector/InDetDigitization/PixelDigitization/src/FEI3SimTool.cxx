/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "FEI3SimTool.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelConditionsData/ChargeCalibParameters.h" //for Thresholds
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "InDetRawData/Pixel1RawData.h"
#include "InDetIdentifier/PixelID.h"
//
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"

#include <cmath>

namespace{
  template <size_t n>
  double 
  getProbability(const std::array<double,n> &bounds, const std::array<double,n> &probs, const double &val, const double defval=0.){
    auto pCategory = std::upper_bound(bounds.begin(), bounds.end(),val);
    if (pCategory == bounds.end()) return defval;
    auto idx = std::distance(bounds.begin(), pCategory);
    return probs[idx];
  }


}

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

  if (std::abs(barrel_ec) != m_BarrelEC) {
    return;
  }

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<PixelModuleData> moduleDataHandle(m_moduleDataKey, ctx);
  const PixelModuleData *moduleData = *moduleDataHandle;
  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;
  const auto selectedTuneYear = moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex);
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
    const auto thresholds = calibData->getThresholds(type, moduleHash, FE);
    // Apply analog threshold, timing simulation
    double th0 = thresholds.value;
    double ith0 = thresholds.inTimeValue;

    double thrand1 = CLHEP::RandGaussZiggurat::shoot(rndmEngine);
    double thrand2 = CLHEP::RandGaussZiggurat::shoot(rndmEngine);
    double threshold = th0
                       + thresholds.sigma * thrand1
                       + thresholds.noise * thrand2;
                       // This noise check is unaffected by digitizationFlags.doInDetNoise in
                       // 21.0 - see PixelCellDiscriminator.cxx in that branch

    if (charge > threshold) {
      int bunchSim = 0;
      if (diode.totalCharge().fromTrack()) {
        if (selectedTuneYear == 2022) {
          bunchSim = relativeBunch2022(diode.totalCharge(), tot, barrel_ec, layerIndex, moduleIndex, rndmEngine);
        } 
        else if (selectedTuneYear == 2018) {
          bunchSim = relativeBunch2018(diode.totalCharge(), barrel_ec, layerIndex, moduleIndex,  rndmEngine);
        } 
        else if (selectedTuneYear == 2015) {
          bunchSim = relativeBunch2015(diode.totalCharge(), barrel_ec, layerIndex, moduleIndex,  rndmEngine);
        } 
        else if (selectedTuneYear == 2009) {
          double intimethreshold = (ith0 / th0) * threshold;
          bunchSim = relativeBunch2009(threshold, intimethreshold, diode.totalCharge(), rndmEngine);
        }
      } 
      else {
        if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) > 0) {
          bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, m_numberOfBcid);
        }
      }

      if (bunchSim < 0 || bunchSim > m_numberOfBcid) {
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
    if (bunch >= 0 && bunch < m_numberOfBcid) {
      rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch, 0, bunch));
    }
    
    // Duplication mechanism for FEI3 small hits :
    if (m_duplication) {//is true for run1 only
      static constexpr int smallHitThreshold{7}; //constant for both barrel and endcap, never changes
      bool smallHitChk = false;
      if (nToT <= smallHitThreshold) {
        smallHitChk = true;
      }
      if (smallHitChk && bunch > 0 && bunch <= m_numberOfBcid) {
        rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch - 1, 0, bunch - 1));
      }
    }
  }
  }

int FEI3SimTool::relativeBunch2009(const double threshold, const double intimethreshold,
                                   const SiTotalCharge& totalCharge,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  int BCID = 0;
  double myTimeWalkEff = 0.;
  double overdrive = intimethreshold - threshold;

  //my TimeWalk computation through PARAMETRIZATION (by Francesco De Lorenzi - Milan)
  //double curvature  =  7.6e7*overdrive-2.64e10;
  //double divergence = -1.6*overdrive+942 ;
  //double myTimeWalk    = curvature/(pow((totalCharge.charge()-divergence),2.5));

  //my TimeWalk computation through PARAMETRIZATION from 2009 cosmic data (by I. Ibragimov and D. Miller)
  double p1 = 20. / std::log(intimethreshold / overdrive);
  double p0 = p1 * std::log(1. - threshold / 100000.);

  double myTimeWalk = -p0 - p1 * std::log(1. - threshold / totalCharge.charge());

  myTimeWalkEff = myTimeWalk + myTimeWalk * 0.2 * CLHEP::RandGaussZiggurat::shoot(rndmEngine);
  const double limit = m_timeJitter * 0.5;
  double randomJitter = CLHEP::RandFlat::shoot(rndmEngine, - limit, limit);

  //double G4Time	 = totalCharge.time();

  double G4Time = getG4Time(totalCharge);
  double timing = m_timeOffset + myTimeWalkEff + randomJitter + G4Time;
  BCID = static_cast<int>(std::floor(timing / m_bunchSpace));
  //ATH_MSG_DEBUG (  CTW << " , " << myTimeWalkEff << " , " << G4Time << " , " << timing << " , " << BCID );

  return BCID;
}

// This is the new parameterization based on the 2015 collision data.
int FEI3SimTool::relativeBunch2015(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
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
  static constexpr size_t nModules=7;
  static constexpr size_t nCalibrationPoints=6;
  static constexpr std::array<double, nCalibrationPoints> totCharges = {
    4100., 4150., 4600., 
    5250., 5850.,6500.
  };
  
  double prob = 0.0;
  const bool isBarrel = (barrel_ec == 0);
  if (isBarrel && layer_disk == 1) {
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
     {{ 0.9349, 0.2520, 0.0308, 0.0160, 0.0104, 0.0127},
     { 0.9087, 0.2845, 0.0504, 0.0198, 0.0141, 0.0122},
     { 0.9060, 0.2885, 0.0387, 0.0126, 0.0116, 0.0052},
     { 0.8774, 0.3066, 0.0449, 0.0188, 0.0169, 0.0096},
     { 0.8725, 0.2962, 0.0472, 0.0188, 0.0141, 0.0130},
     { 0.8731, 0.3443, 0.0686, 0.0243, 0.0139, 0.0089},
     { 0.8545, 0.2946, 0.0524, 0.0218, 0.0218, 0.0191}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, totalCharge.charge() );
  }
  
  if (isBarrel && layer_disk == 2) {
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.9479, 0.6051, 0.2031, 0.0735, 0.0462, 0.0272},
      {0.9736, 0.6344, 0.2439, 0.1000, 0.0435, 0.0335},
      {0.9461, 0.6180, 0.1755, 0.0647, 0.0476, 0.0470},
      {0.9542, 0.5839, 0.1899, 0.0604, 0.0576, 0.0285},
      {0.9233, 0.5712, 0.1633, 0.0796, 0.0612, 0.0384},
      {0.8994, 0.5176, 0.1626, 0.0698, 0.0416, 0.0382},
      {0.8919, 0.5313, 0.1585, 0.0520, 0.0318, 0.0254}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, totalCharge.charge() );
  }
  if (isBarrel && layer_disk == 3) {
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.9182, 0.6744, 0.3174,0.1460, 0.1001, 0.0587},
      {0.9255, 0.6995, 0.3046,0.1449, 0.0954, 0.0608},
      {0.9419, 0.7380, 0.3346,0.1615, 0.0726, 0.0564},
      {0.9319, 0.6747, 0.2640,0.1018, 0.0588, 0.0502},
      {0.9276, 0.6959, 0.2859,0.1214, 0.0776, 0.0387},
      {0.8845, 0.6270, 0.2798,0.1209, 0.0706, 0.0703},
      {0.8726, 0.6358, 0.2907,0.1051, 0.0646, 0.0685}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, totalCharge.charge() );
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }

  int BCID =
    static_cast<int>(std::floor((G4Time +
                            m_timeOffset + timeWalk) / m_bunchSpace));

  return BCID;
}

int FEI3SimTool::relativeBunch2018(const SiTotalCharge& totalCharge, int barrel_ec, int layer_disk, int moduleID,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  /**
   * 2020.01.20  Minori.Fujimoto@cern.ch
   *
   * The time walk effect is directly tuned with timing scan data (collision) in 2017/18.
   * https://indico.cern.ch/event/880804/
   */
  const bool isBarrel = (barrel_ec == 0);
  const bool isEndcap = (std::abs(barrel_ec) == 2);
  //is there any other possibility, or does !isBarrel == isEndcap ?
  static constexpr size_t nModules=7;
  double prob = 0.0;
  if (isBarrel && layer_disk == 1) {
    static constexpr size_t nCalibrationPoints=9;
    static constexpr std::array<double, nCalibrationPoints> totCharges{
      6480., 6800., 7000., 
      9000., 10000., 11000., 
      12000., 13000., 14000.
    };
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.035, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.035, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.075, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.075, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.060, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.060, 0.010, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001},
      {0.050, 0.008, 0.010, 0.005, 0.001, 0.001, 0.001, 0.001, 0.001}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, totalCharge.charge() );
  
  }
  if (isBarrel && layer_disk == 2) {
    static constexpr size_t nCalibrationPoints=8;
    static constexpr std::array<double, nCalibrationPoints> totChargeLayer2{
      5094.9, 5100.0, 5800.0,
      6500.0, 7000.0, 7500.0,
      8200.0, 9500.0
    };
    
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.1012, 0.0500, 0.0350, 0.0250, 0.0200, 0.0150,0.0100, 0.0100},
      {0.0978, 0.0500, 0.0405, 0.0250, 0.0200, 0.0150, 0.0100, 0.0100},
      {0.1012, 0.0500, 0.0392, 0.0250, 0.0200, 0.0150, 0.0100, 0.0100},
      {0.1015, 0.0500, 0.0390, 0.0250, 0.0200, 0.0150, 0.0100, 0.0100},
      {0.0977, 0.0500, 0.0150, 0.0150, 0.0200, 0.0150, 0.0100, 0.0100},
      {0.0966, 0.0500, 0.0369, 0.0256, 0.0200, 0.0150, 0.0100, 0.0100},
      {0.1053, 0.0500, 0.0379, 0.0252, 0.0200, 0.0150, 0.0100, 0.0100}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totChargeLayer2, probArray, totalCharge.charge() );
  }
  if (isBarrel && layer_disk == 3) {
    static constexpr size_t nCalibrationPoints=8;
    static constexpr std::array<double, nCalibrationPoints> totChargeLayer3{
      5055.0, 5070.0, 5700.0, 
      6550.0, 7000.0, 7500.0,
      8200.0, 9500.0
    };
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.1451, 0.0915, 0.0681, 0.0518, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1418, 0.0800, 0.0600, 0.0497, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1481, 0.0891, 0.0627, 0.0488, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1590, 0.0930, 0.0635, 0.0485, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1590, 0.1214, 0.0776, 0.0387, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1518, 0.0874, 0.0603, 0.0460, 0.0300, 0.0200, 0.0200, 0.0100},
      {0.1461, 0.0825, 0.0571, 0.0441, 0.0300, 0.0200, 0.0200, 0.0100}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totChargeLayer3, probArray, totalCharge.charge() );
  }
  if (isEndcap && layer_disk == 0) {
    static constexpr size_t nCalibrationPoints=8;
    static constexpr std::array<double, nCalibrationPoints> totChargeLayerEc0{
      5550.0, 6000.0, 6400.0,
      6500.0, 6800.0, 7300.0,
      7400.0, 7500.0
    };
    static constexpr std::array<double, nCalibrationPoints> prob0{
      0.124, 0.067, 0.0005,
      0.002, 0.040, 0.031,
      0.040, 0.001
    };
    prob = getProbability(totChargeLayerEc0, prob0, totalCharge.charge());
  }
  if (isEndcap && layer_disk == 1) {
    static constexpr size_t nCalibrationPoints=8;
    static constexpr std::array<double, nCalibrationPoints> totChargeLayerEc1{
      5550.0, 6000.0, 6400.0,
      6500.0, 6800.0, 7300.0,
      7400.0, 7500.0
    };
    static constexpr std::array<double, nCalibrationPoints> prob0{
      0.124, 0.067, 0.0005,
      0.002, 0.040, 0.031,
      0.040, 0.001
    };
    prob = getProbability(totChargeLayerEc1, prob0, totalCharge.charge());
  }
  if (isEndcap && layer_disk == 2) {
    static constexpr size_t nCalibrationPoints=8;
    static constexpr std::array<double, nCalibrationPoints> totChargeLayerEc2{
      5400.0, 5700.0, 5701.0,
      5702.0, 5800.0, 6000.0,
      6500.0, 7000.0
    };
    static constexpr std::array<double, nCalibrationPoints> prob0{
      0.180, 0.067, 0.0005,
      0.0005, 0.036, 0.031,
      0.034, 0.001
    };
    prob = getProbability(totChargeLayerEc2, prob0, totalCharge.charge());
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }

  int BCID =
    static_cast<int>(std::floor((G4Time +
                            m_timeOffset + timeWalk) / m_bunchSpace));

  return BCID;
}

int FEI3SimTool::relativeBunch2022(const SiTotalCharge& totalCharge, const double tot, int barrel_ec, int layer_disk, int moduleID,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
                                   
  static constexpr size_t nModules=7;
  
  /**
   * 2023.03.07  Taken from timing scan data in 2022.
   */
  
  double prob = 0.0;
  const bool isBarrel = (barrel_ec == 0);
  const bool isEndcap = (std::abs(barrel_ec) == 2);
  //is there any other possibility, or does !isBarrel == isEndcap ?
  if (isBarrel && layer_disk==1) {    // b-layer
    static constexpr size_t nCalibrationPoints=9;
    static constexpr std::array<double, nCalibrationPoints> totCharges{
      4.5, 5.5, 6.5, 
      7.5, 8.5, 9.5, 
      10.5, 11.5, 12.5
    };
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.0670, 0.0311, 0.0110, 0.0098, 0.0106, 0.0072, 0.0061, 0.0072, 0.0089},
      {0.0275, 0.0123, 0.0099, 0.0098, 0.0084, 0.0079, 0.0042, 0.0033, 0.0037},
      {0.0684, 0.0275, 0.0218, 0.0142, 0.0111, 0.0077, 0.0068, 0.0062, 0.0051},
      {0.0536, 0.0134, 0.0101, 0.0081, 0.0074, 0.0051,0.0049, 0.0049, 0.0027},
      {0.0806, 0.0340, 0.0203,0.0225, 0.0198, 0.0121,0.0095, 0.0069, 0.0049},
      {0.0736, 0.0164, 0.0143,0.0113, 0.0091, 0.0079,0.0068, 0.0060, 0.0056},
      {0.1190, 0.0275, 0.0126,0.0125, 0.0083, 0.0076,0.0064, 0.0055, 0.0060}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, tot);
  }
  else if (isBarrel && layer_disk==2) {  // layer-1
    static constexpr size_t nCalibrationPoints=21;
    //use constexpr immediately-executed-lambda to initialise boundaries
    //to 21 incremental values from 5.5 to 25.5
    //in C++20, can probably use std::iota
    static constexpr auto totCharges = [&] {
      std::array<double, nCalibrationPoints> a{};
      for (size_t i = 0; i < nCalibrationPoints; ++i) {
          a[i] =5.5 + i;
      }
      return a;
    }();
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.3755, 0.0841, 0.0516, 
        0.0390, 0.0265, 0.0206, 
        0.0234, 0.0184, 0.0153, 
        0.0122, 0.0100, 0.0129,
        0.0109, 0.0088, 0.0134,
        0.0145, 0.0107, 0.0104,
        0.0123, 0.0070, 0.0065},
      {0.4535, 0.1277, 0.0626, 
        0.0443, 0.0341, 0.0288, 
        0.0270, 0.0224, 0.0227, 
        0.0196, 0.0145, 0.0163,
        0.0128, 0.0129, 0.0126,
        0.0144, 0.0118, 0.0091,
        0.0131, 0.0120, 0.0119},
      {0.5102, 0.1059, 0.0575,
        0.0411, 0.0309, 0.0333, 
        0.0274, 0.0258, 0.0209,
        0.0209, 0.0170, 0.0143,
        0.0141, 0.0164, 0.0145,
        0.0131, 0.0112, 0.0159,
        0.0128, 0.0097, 0.0088},
      {0.4122, 0.1038, 0.0567,
        0.0371, 0.0288, 0.0268,
        0.0211, 0.0238, 0.0223, 
        0.0166, 0.0161, 0.0175,
        0.0129, 0.0091, 0.0136,
        0.0126, 0.0133, 0.0087,
        0.0082, 0.0077, 0.0078},
      {0.4174, 0.0976, 0.0527,
        0.0403, 0.0329, 0.0245, 
        0.0254, 0.0240, 0.0228, 
        0.0203, 0.0151, 0.0143, 
        0.0171, 0.0124, 0.0140,
        0.0119, 0.0134, 0.0090, 
        0.0093, 0.0109, 0.0110},
      {0.4079, 0.0869, 0.0441,
        0.0348, 0.0308, 0.0249,
        0.0221, 0.0216, 0.0211,
        0.0213, 0.0184, 0.0163,
        0.0167, 0.0143, 0.0125,
        0.0111, 0.0124, 0.0139,
        0.0148, 0.0104, 0.0074},
      {0.4023, 0.1047, 0.0589,
        0.0439, 0.0332, 0.0292,
        0.0270, 0.0209, 0.0166,
        0.0192, 0.0194, 0.0191,
        0.0155, 0.0149, 0.0123,
        0.0117, 0.0113, 0.0103,
        0.0153, 0.0088, 0.0100}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, tot );
  }
  else if (isBarrel && layer_disk==3) {  // layer-2
    static constexpr size_t nCalibrationPoints=21;

    //21 incremental values from 5.5 to 25.5
    static constexpr auto totCharges = [&] {
      std::array<double, nCalibrationPoints> a{};
      for (size_t i = 0; i < nCalibrationPoints; ++i) {
          a[i] =5.5 + i;
      }
      return a;
    }();
    //
    static constexpr std::array<std::array<double,nCalibrationPoints>, nModules> allProbabilities{
      {{0.4812, 0.1637, 0.0945,
        0.0670, 0.0579, 0.0451, 
        0.0317, 0.0249, 0.0191,
        0.0270, 0.0227, 0.0190,
        0.0168, 0.0205, 0.0162,
        0.0179, 0.0228, 0.0169,
        0.0136, 0.0089, 0.0198},
      {0.5605, 0.1837, 0.1040,
        0.0654, 0.0465, 0.0391,
        0.0337, 0.0325, 0.0242, 
        0.0270, 0.0218, 0.0191,
        0.0172, 0.0149, 0.0158,
        0.0153, 0.0134, 0.0204,
        0.0142, 0.0131, 0.0158},
      {0.5681, 0.1665, 0.1006,
        0.0659, 0.0478, 0.0417, 
        0.0330, 0.0343, 0.0310, 
        0.0271, 0.0244, 0.0169,
        0.0191, 0.0223, 0.0211,
        0.0170, 0.0153, 0.0193,
        0.0194, 0.0146, 0.0141},
      {0.5725, 0.1725, 0.0918,
        0.0529, 0.0416, 0.0336,
        0.0261, 0.0273, 0.0241,
        0.0195, 0.0264, 0.0178,
        0.0201, 0.0191, 0.0200,
        0.0150, 0.0139, 0.0151,
        0.0104, 0.0121, 0.0111},
      {0.5377, 0.1785, 0.1089,
        0.0702, 0.0474, 0.0476,
        0.0392, 0.0308, 0.0318,
        0.0290, 0.0244, 0.0262,
        0.0228, 0.0163, 0.0158,
        0.0153, 0.0185, 0.0157,
        0.0165, 0.0112, 0.0095},
      {0.5317, 0.1626, 0.0897,
        0.0606, 0.0434, 0.0346,
        0.0290, 0.0289, 0.0268,
        0.0268, 0.0275, 0.0262,
        0.0239, 0.0222, 0.0216,
        0.0213, 0.0195, 0.0209,
        0.0150, 0.0186, 0.0125},
      {0.5822, 0.1635, 0.0925,
        0.0583, 0.0498, 0.0434,
        0.0303, 0.0267, 0.0253,
        0.0247, 0.0219, 0.0223,
        0.0195, 0.0141, 0.0118,
        0.0207, 0.0169, 0.0121,
        0.0117, 0.0118, 0.0102}}
    };
    const auto & probArray = allProbabilities.at(std::abs(moduleID));
    prob = getProbability(totCharges, probArray, tot );
  }
  else if (isEndcap) {  // Endcap
    static constexpr size_t nCalibrationPoints=21;
    //21 incremental values from 5.5 to 25.5
    static constexpr auto totCharges = [] {
      std::array<double, nCalibrationPoints> a{};
      for (size_t i = 0; i != nCalibrationPoints; ++i) {
          a[i] =5.5 + i;
      }
      return a;
    }();
    static constexpr std::array<double, nCalibrationPoints> probEc{
      0.4896, 0.1294, 0.0684,
      0.0491, 0.0379, 0.0350,
      0.0315, 0.0275, 0.0251,
      0.0246, 0.0221, 0.0186,
      0.0182, 0.0190, 0.0176,
      0.0133, 0.0127, 0.0107,
      0.0113, 0.0096, 0.0086
    };
    prob = getProbability(totCharges, probEc, tot);
  }

  double G4Time = getG4Time(totalCharge);
  double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

  double timeWalk = 0.0;
  if (rnd < prob) {
    timeWalk = 25.0;
  }
  int BCID = static_cast<int>(floor((G4Time+m_timeOffset+timeWalk)/m_bunchSpace));
  return BCID;
}
