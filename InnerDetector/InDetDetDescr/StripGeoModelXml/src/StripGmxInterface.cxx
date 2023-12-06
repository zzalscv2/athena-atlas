/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StripGmxInterface.h"

#include <GaudiKernel/GaudiException.h>
#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetSimEvent/SiHitIdHelper.h>
#include <ReadoutGeometryBase/SiCommonItems.h>
#include <SCT_ReadoutGeometry/SCT_DetectorManager.h>
#include <SCT_ReadoutGeometry/StripBoxDesign.h>
#include <SCT_ReadoutGeometry/StripStereoAnnulusDesign.h>

#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecord.h>
#include <RDBAccessSvc/IRDBRecordset.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

namespace
{
constexpr int SCT_HitIndex{1};
}


namespace InDetDD
{

namespace ITk
{

StripGmxInterface::StripGmxInterface(SCT_DetectorManager *detectorManager,
                                     SiCommonItems *commonItems,
                                     WaferTree *waferTree)
  : AthMessaging("StripGmxInterface"),
    m_detectorManager(detectorManager),
    m_commonItems(commonItems),
    m_waferTree(waferTree)
{}


int StripGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  //
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers"

  int hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitId(SCT_HitIndex,
                                                            index["barrel_endcap"],
                                                            index["layer_wheel"],
                                                            index["eta_module"],
                                                            index["phi_module"],
                                                            index["side"]);

