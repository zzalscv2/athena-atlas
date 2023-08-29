/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/RpdSubtractCentroidTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "TMath.h"

namespace ZDC
{

struct RpdChannelData {
  int channel;
  float_t xposRel;
  float_t yposRel;
  uint16_t row;
  uint16_t col;
  float_t amplitude;
  float_t amplitudeSubtr;
  float_t pileupFrac;
  unsigned int status;
};

RpdSubtractCentroidTool::RpdSubtractCentroidTool(const std::string& name) :
  asg::AsgTool(name),
  m_name(name),
  m_init(false),
  m_validInput(false),
  m_runNumber(0),
  m_lumiBlock(0),
  m_xCenter({0, 0}),
  m_yCenter({0, 0}),
  m_xyRotAngle({0, 0}),
  m_yzRotAngle({0, 0}),
  m_xCentAvg({0, 0}),
  m_yCentAvg({0, 0}),
  m_status({1 << ValidBit, 1 << ValidBit}), // calculation is valid by default
  m_ampSum({0, 0}),
  m_ampSumSub({0, 0}),
  m_ampSub({
    std::vector<std::vector<float>>(m_nRows, std::vector<float>(m_nCols)),
    std::vector<std::vector<float>>(m_nRows, std::vector<float>(m_nCols))
  }),
  m_xCentUnsub({0, 0}),
  m_yCentUnsub({0, 0}),
  m_xCentUnsubCor({0, 0}),
  m_yCentUnsubCor({0, 0}),
  m_xCent({0, 0}),
  m_yCent({0, 0}),
  m_xCentCor({0, 0}),
  m_yCentCor({0, 0}),
  m_xCentRowUnsub({std::vector<float>(m_nRows, 0), std::vector<float>(m_nRows, 0)}),
  m_yCentColUnsub({std::vector<float>(m_nCols, 0), std::vector<float>(m_nCols, 0)}),
  m_xCentRow({std::vector<float>(m_nRows, 0), std::vector<float>(m_nRows, 0)}),
  m_yCentCol({std::vector<float>(m_nCols, 0), std::vector<float>(m_nCols, 0)}),
  m_xStdev({0, 0}),
  m_yStdev({0, 0}),
  m_xyCov({0, 0})
{
#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif

declareProperty("ZdcModuleContainerName", m_zdcModuleContainerName = "ZdcModules", "Location of ZDC processed data");
declareProperty("ZdcSumContainerName", m_zdcSumContainerName = "ZdcSums", "Location of ZDC processed sums");
declareProperty("WriteAux", m_writeAux = true);
declareProperty("AuxSuffix", m_auxSuffix = "");

declareProperty("MinZDCEnergy", m_minZDCEnergy = {0, 0}, "Minimum ZDC energy for which the analysis will be performed");
declareProperty("MaxZDCEnergy", m_maxZDCEnergy = {9999, 9999}, "Maximum ZDC energy for which the analysis will be performed");
declareProperty("MinSubAmp", m_minSubAmp = {-9999, -9999}, "Lowest value for a subtracted amplitude to be included in calculation");
declareProperty("SubAmpUnderflowFrac", m_subAmpUnderflowFrac = {1, 1}, "Lowest (most negative) value for a subtracted amplitude as fraction of the sum, if exceeded calculation invalid");
declareProperty("PileupMaxFrac", m_pileupMaxFrac = {1, 1}, "Largest fractional pileup allowed for any channel");

}

RpdSubtractCentroidTool::~RpdSubtractCentroidTool()
{
  ATH_MSG_DEBUG("Deleting RpdSubtractCentroidTool named " << m_name);
}

StatusCode RpdSubtractCentroidTool::initialize()
{
  // initialize read and write handle keys
  ATH_CHECK(m_eventInfoKey.initialize());
  m_xposRelKey = m_zdcModuleContainerName + ".xposRel" + m_auxSuffix;
  ATH_CHECK(m_xposRelKey.initialize());
  m_yposRelKey = m_zdcModuleContainerName + ".yposRel" + m_auxSuffix;
  ATH_CHECK(m_yposRelKey.initialize());
  m_rowKey = m_zdcModuleContainerName + ".row" + m_auxSuffix;
  ATH_CHECK(m_rowKey.initialize());
  m_colKey = m_zdcModuleContainerName + ".col" + m_auxSuffix;
  ATH_CHECK(m_colKey.initialize());
  m_RPDChannelAmplitudeKey = m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelAmplitudeKey.initialize());
  m_RPDChannelAmplitudeCalibKey = m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelAmplitudeCalibKey.initialize());
  m_RPDChannelPileupFracKey = m_zdcModuleContainerName + ".RPDChannelPileupFrac" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelPileupFracKey.initialize());
  m_RPDChannelStatusKey = m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelStatusKey.initialize());
  
  m_RPDStatusKey = m_zdcSumContainerName + ".RPDStatus" + m_auxSuffix;
  ATH_CHECK(m_RPDStatusKey.initialize());
  m_ZDCFinalEnergyKey = m_zdcSumContainerName + ".FinalEnergy" + m_auxSuffix;
  ATH_CHECK(m_ZDCFinalEnergyKey.initialize());
  m_ZDCStatusKey = m_zdcSumContainerName + ".Status" + m_auxSuffix;
  ATH_CHECK(m_rpdSubAmpKey.initialize());
  m_rpdSubAmpKey = m_zdcSumContainerName + ".rpdSubAmp" + m_auxSuffix;
  ATH_CHECK(m_rpdSubAmpSumKey.initialize());
  m_rpdSubAmpSumKey = m_zdcSumContainerName + ".rpdSubAmpSum" + m_auxSuffix;
  ATH_CHECK(m_ZDCStatusKey.initialize());
  m_xCentroidKey = m_zdcSumContainerName + ".xCentroid" + m_auxSuffix;
  ATH_CHECK(m_xCentroidKey.initialize());
  m_yCentroidKey = m_zdcSumContainerName + ".yCentroid" + m_auxSuffix;
  ATH_CHECK(m_yCentroidKey.initialize());
  m_xDetCentroidKey = m_zdcSumContainerName + ".xDetCentroid" + m_auxSuffix;
  ATH_CHECK(m_xDetCentroidKey.initialize());
  m_yDetCentroidKey = m_zdcSumContainerName + ".yDetCentroid" + m_auxSuffix;
  ATH_CHECK(m_yDetCentroidKey.initialize());
  m_xDetCentroidUnsubKey = m_zdcSumContainerName + ".xDetCentroidUnsub" + m_auxSuffix;
  ATH_CHECK(m_xDetCentroidUnsubKey.initialize());
  m_yDetCentroidUnsubKey = m_zdcSumContainerName + ".yDetCentroidUnsub" + m_auxSuffix;
  ATH_CHECK(m_yDetCentroidUnsubKey.initialize());
  m_xDetRowCentroidKey = m_zdcSumContainerName + ".xDetRowCentroid" + m_auxSuffix;
  ATH_CHECK(m_xDetRowCentroidKey.initialize());
  m_yDetColCentroidKey = m_zdcSumContainerName + ".yDetColCentroid" + m_auxSuffix;
  ATH_CHECK(m_yDetColCentroidKey.initialize());
  m_xDetRowCentroidStdevKey = m_zdcSumContainerName + ".xDetRowCentroidStdev" + m_auxSuffix;
  ATH_CHECK(m_xDetRowCentroidStdevKey.initialize());
  m_yDetColCentroidStdevKey = m_zdcSumContainerName + ".yDetColCentroidStdev" + m_auxSuffix;
  ATH_CHECK(m_yDetColCentroidStdevKey.initialize());
  m_centroidStatusKey = m_zdcSumContainerName + ".centroidStatus" + m_auxSuffix;
  ATH_CHECK(m_centroidStatusKey.initialize());

  if (m_writeAux && m_auxSuffix != "") {
    ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
  }

  m_init = true;
  
  return StatusCode::SUCCESS;
}

