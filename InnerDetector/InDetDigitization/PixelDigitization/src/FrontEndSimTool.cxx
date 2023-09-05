/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelDigitization/FrontEndSimTool.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetSimEvent/SiTotalCharge.h"

#include "SiDigitization/SiHelper.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"

#include "StoreGate/ReadCondHandleKey.h"
#include <limits>

FrontEndSimTool::FrontEndSimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  AthAlgTool(type, name, parent) {
  declareInterface<FrontEndSimTool>(this);
}

StatusCode 
FrontEndSimTool::initialize() {
  ATH_CHECK(m_pixelConditionsTool.retrieve());
  ATH_CHECK(m_pixelReadout.retrieve());
  ATH_CHECK(m_moduleDataKey.initialize());
  ATH_CHECK(m_chargeDataKey.initialize());
  if (m_cosmics){
    m_numberOfBcid = 8;
  }
  
  return StatusCode::SUCCESS;
}

StatusCode 
FrontEndSimTool::finalize() {return StatusCode::FAILURE;}

void 
FrontEndSimTool::crossTalk(double crossTalk, SiChargedDiodeCollection& chargedDiodes) const{
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());
  SiChargedDiodeMap oldChargedDiodes = chargedDiodes.chargedDiodes();

  for (SiChargedDiodeIterator i_chargedDiode = oldChargedDiodes.begin(); i_chargedDiode != oldChargedDiodes.end();
       ++i_chargedDiode) {
    InDetDD::SiCellId diode = (*i_chargedDiode).second.diode();
    std::vector<InDetDD::SiCellId> neighbours;
    p_design->neighboursOfCell(diode, neighbours);
    for (std::vector<InDetDD::SiCellId>::const_iterator p_neighbour = neighbours.begin();
         p_neighbour != neighbours.end(); ++p_neighbour) {
      const double intersection = p_design->intersectionLength(diode, *p_neighbour);
      // add cross talk only if the intersection is non-zero
      // if the original pixel is at (col,row) then the intersection length is
      // (col+-1, row+-1) : 0 -> diagonal
      // (col   , row+-1) : 0.4 mm (or 0.6 if long pixel) pixel width  = 400um or 600um
      // (col+-1, row   ) : 0.05 mm , pixel height = 50um
      // intersection length is just the length of the contact surface between the two pixels
      if (intersection > 0) {
        // create a new charge:
        // Q(new) = Q*L*X
        //   Q = charge of source pixel
        //   L = intersection length [mm]
        //   X = crosstalk factor    [mm-1]
        const SiChargedDiode& siCharge = (*i_chargedDiode).second;
        SiCharge charge(siCharge.charge() * intersection* crossTalk,
                        siCharge.totalCharge().time(), SiCharge::diodeX_Talk, siCharge.totalCharge().particleLink());
        chargedDiodes.add(*p_neighbour, charge);
      }
    }
  }
  return;
}


void 
FrontEndSimTool::thermalNoise(double thermalNoise, SiChargedDiodeCollection& chargedDiodes,
                    CLHEP::HepRandomEngine* rndmEngine) const{
  for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
       i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
    SiCharge charge(thermalNoise * CLHEP::RandGaussZiggurat::shoot(rndmEngine), 0, SiCharge::noise);
    (*i_chargedDiode)->add(charge);
  }
  return;
}