  ATH_MSG_DEBUG("Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                               << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
  ATH_MSG_DEBUG("hitIdOfWafer = " << std::hex << hitIdOfWafer << std::dec);
  ATH_MSG_DEBUG(" bec = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfWafer)
                << " lay = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfWafer)
                << " eta = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfWafer)
                << " phi = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfWafer)
                << " side = " << SiHitIdHelper::GetHelper()->getSide(hitIdOfWafer));
  return hitIdOfWafer;
}

int StripGmxInterface::splitSensorId(std::map<std::string, int> &index,
                                     std::pair<std::string, int> &extraIndex,
                                     std::map<std::string, int> &updatedIndex ) const
{
  //
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers"

  if (extraIndex.first != "eta_module") {
    ATH_MSG_FATAL("Base Identifier: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                  << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
    ATH_MSG_FATAL("Attempting to split "<< extraIndex.second);
    ATH_MSG_FATAL("Only splitting of eta_module supported for ITk strips!!!");
    return -1;
  }

  //add the required amount to the requested field
  updatedIndex = index;
  updatedIndex[extraIndex.first] += extraIndex.second;

  int hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitId(SCT_HitIndex,
                                                            index["barrel_endcap"],
                                                            index["layer_wheel"],
                                                            index["eta_module"] + extraIndex.second,
                                                            index["phi_module"],
                                                            index["side"]);

  ATH_MSG_DEBUG("Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                << index["eta_module"] + extraIndex.second << " " << index["phi_module"] << " " << index["side"]);
  ATH_MSG_DEBUG("hitIdOfWafer = " << std::hex << hitIdOfWafer << std::dec);
  ATH_MSG_DEBUG(" bec = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfWafer)
                << " lay = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfWafer)
                << " eta = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfWafer)
                << " phi = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfWafer)
                << " side = " << SiHitIdHelper::GetHelper()->getSide(hitIdOfWafer));
  return hitIdOfWafer;
}


void StripGmxInterface::addSensorType(const std::string& clas,
                                      const std::string& typeName,
                                      const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_DEBUG("addSensorType called for class " << clas << ", typeName " << typeName);

  if (clas == "SiStripBox") {
    makeSiStripBox(typeName, parameters);
  } else if (clas == "StereoAnnulus") {
    makeStereoAnnulus(typeName, parameters);
  } else { // To-do: add "Annulus"
    ATH_MSG_ERROR("addSensorType: unrecognised sensor class: " << clas);
    ATH_MSG_ERROR("No sensor design created");
  }
}


void StripGmxInterface::makeSiStripBox(const std::string &typeName,
                                       const std::map<std::string, std::string> &parameters)
{
  //
  // Get all parameters.
  //
  SiDetectorDesign::Axis stripDirection;
  SiDetectorDesign::Axis fieldDirection;
  double thickness{0.320};
  int readoutSide{1};
  InDetDD::CarrierType carrier(InDetDD::electrons);
  int nRows{1};
  double pitch{0.080};
  int nStrips{1280}; // Per row
  double length{25.0};

  std::string carrierString;
  getParameter(typeName, parameters, "carrierType", carrierString);
  if (carrierString == "electrons") {
    carrier = InDetDD::electrons;
  } else if (carrierString == "holes") {
    carrier = InDetDD::holes;
  } else {
    throw GaudiException("Parameter carrierType should be electrons or holes for " + typeName,
                         "StripGmxInterface::makeSiStripBox", StatusCode::FAILURE);
  }

  std::string readoutSideString;
  getParameter(typeName, parameters, "readoutSide", readoutSideString);
  if (readoutSideString == "+") {
    readoutSide = 1;
  } else if (readoutSideString == "-") {
    readoutSide = -1;
  } else {
    throw GaudiException("Parameter readoutSide should be + or - for " + typeName,
                         "StripGmxInterface::makeSiStripBox", StatusCode::FAILURE);
  }

  std::string fieldDirectionString;
  getParameter(typeName, parameters, "fieldDirection", fieldDirectionString);
  if (fieldDirectionString == "x") {
    fieldDirection = SiDetectorDesign::xAxis;
  } else if (fieldDirectionString == "y") {
    fieldDirection = SiDetectorDesign::yAxis;
  } else if (fieldDirectionString == "z") {
    fieldDirection = SiDetectorDesign::zAxis;
  } else {
    throw GaudiException("Parameter fieldDirection should be x, y, or z for " + typeName,
                         "StripGmxInterface::makeSiStripBox", StatusCode::FAILURE);
  }

  std::string stripDirectionString;
  getParameter(typeName, parameters, "stripDirection", stripDirectionString);
  if (stripDirectionString == "x") {
    stripDirection = SiDetectorDesign::xAxis;
  } else if (stripDirectionString == "y") {
    stripDirection = SiDetectorDesign::yAxis;
  } else if (stripDirectionString == "z") {
    stripDirection = SiDetectorDesign::zAxis;
  } else {
    throw GaudiException("Parameter stripDirection should be x, y, or z for " + typeName,
                         "StripGmxInterface::makeSiStripBox", StatusCode::FAILURE);
  }

  getParameter(typeName, parameters, "thickness", thickness);
  getParameter(typeName, parameters, "nRows", nRows);
  getParameter(typeName, parameters, "nStrips", nStrips);
  getParameter(typeName, parameters, "pitch", pitch);
  getParameter(typeName, parameters, "stripLength", length);

  //
  // Make Sensor Design and add to DetectorManager
  //

  int splitLevel{};
  if (checkParameter(typeName, parameters, "splitLevel", splitLevel)) {
    // start from middle of first strip row
    double initZShift = length * (-static_cast<double>(splitLevel) * 0.5 + 0.5);

    // now, the "Mother"...
    // This is a container for all the other designs, to allow navigation
    // between the different rows on a simulated sensor in the HITS
    auto motherDesign = std::make_unique<StripBoxDesign>(stripDirection,
                                                         fieldDirection,
                                                         thickness,
                                                         readoutSide,
                                                         carrier,
                                                         nRows,
                                                         nStrips,
                                                         pitch,
                                                         length);

    for (int i = 0; i< splitLevel; i++) {
      for (int side : {0,1}) { //need different additional shift transform per side...
        int sign = (side == 0) ? 1 : -1; //...because shift in different direction per side
        double zShift = sign * (initZShift + (i * length));

        auto design = std::make_unique<StripBoxDesign>(stripDirection,
                                                       fieldDirection,
                                                       thickness,
                                                       readoutSide,
                                                       carrier,
                                                       1, //single row
                                                       nStrips,
                                                       pitch,
                                                       length,
                                                       zShift);

        design->setMother(motherDesign.get());
        motherDesign->addChildDesign(i,design.get());

        std::string splitName = typeName + "_" + std::to_string(i) + "_" + std::to_string(side);
        m_geometryMap[splitName] = m_detectorManager->addDesign(std::move(design));
      }
    }

    // Add to map for addSensor routine
    m_motherMap[typeName] = motherDesign.get();
    m_detectorManager->addMotherDesign(std::move(motherDesign));
  } else { // no split level
    auto design = std::make_unique<StripBoxDesign>(stripDirection,
                                                   fieldDirection,
                                                   thickness,
                                                   readoutSide,
                                                   carrier,
                                                   nRows,
                                                   nStrips,
                                                   pitch,
                                                   length);

    // Add to map for addSensor routine
    m_geometryMap[typeName] = design.get();
    m_detectorManager->addDesign(std::move(design));
  }
}


void StripGmxInterface::makeStereoAnnulus(const std::string &typeName,
                                          const std::map<std::string, std::string> &parameters)
{
  //
  //  Get all parameters.
  //
  int readoutSide{1};
  SiDetectorDesign::Axis fieldDirection;
  SiDetectorDesign::Axis stripDirection;
  InDetDD::CarrierType carrier{InDetDD::electrons};
  double thickness{0.320};
  double stereoAngle{0.020};
  double centreR{500.};
  int nRows{1};
  std::vector <int> nStrips;
  std::vector<double> phiPitch;
  std::vector<double> startR;
  std::vector<double> endR;
  bool usePC{false}; // initialise to false

  std::string carrierString;
  getParameter(typeName, parameters, "carrierType", carrierString);
  if (carrierString == "electrons") {
    carrier = InDetDD::electrons;
  } else if (carrierString == "holes") {
    carrier = InDetDD::holes;
  } else {
    throw GaudiException("Parameter carrierType should be electrons or holes for " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  std::string readoutSideString;
  getParameter(typeName, parameters, "readoutSide", readoutSideString);
  if (readoutSideString == "+") {
    readoutSide = 1;
  } else if (readoutSideString == "-") {
    readoutSide = -1;
  } else {
    throw GaudiException("Parameter readoutSide should be + or - for " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  std::string fieldDirectionString;
  getParameter(typeName, parameters, "fieldDirection", fieldDirectionString);
  if (fieldDirectionString == "x") {
    fieldDirection = SiDetectorDesign::xAxis;
  } else if (fieldDirectionString == "y") {
    fieldDirection = SiDetectorDesign::yAxis;
  } else if (fieldDirectionString == "z") {
    fieldDirection = SiDetectorDesign::zAxis;
  } else {
    throw GaudiException("Parameter fieldDirection should be x, y, or z for " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  std::string stripDirectionString;
  getParameter(typeName, parameters, "stripDirection", stripDirectionString);
  if (stripDirectionString == "x") {
    stripDirection = SiDetectorDesign::xAxis;
  } else if (stripDirectionString == "y") {
    stripDirection = SiDetectorDesign::yAxis;
  } else if (stripDirectionString == "z") {
    stripDirection = SiDetectorDesign::zAxis;
  } else {
    throw GaudiException("Parameter stripDirection should be x, y, or z for " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  getParameter(typeName, parameters, "thickness", thickness);
  getParameter(typeName, parameters, "stereoAngle", stereoAngle);
  getParameter(typeName, parameters, "centreR", centreR);
  getParameter(typeName, parameters, "nRows", nRows);

  getParameters(typeName, parameters, "nStrips", nStrips);
  if (nStrips.size() != static_cast<size_t>(nRows)) {
    throw GaudiException("Wrong number of nStrips " + std::to_string(nStrips.size()) + " " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  getParameters(typeName, parameters, "phiPitch", phiPitch);
  if (phiPitch.size() != static_cast<size_t>(nRows)) {
    throw GaudiException("Wrong number of pitches " + std::to_string(phiPitch.size()) + " " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  getParameters(typeName, parameters, "startR", startR);
  if (startR.size() != static_cast<size_t>(nRows)) {
    throw GaudiException("Wrong number of startRs " + std::to_string(startR.size()) + " " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  getParameters(typeName, parameters, "endR", endR);
  if (endR.size() != static_cast<size_t>(nRows)) {
    throw GaudiException("Wrong number of endRs " + std::to_string(endR.size()) + " " + typeName,
                         "StripGmxInterface::makeStereoAnnulus", StatusCode::FAILURE);
  }

  if (checkParameter(typeName, parameters, "usePC", usePC)) ATH_MSG_INFO("Using polar co-ordinates for strip stereo annulus modules");

  //
  //  Make Sensor Design and add it to the DetectorManager
  //
  std::vector<int> singleRowStrips;
  std::vector<double> singleRowPitch;
  std::vector<double> singleRowMinR;
  std::vector<double> singleRowMaxR;

  int splitLevel{};
  if (checkParameter(typeName, parameters, "splitLevel", splitLevel)) {
    // now the mother...
    auto motherDesign = std::make_unique<StripStereoAnnulusDesign>(stripDirection,
                                                                   fieldDirection,
                                                                   thickness,
                                                                   readoutSide,
                                                                   carrier,
                                                                   nRows,
                                                                   nStrips,
                                                                   phiPitch,
                                                                   startR,
                                                                   endR,
                                                                   stereoAngle,
                                                                   centreR,
                                                                   usePC);

    for (int i = 0; i < splitLevel; i++) {
      singleRowStrips.clear();
      singleRowPitch.clear();
      singleRowMinR.clear();
      singleRowMaxR.clear();

      singleRowStrips.push_back(nStrips[i]);
      singleRowPitch.push_back(phiPitch[i]);
      singleRowMinR.push_back(startR[i]);
      singleRowMaxR.push_back(endR[i]);
    //"shift" radius for correcting local<->global transforms
    //centreR remains the relevant radius for bounds/stereo calculations
    //since the strip frame is defined per wafer not per row
      double thisCentreR = (singleRowMinR[0] + singleRowMaxR[0] ) *0.5;

      auto design = std::make_unique<StripStereoAnnulusDesign>(stripDirection,
                                                               fieldDirection,
                                                               thickness,
                                                               readoutSide,
                                                               carrier,
                                                               1,//this design represents a single row by definition
                                                               singleRowStrips,
                                                               singleRowPitch,
                                                               singleRowMinR,
                                                               singleRowMaxR,
                                                               stereoAngle,
                                                               thisCentreR,
                                                               centreR,
                                                               usePC);

      // Add to map for addSensor routine
      std::string splitName = typeName + "_" + std::to_string(i);
      design->setMother(motherDesign.get());
      motherDesign->addChildDesign(i,design.get());

      m_geometryMap[splitName] = design.get();
      m_detectorManager->addDesign(std::move(design));

    }

    // finally, declare to the manager (now becomes const)
    m_motherMap[typeName] = motherDesign.get();
    m_detectorManager->addMotherDesign(std::move(motherDesign));

  } else {
    auto design = std::make_unique<StripStereoAnnulusDesign>(stripDirection,
                                                             fieldDirection,
                                                             thickness,
                                                             readoutSide,
                                                             carrier,
                                                             nRows,
                                                             nStrips,
                                                             phiPitch,
                                                             startR,
                                                             endR,
                                                             stereoAngle,
                                                             centreR,
                                                             usePC);

    m_geometryMap[typeName] = design.get();
    m_detectorManager->addDesign(std::move(design));
  }
}

void StripGmxInterface::addSplitSensor(const std::string& typeName,
                                       std::map<std::string, int> &index,
                                       std::pair<std::string, int> &extraIndex,
                                       int /* hitIdOfWafer */,
                                       GeoVFullPhysVol *fpv,
				       int /* splitLevel */)
{
  std::map<std::string, int> updatedIndex;
  splitSensorId(index,extraIndex,updatedIndex);
  int splitIndex = extraIndex.second;
  //
  // Get the ATLAS "Offline" wafer identifier
  //
  const SCT_ID *sctIdHelper = dynamic_cast<const SCT_ID *> (m_commonItems->getIdHelper());
  Identifier id = sctIdHelper->wafer_id(updatedIndex["barrel_endcap"],
                                        updatedIndex["layer_wheel"],
                                        updatedIndex["phi_module"],
                                        updatedIndex["eta_module"],
                                        updatedIndex["side"]);
  IdentifierHash hashId = sctIdHelper->wafer_hash(id);
  //
  // Now do our best to check if this is a valid id. If either the gmx file is wrong, or the xml file
  // defining the allowed id's is wrong, you can get disallowed id's. These cause a crash later
  // if allowed through. To do the check, we ask for the hash-id of this id. Invalid ids give a
  // special invalid hash-id (0xFFFFFFFF). But we don't exit the run, to help debug things quicker.
  //
  if (!hashId.is_valid()) {
    ATH_MSG_ERROR("Invalid id for sensitive wafer " << typeName << " volume with indices");
    for (const auto& [key, value] : index) {
      msg() << MSG::ERROR << key << " = " << value << "; ";
    }
    msg() << MSG::ERROR << endmsg;
    ATH_MSG_ERROR("Refusing to make it into a sensitive element. Incompatible gmx and identifier-xml files.");
    return;
  }

  //
  // Create the detector element and add to the DetectorManager
  //
  std::string splitTypeName = typeName + "_" + std::to_string(splitIndex);
  if (updatedIndex["barrel_endcap"] == 0) { // only barrel-type have side dependence
    splitTypeName += "_" + std::to_string(updatedIndex["side"]);
  }

  auto it = m_geometryMap.find(splitTypeName);
  if(it == m_geometryMap.end() || it->second == nullptr) {
    ATH_MSG_ERROR("addSplitSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }
  const SiDetectorDesign *design = it->second;

  m_detectorManager->addDetectorElement(new SiDetectorElement(id, design, fpv, m_commonItems));

  //
  // Build up a map-structure for numerology
  //
  Wafer wafer((unsigned int) hashId);
  std::string errorMessage("");
  if (!m_waferTree->add(updatedIndex["barrel_endcap"],
                        updatedIndex["layer_wheel"],
                        updatedIndex["eta_module"],
                        updatedIndex["phi_module"],
                        updatedIndex["side"],
                        wafer,
                        errorMessage)) {
    ATH_MSG_ERROR(errorMessage);
  }

  return;
}


void StripGmxInterface::addSensor(const std::string& typeName,
                                  std::map<std::string, int> &index,
                                  int /* sensitiveId */,
                                  GeoVFullPhysVol *fpv)
{
  //
  // Get the ATLAS "Offline" wafer identifier
  //
  const SCT_ID *sctIdHelper = dynamic_cast<const SCT_ID *> (m_commonItems->getIdHelper());
  Identifier id = sctIdHelper->wafer_id(index["barrel_endcap"],
                                        index["layer_wheel"],
                                        index["phi_module"],
                                        index["eta_module"],
                                        index["side"]);
  IdentifierHash hashId = sctIdHelper->wafer_hash(id);
  //
  // Now do our best to check if this is a valid id. If either the gmx file is wrong, or the xml file
  // defining the allowed id's is wrong, you can get disallowed id's. These cause a crash later
  // if allowed through. To do the check, we ask for the hash-id of this id. Invalid ids give a
  // special invalid hash-id (0xFFFFFFFF). But we don't exit the run, to help debug things quicker.
  //
  if (!hashId.is_valid()) {
    ATH_MSG_ERROR("Invalid id for sensitive module " << typeName << " volume with indices");
    for (const auto& [key, value] : index) {
      msg() << MSG::ERROR << key << " = " << value << "; ";
    }
    msg() << MSG::ERROR << endmsg;
    ATH_MSG_ERROR("Refusing to make it into a sensitive element. Incompatible gmx and identifier-xml files.");
    return;
  }

  //
  // Create the detector element and add to the DetectorManager
  //
  auto it = m_geometryMap.find(typeName);
  if(it == m_geometryMap.end() || it->second == nullptr) {
    ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }
  const SiDetectorDesign *design = it->second;

  m_detectorManager->addDetectorElement(new SiDetectorElement(id, design, fpv, m_commonItems));

  //
  // Build up a map-structure for numerology
  //
  Wafer wafer((unsigned int) hashId);
  std::string errorMessage("");
  if (!m_waferTree->add(index["barrel_endcap"],
                        index["layer_wheel"],
                        index["eta_module"],
                        index["phi_module"],
                        index["side"],
                        wafer,
                        errorMessage)) {
    ATH_MSG_ERROR(errorMessage);
  }

  return;
}


void StripGmxInterface::addAlignable(int level,
                                     std::map<std::string, int> &index,
                                     GeoVFullPhysVol *fpv,
                                     GeoAlignableTransform *transform)
{
  ATH_MSG_DEBUG("addAlignable called");
  //
  // Get the offline-id appropriate to the level (0 = wafer, 1 = module, 2 = wheel/cylinder, 3 = part, i.e barrel or an endcap)
  //
  const SCT_ID *sctIdHelper = dynamic_cast<const SCT_ID *> (m_commonItems->getIdHelper());
  Identifier id;
  switch (level) {
    case 0:
      id = sctIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 index["side"]);
      break;
    case 1:
      id = sctIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 0);
      break;
    case 2:
      id = sctIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 0,
                                 0,
                                 0);
      break;
    case 3:
      id = sctIdHelper->wafer_id(index["barrel_endcap"],
                                 0,
                                 0,
                                 0,
                                 0);
      break;
    default:
      throw GaudiException("Unknown level " + std::to_string(level) + " for alignment in addAlignable",
                           "StripGmxInterface::addAlignable", StatusCode::FAILURE);
      break;
  }
  m_detectorManager->addAlignableTransform(level, id, transform, fpv);
}

void StripGmxInterface::buildReadoutGeometryFromSqlite(IRDBAccessSvc * rdbAccessSvc,GeoModelIO::ReadGeoModel* sqlreader){

    IRDBRecordset_ptr stereoAnnulus = rdbAccessSvc->getRecordsetPtr("StereoAnnulus","");
    const std::array<std::string,13> stereoAnnulusParamNames({"thickness","carrierType","readoutSide","fieldDirection","stripDirection","stereoAngle","centreR","nRows","splitLevel","nStrips","phiPitch","startR","endR"});

    if(stereoAnnulus->size() !=0){
       for (unsigned int iR =0;iR<stereoAnnulus->size();iR++){
            std::map<std::string,std::string> stereoAnnulusMap;
            for(std::string paramName:stereoAnnulusParamNames){
                std::string paramValue = (*stereoAnnulus)[iR]->getString(paramName);
                stereoAnnulusMap[paramName] = paramValue;
            }
        std::string stereoAnnulusName = (*stereoAnnulus)[iR]->getString("SensorType");
        makeStereoAnnulus(stereoAnnulusName,stereoAnnulusMap);
        } 
    }
    else ATH_MSG_WARNING("Could not retrieve StereoAnnulus table");
    
    IRDBRecordset_ptr stripBox = rdbAccessSvc->getRecordsetPtr("SiStripBox","");
    const std::array<std::string,10> stripBoxParamNames({"thickness","carrierType","readoutSide","fieldDirection","stripDirection","nRows","stripLength","splitLevel","nStrips","pitch"});

    if(stripBox->size() !=0){
       for (unsigned int iR =0;iR<stripBox->size();iR++){
            std::map<std::string,std::string> stripBoxMap;
            for(std::string paramName:stripBoxParamNames){
                std::string paramValue = (*stripBox)[iR]->getString(paramName);
                stripBoxMap[paramName] = paramValue;
            }
        std::string stripBoxName = (*stripBox)[iR]->getString("SensorType");
        makeSiStripBox(stripBoxName,stripBoxMap);
        } 
    }
    else ATH_MSG_WARNING("Could not retrieve SiStripBox table");

    //Now, loop over the FullPhysVols and create the SiDetectorElements (including splitting where needed)
    //lots of string parsing...
    std::vector<std::string> fields({"barrel_endcap","layer_wheel","phi_module","eta_module","side"}); 
    //The below is a map of string keys which contain all the Identifier/DetElement relevant info, and the associated FullPhysVol
    std::map<std::string, GeoFullPhysVol*> mapFPV = sqlreader->getPublishedNodes<std::string, GeoFullPhysVol*>("GeoModelXML");
    for (const auto&[fullPhysVolInfoString, fullPhysVolPointer] : mapFPV){
        //find the name of the corresponding detector design type
        size_t startRG = fullPhysVolInfoString.find("RG_");
        if(startRG==std::string::npos){
            ATH_MSG_DEBUG("GeoFullPhysVol "<<fullPhysVolInfoString<<" does not have the expected format. Skipping");
            continue;
        } 
        std::string typeName = fullPhysVolInfoString.substr(startRG);
        std::map<std::string, int> index;
        for (std::string field:fields){
        size_t first = fullPhysVolInfoString.find(field+"_");
        size_t last = fullPhysVolInfoString.find("_",first+field.size()+1);//start looking only after end of first delimiter (plus 1 for the "_" appended) ends
        if(first==std::string::npos || last==std::string::npos){
            ATH_MSG_DEBUG("Could not extract "<<field<<" from "<<fullPhysVolInfoString<<". Skipping");
            continue;
        } 
        std::string strNew = fullPhysVolInfoString.substr(first+field.size()+1,last-(first+field.size()+1));
        index[field] = std::stoi(strNew);
        }
        //now check if we need to split
        size_t splitPos = fullPhysVolInfoString.find("split_");
        if(splitPos!=std::string::npos){
            size_t last = fullPhysVolInfoString.find("_",splitPos+6);//"split_" is 6 characters
            std::string strNew = fullPhysVolInfoString.substr(splitPos+6,last-(splitPos+6));
            int splitLevel = std::stoi(strNew);
            for(int i=0;i<splitLevel;i++){
              std::string field = "eta_module";//eventually specify in Xml the field to split in?
              std::pair<std::string,int> extraIndex(field,i);
              addSplitSensor(typeName,index,extraIndex,0,fullPhysVolPointer,splitLevel);
            }
        }
        else addSensor(typeName,index,0,fullPhysVolPointer);
    }
}

} // namespace ITk
} // namespace InDetDD