StatusCode RpdSubtractCentroidTool::recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer)
{
  if (moduleContainer.size() == 0) {
    // if no modules, do nothing
    return StatusCode::SUCCESS;
  }

  // initialize read handles from read handle keys
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) {
    return StatusCode::FAILURE;
  }
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> xposRelHandle(m_xposRelKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> yposRelHandle(m_yposRelKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, uint16_t> rowHandle(m_rowKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, uint16_t> colHandle(m_colKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelAmplitudeHandle(m_RPDChannelAmplitudeKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelAmplitudeCalibHandle(m_RPDChannelAmplitudeCalibKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelPileupFracHandle(m_RPDChannelPileupFracKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> rpdChannelStatusHandle(m_RPDChannelStatusKey);  
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> rpdSideStatusHandle(m_RPDStatusKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcFinalEnergyHandle(m_ZDCFinalEnergyKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcStatusHandle(m_ZDCStatusKey);

  m_runNumber = eventInfo->runNumber();
  m_lumiBlock = eventInfo->lumiBlock();
  
  std::array<std::vector<std::vector<RpdChannelData>>, 2> rpdChannelData = {
    std::vector<std::vector<RpdChannelData>>(4, std::vector<RpdChannelData>(4)),
    std::vector<std::vector<RpdChannelData>>(4, std::vector<RpdChannelData>(4))
  };
  std::array<unsigned int, 2> rpdSideStatus;
  std::array<unsigned int, 2> zdcSideStatus;
  std::array<float, 2> zdcFinalEnergy;

  ATH_MSG_DEBUG("Starting event processing");
  ATH_MSG_DEBUG("LB=" << m_lumiBlock);
  
  ATH_MSG_DEBUG("Processing modules");
  for (const auto zdcModule : moduleContainer) {
    if (zdcModule->zdcType() == 1) {
      //  This is RPD data in Run 3
      //
      // right now it is assumed that this tool will not be invoked unless there is a run 3 RPD
      // if (m_LHCRun < 3) continue; // type == 1 -> pixel data in runs 2 and 3, skip
      
      int side = -1;
      if (zdcModule->zdcSide() == -1) {
        side = 0;
      } else if (zdcModule->zdcSide() == 1) {
        side = 1;
      } else {
        ATH_MSG_WARNING("Invalid side value found for module number: " << zdcModule->zdcModule() << ", side value = " << side);
        continue;
      }
      
      const unsigned int &rpdChannel = zdcModule->zdcChannel();

      ATH_MSG_DEBUG("RPD side " << side << " chan " << rpdChannel);

      if (rpdChannel > 15) {
        //
        //  The data is somehow corrupt, spit out an error
        //
        ATH_MSG_WARNING("Invalid RPD channel found on side " << side << ", channel number = " << rpdChannel << ", skipping this module");
        continue;
      } else {
        const uint16_t &row = rowHandle(*zdcModule);
        const uint16_t &col = colHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).channel = rpdChannel;
        rpdChannelData.at(side).at(row).at(col).xposRel = xposRelHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).yposRel = yposRelHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).row = rowHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).col = colHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).amplitude = rpdChannelAmplitudeHandle(*zdcModule);
        // ... = RPDChannelAmplitudeCalibHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).pileupFrac = rpdChannelPileupFracHandle(*zdcModule);
        rpdChannelData.at(side).at(row).at(col).status = rpdChannelStatusHandle(*zdcModule);
      }
    }
  }

  for (const auto zdcSum: moduleSumContainer) {
    int side = -1;
    if (zdcSum->zdcSide() == -1) {
      side = 0;
    } else if (zdcSum->zdcSide() == 1) {
      side = 1;
    } else {
      ATH_MSG_WARNING("Invalid side value found for module number: " << zdcSum->zdcModule() << ", side value = " << side);
      continue;
    }
    rpdSideStatus.at(side) = rpdSideStatusHandle(*zdcSum);
    zdcSideStatus.at(side) = zdcStatusHandle(*zdcSum);
    zdcFinalEnergy.at(side) = zdcFinalEnergyHandle(*zdcSum);

  }

  // check values and set status bits accordingly
  for (int side : {0, 1}) {
    if (zdcSideStatus.at(side)) {
      m_status.at(side) |= 1 << ZDCValidBit;
    } else {
      // => centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    if (zdcFinalEnergy.at(side) > m_maxZDCEnergy.at(side)) {
      m_status.at(side) |= 1 << ExcessiveZDCEnergyBit;
      // => centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    if (zdcFinalEnergy.at(side) < m_minZDCEnergy.at(side)) {
      m_status.at(side) |= 1 << MinimumZDCEnergyBit;
      // => centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    bool rpdPileup = rpdSideStatus.at(side) & (1 << RPDDataAnalyzer::SideOutOfTimePileupBit);
    if (rpdPileup) {
      m_status.at(side) |= 1 << PileupBit;
    }
    bool rpdAnaValid = rpdSideStatus.at(side) & (1 << RPDDataAnalyzer::SideValidBit);
    if (rpdAnaValid) {
      m_status.at(side) |= 1 << RPDValidBit;
    } else {
      // => centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    // check channels of RPD
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        if (rpdChannelData.at(side).at(row).at(col).pileupFrac > m_pileupMaxFrac.at(side)) {
          m_status.at(side) |= 1 << ExcessivePileupBit;
          // => centroid calculation is invalid
          m_status.at(side) &= ~(1 << ValidBit);
        }
      }
    }
  }

  // do the subtraction
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        if (row == m_nRows - 1) {
          // top row -> nothing to subtract
          float amplitudeSubtr = rpdChannelData.at(side).at(row).at(col).amplitude;
          rpdChannelData.at(side).at(row).at(col).amplitudeSubtr = amplitudeSubtr;
          m_ampSub.at(side).at(row).at(col) = amplitudeSubtr;
        } else {
          // other rows -> subtract the tile above this one
          float amplitudeSubtr = rpdChannelData.at(side).at(row).at(col).amplitude - rpdChannelData.at(side).at(row + 1).at(col).amplitude;
          rpdChannelData.at(side).at(row).at(col).amplitudeSubtr = amplitudeSubtr;
          m_ampSub.at(side).at(row).at(col) = amplitudeSubtr;
        }
      }
    }
  }

  // check the results of subtraction
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        if (rpdChannelData.at(side).at(row).at(col).amplitudeSubtr < m_minSubAmp.at(side)) {
          rpdChannelData.at(side).at(row).at(col).amplitudeSubtr = 0;
        }
      }
    }
  }

  // calculate row, col, and total sums
  std::array<std::vector<float>, 2> rowSumsUnsub = {std::vector<float>(4, 0), std::vector<float>(4, 0)};
  std::array<std::vector<float>, 2> colSumsUnsub = {std::vector<float>(4, 0), std::vector<float>(4, 0)};
  std::array<std::vector<float>, 2> rowSums = {std::vector<float>(4, 0), std::vector<float>(4, 0)};
  std::array<std::vector<float>, 2> colSums = {std::vector<float>(4, 0), std::vector<float>(4, 0)};
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        rowSumsUnsub.at(side).at(row) += rpdChannelData.at(side).at(row).at(col).amplitude;
        rowSums.at(side).at(row) += rpdChannelData.at(side).at(row).at(col).amplitudeSubtr;
        colSumsUnsub.at(side).at(col) += rpdChannelData.at(side).at(row).at(col).amplitude;
        colSums.at(side).at(col) += rpdChannelData.at(side).at(row).at(col).amplitudeSubtr;
        m_ampSum.at(side) += rpdChannelData.at(side).at(row).at(col).amplitude;
        m_ampSumSub.at(side) += rpdChannelData.at(side).at(row).at(col).amplitudeSubtr;
      }
    }
  }

  // check for any zero values in sum -> avoid dividing by zero; negative total sum is also bad
  for (int side : {0, 1}) {
    if (m_ampSum.at(side) <= 0) {
      m_status.at(side) |= 1 << ZeroSumBit;
      // => unsub centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    if (m_ampSumSub.at(side) <= 0) {
      m_status.at(side) |= 1 << ZeroSumBit;
      // => centroid calculation is invalid
      m_status.at(side) &= ~(1 << ValidBit);
    }
    for (int row = 0; row < m_nRows; row++) {
      if (rowSumsUnsub.at(side).at(row) <= 0) {
        m_status.at(side) |= 1 << (ZeroSumRow0Bit + row);
      }
      if (rowSums.at(side).at(row) <= 0) {
        m_status.at(side) |= 1 << (ZeroSumRow0Bit + row);
      }
    }
    for (int col = 0; col < m_nCols; col++) {
      if (colSumsUnsub.at(side).at(col) <= 0) {
        m_status.at(side) |= 1 << (ZeroSumCol0Bit + col);
      }
      if (colSums.at(side).at(col) <= 0) {
        m_status.at(side) |= 1 << (ZeroSumCol0Bit + col);
      }
    }
  }

  // check for negative amplitudes as a fraction of total sum
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        const float &amplitudeSubtr = rpdChannelData.at(side).at(row).at(col).amplitudeSubtr;
        if (amplitudeSubtr < 0 && -amplitudeSubtr/m_ampSumSub.at(side) < m_subAmpUnderflowFrac.at(side)) {
          m_status.at(side) |= 1 << ExcessivePileupBit;
          // => centroid calculation is invalid
          m_status.at(side) &= ~(1 << ValidBit);
        }
      }
    }
  }

  // calculate centroid
  for (int side : {0, 1}) {
    for (int col = 0; col < m_nCols; col++) {
      if (m_ampSum.at(side) > 0) {
        m_xCentUnsub.at(side) += colSumsUnsub.at(side).at(col)*rpdChannelData.at(side).at(0).at(col).xposRel/m_ampSum.at(side);
      }
      if (m_ampSumSub.at(side) > 0) {
        m_xCent.at(side) += colSums.at(side).at(col)*rpdChannelData.at(side).at(0).at(col).xposRel/m_ampSumSub.at(side);
      }
    }  
  }
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      if (m_ampSum.at(side) > 0) {
        m_yCentUnsub.at(side) += rowSumsUnsub.at(side).at(row)*rpdChannelData.at(side).at(row).at(0).yposRel/m_ampSum.at(side);
      }
      if (m_ampSumSub.at(side) > 0) {
        m_yCent.at(side) += rowSums.at(side).at(row)*rpdChannelData.at(side).at(row).at(0).yposRel/m_ampSumSub.at(side);
      }
    }  
  }

  // calculate x centroid for each row and y centroid for each col
  for (int side : {0, 1}) {
    for (int row = 0; row < m_nRows; row++) {
      for (int col = 0; col < m_nCols; col++) {
        if (rowSumsUnsub.at(side).at(row) > 0) {
          m_xCentRowUnsub.at(side).at(row) += rpdChannelData.at(side).at(row).at(col).amplitude*rpdChannelData.at(side).at(row).at(col).xposRel/rowSumsUnsub.at(side).at(row);
        }
        if (rowSums.at(side).at(row) > 0) {
          m_xCentRow.at(side).at(row) += rpdChannelData.at(side).at(row).at(col).amplitudeSubtr*rpdChannelData.at(side).at(row).at(col).xposRel/rowSums.at(side).at(row);
        }
        if (colSumsUnsub.at(side).at(col) > 0) {
          m_yCentColUnsub.at(side).at(col) += rpdChannelData.at(side).at(row).at(col).amplitude*rpdChannelData.at(side).at(row).at(col).yposRel/colSumsUnsub.at(side).at(col);
        }
        if (colSums.at(side).at(col) > 0) {
          m_yCentCol.at(side).at(col) += rpdChannelData.at(side).at(row).at(col).amplitudeSubtr*rpdChannelData.at(side).at(row).at(col).yposRel/colSums.at(side).at(col);
        }
      }
    }
  }
  
  // calculate standard deviation of row x / col y centroids
  for (int side : {0, 1}) {
    m_xStdev.at(side) = TMath::RMS(m_xCentRow.at(side).begin(), m_xCentRow.at(side).end());
    m_yStdev.at(side) = TMath::RMS(m_yCentCol.at(side).begin(), m_yCentCol.at(side).end());
  }

  // use information from geometry (TODO: and calibration) to get centroid in beamline coordinates
  for (int side : {0, 1}) {
    float x = m_xCentUnsubCor.at(side);
    float y = m_yCentUnsubCor.at(side);
    m_xCentUnsubCor.at(side) = geometryCorrectionX(x, y, side);
    m_yCentUnsubCor.at(side) = geometryCorrectionY(x, y, side);
    x = m_xCentCor.at(side);
    y = m_yCentCor.at(side);
    m_xCentCor.at(side) = geometryCorrectionX(x, y, side);
    m_yCentCor.at(side) = geometryCorrectionY(x, y, side);
  }

  ATH_MSG_DEBUG("Finishing event processing");
  
  ATH_MSG_DEBUG("Adding variables with suffix=" + m_auxSuffix);

  // decorate zdc sum

  // initialize write handles from write handle keys
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<std::vector<float>>> rpdSubAmp(m_rpdSubAmpKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> rpdSubAmpSum(m_rpdSubAmpSumKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xCentroidHandle(m_xCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yCentroidHandle(m_yCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xDetCentroidHandle(m_xDetCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yDetCentroidHandle(m_yDetCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xDetCentroidUnsubHandle(m_xDetCentroidUnsubKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yDetCentroidUnsubHandle(m_yDetCentroidUnsubKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> xDetRowCentroidHandle(m_xDetRowCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> yDetColCentroidHandle(m_yDetColCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xDetRowCentroidStdevHandle(m_xDetRowCentroidStdevKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yDetColCentroidStdevHandle(m_yDetColCentroidStdevKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> centroidStatusHandle(m_centroidStatusKey);
  
  for (const auto zdcSum: moduleSumContainer) {
    int side = -1;
    if (zdcSum->zdcSide() == -1) {
      side = 0;
    } else if (zdcSum->zdcSide() == 1) {
      side = 1;
    } else {
      ATH_MSG_WARNING("Invalid side value found for module number: " << zdcSum->zdcModule() << ", side value = " << side);
      continue;
    }
    rpdSubAmp(*zdcSum) = m_ampSub.at(side);
    rpdSubAmpSum(*zdcSum) = m_ampSumSub.at(side);
    xCentroidHandle(*zdcSum) = m_xCentCor.at(side);
    yCentroidHandle(*zdcSum) = m_yCentCor.at(side);
    xDetCentroidHandle(*zdcSum) = m_xCent.at(side);
    yDetCentroidHandle(*zdcSum) = m_yCent.at(side);
    xDetCentroidUnsubHandle(*zdcSum) = m_xCentUnsub.at(side);
    yDetCentroidUnsubHandle(*zdcSum) = m_yCentUnsub.at(side);
    xDetRowCentroidHandle(*zdcSum) = m_xCentRow.at(side);
    yDetColCentroidHandle(*zdcSum) = m_yCentCol.at(side);
    xDetRowCentroidStdevHandle(*zdcSum) = m_xStdev.at(side);
    yDetColCentroidStdevHandle(*zdcSum) = m_yStdev.at(side);
    centroidStatusHandle(*zdcSum) = m_status.at(side);
  }

  return StatusCode::SUCCESS;
}

StatusCode RpdSubtractCentroidTool::reprocessZdc()
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Trying to retrieve " << m_zdcModuleContainerName);

  const xAOD::ZdcModuleContainer* zdcModules = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcModules, m_zdcModuleContainerName));

  const xAOD::ZdcModuleContainer* zdcSums = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcSums, m_zdcSumContainerName));

  ATH_CHECK(recoZdcModules(*zdcModules, *zdcSums));

  return StatusCode::SUCCESS;
}

float RpdSubtractCentroidTool::geometryCorrectionX(float x_rpd, float y_rpd, int side) {
  // correct for rotation of RPD about z axis (in xy plane)
  float x_beamline = x_rpd*TMath::Cos(m_xyRotAngle.at(side)) - y_rpd*TMath::Sin(m_xyRotAngle.at(side));
  // correct for offsest of RPD
  x_beamline += m_xCenter.at(side);
  return x_beamline;
}

float RpdSubtractCentroidTool::geometryCorrectionY(float x_rpd, float y_rpd, int side) {
  // correct for rotation of RPD about z axis (in xy plane)
  float y_beamline = x_rpd*TMath::Sin(m_xyRotAngle.at(side)) + y_rpd*TMath::Cos(m_xyRotAngle.at(side));
  // correct for rotation of RPD about x axis (in yz plane)
  y_beamline = y_beamline*TMath::Cos(m_yzRotAngle.at(side));
  // correct for offsest of RPD
  y_beamline += m_yCenter.at(side);
  return y_beamline;
}

} // namespace ZDC