void 
FrontEndSimTool::randomNoise(SiChargedDiodeCollection& chargedDiodes, const PixelModuleData *moduleData,
            const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine) const{
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
  int nNoise = CLHEP::RandPoisson::shoot(rndmEngine,
                                         p_design->numberOfCircuits() * p_design->columnsPerCircuit() * p_design->rowsPerCircuit() *
                                         moduleData->getNoiseOccupancy(barrel_ec,
                                                                       layerIndex) *
                                         static_cast<double>(moduleData->getNumberOfBCID(barrel_ec, layerIndex)));

  for (int i = 0; i < nNoise; i++) {
    int circuit = CLHEP::RandFlat::shootInt(rndmEngine, p_design->numberOfCircuits());
    int column = CLHEP::RandFlat::shootInt(rndmEngine, p_design->columnsPerCircuit());
    int row = CLHEP::RandFlat::shootInt(rndmEngine, p_design->rowsPerCircuit());
    if (row > 159 && p_design->getReadoutTechnology() == InDetDD::PixelReadoutTechnology::FEI3) {
      row += 8;
    } // jump over ganged pixels - rowsPerCircuit == 320 above

    InDetDD::SiReadoutCellId roCell(row, p_design->columnsPerCircuit() * circuit + column);
    Identifier noisyID = chargedDiodes.element()->identifierFromCellId(roCell);

    if (roCell.isValid()) {
      InDetDD::SiCellId diodeNoise = roCell;

      float x = static_cast<float>(CLHEP::RandFlat::shoot(rndmEngine, 0., 1.));  // returns double
      size_t bin{};
      const std::vector<float> &noiseShape = moduleData->getNoiseShape(barrel_ec, layerIndex);
      for (size_t j = 1; j < noiseShape.size(); j++) {
        if (x > noiseShape[j - 1] && x <= noiseShape[j]) {
          bin = j - 1;
          continue;
        }
      }
      float noiseToTm = bin + 1.5f;
      float noiseToT = CLHEP::RandGaussZiggurat::shoot(rndmEngine, noiseToTm, 1.f);
      if (noiseToT < 1.f) { continue; }  // throw away unphysical noise

      // protection to the overflow ToT, that depends on the sensor technology
      float overflowToT = std::numeric_limits<float>::max();
      if (p_design->getReadoutTechnology() == InDetDD::PixelReadoutTechnology::FEI4) {
        overflowToT = moduleData->getFEI4OverflowToT(barrel_ec, layerIndex);
      }
      else if (p_design->getReadoutTechnology()==InDetDD::PixelReadoutTechnology::FEI3) {
        overflowToT = moduleData->getFEI3Latency(barrel_ec, layerIndex);
      }
      noiseToT = std::min(noiseToT, overflowToT);

      InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(noisyID);
      float chargeShape = chargeCalibData->getCharge(type, moduleHash, circuit, noiseToT);

      chargedDiodes.add(diodeNoise, SiCharge(chargeShape, 0, SiCharge::noise));
    }
  }
  return;
}

void 
FrontEndSimTool::randomDisable(SiChargedDiodeCollection& chargedDiodes,
              const PixelModuleData *moduleData,
              CLHEP::HepRandomEngine* rndmEngine) const{
  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
  for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
       i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
    if (CLHEP::RandFlat::shoot(rndmEngine) < moduleData->getDisableProbability(barrel_ec, layerIndex)) {
      SiHelper::disabled(**i_chargedDiode, true, false);
    }
  }
  return;
}

double 
FrontEndSimTool::getG4Time(const SiTotalCharge& totalCharge) const {
  // If there is one single charge, return its time:
  if (totalCharge.chargeComposition().empty()) {
    return totalCharge.time();
  }

  auto p_charge = totalCharge.chargeComposition().begin();
  int findfirst = 0;
  SiCharge first = *p_charge;

  // Look for first charge which is not noise
  for (; p_charge != totalCharge.chargeComposition().end(); p_charge++) {
    if (p_charge->processType() != SiCharge::noise) {
      findfirst = 1;
      break;
    }
  }

  // if all charges were noise, return the time of the highest charge
  if (findfirst == 0) {
    return totalCharge.time();
  }

  // look for the earlist charge among the remaining non-noise charges:
  first = *p_charge;
  p_charge++;

  for (; p_charge != totalCharge.chargeComposition().end(); p_charge++) {
    if (p_charge->time() < first.time() && p_charge->processType() != SiCharge::noise) {
      first = *p_charge;
    }
  }
  return first.time();
}

