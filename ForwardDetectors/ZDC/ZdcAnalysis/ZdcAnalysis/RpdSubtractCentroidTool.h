/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H
#define ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H

#include <array>

#include "AsgTools/AsgTool.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "xAODForward/ZdcModuleContainer.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "ZdcAnalysis/ZDCMsg.h"

#include "xAODEventInfo/EventInfo.h"
#include "CxxUtils/checker_macros.h"

namespace ZDC
{

class RpdSubtractCentroidTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{
  ASG_TOOL_CLASS(RpdSubtractCentroidTool, ZDC::IZdcAnalysisTool)

  /**
   * The centroid calculation is invalid (ValidBit -> 0) if:
   * - ZDC or RPD analysis failed
   * - ZDC energy is out of bounds (min, max) set as tool property
   * - fractional pileup in a channel exceeds max set as tool property
   * - a subtracted amplitude is negative and fraction of sum exceeds max set as tool property
   * - an amplitude sum over all tiles was zero or negative
   * The centroid calculation is NOT invalidated if:
   * - there is pileup that does not exceed specified limit
   * - a subtracted amplitude is negative but fraction does not exceed specified underflow limit
   * - an amplitude sum over a row or column was zero or negative
   */
  enum {
    ValidBit = 0,              // the output for the given side is valid
	  ZDCValidBit = 1,           // the ZDC analysis on this side was valid, i.e. not failed
	  RPDValidBit = 2,           // the RPD analysis on this side was valid, i.e. not failed
	  MinimumZDCEnergyBit = 3,   // there was less than the minimum ZDC energy on this side
	  ExcessiveZDCEnergyBit = 4, // there was more than the maximum ZDC energy on this side
	  PileupBit = 5,             // pileup was detected on at least one channel
	  ExcessivePileupBit = 6,    // pileup exceeding specified limit on at least one channel
	  SubtrUnderflowBit = 7,     // the subtraction yielded excessively negative values in at least one channel
	  ZeroSumBit = 8,            // a total amplitude sum was zero/negative => undefined/nonsense centroid

    ZeroSumRow0Bit =  9, // amplitude sum over row 0 was zero/negative => undefined/nonsense row 0 x centroid
    ZeroSumRow1Bit = 10, // amplitude sum over row 1 was zero/negative => undefined/nonsense row 1 x centroid
    ZeroSumRow2Bit = 11, // amplitude sum over row 2 was zero/negative => undefined/nonsense row 2 x centroid
    ZeroSumRow3Bit = 12, // amplitude sum over row 3 was zero/negative => undefined/nonsense row 3 x centroid
    ZeroSumCol0Bit = 13, // amplitude sum over col 0 was zero/negative => undefined/nonsense col 0 y centroid
    ZeroSumCol1Bit = 14, // amplitude sum over col 1 was zero/negative => undefined/nonsense col 1 y centroid
    ZeroSumCol2Bit = 15, // amplitude sum over col 2 was zero/negative => undefined/nonsense col 2 y centroid
    ZeroSumCol3Bit = 16, // amplitude sum over col 3 was zero/negative => undefined/nonsense col 3 y centroid
  };
    
public:
  RpdSubtractCentroidTool(const std::string& name);
  virtual ~RpdSubtractCentroidTool() override;

  //interface from AsgTool
  StatusCode initialize() override;
  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

private:
  // Private methods
  //
  /**
   * @brief Calculate the x position in beamline coordinates from a position in RPD detector
   * coordinates using the offset of the RPD center and the rotation of the RPD plane.
   * 
   * @param x_rpd x position in RPD detector coordinates
   * @param y_rpd y position in RPD detector coordinates
   * @param side side of RPD (C = 0, A = 1)
   * @return float x position in beamline coordinates
   */
  float geometryCorrectionX(float x_rpd, float y_rpd, int side) const;
  /**
   * @brief Calculate the y position in beamline coordinates from a position in RPD detector
   * coordinates using the offset of the RPD center and the rotation of the RPD plane.
   * 
   * @param x_rpd x position in RPD detector coordinates
   * @param y_rpd y position in RPD detector coordinates
   * @param side side of RPD (C = 0, A = 1)
   * @return float y position in beamline coordinates
   */
  float geometryCorrectionY(float x_rpd, float y_rpd, int side) const;

  // Data members
  //
  std::string m_name;
  bool m_init;

  const int m_nRows = 4;
  const int m_nCols = 4;

  // Configuration data, set to be properties of the tool
  //   as in the ZDC, we use side C = 0, side A = 1 for indexing 
  //   note that ZDC side from ZdcSums container is C = -1, A = 1, but
  //     side C is mapped -1 -> 0 for indexing
  //
  std::vector<float> m_minZDCEnergy; // the minimum ZDC energy for which calculation is valid
  std::vector<float> m_maxZDCEnergy; // the maximum ZDC energy for which calculation is valid
  std::vector<float> m_minSubAmp; // the lowest value for a subtracted amplitude to be included in calculation, otherwise taken to be zero
  std::vector<float> m_subAmpUnderflowFrac; // the lowest (most negative) value for a subtracted amplitude as fraction of the sum, if exceeded calculation is invalid
  std::vector<float> m_pileupMaxFrac; // the largest fractional pileup allowed for any channel, if exceeded calculation is invalid
  
  bool m_writeAux;
  std::string m_auxSuffix;
  std::string m_zdcModuleContainerName;
  std::string m_zdcSumContainerName;

  // Information from geometry
  //   as in the ZDC, we use side C = 0, side A = 1 for indexing 
  /**
   * beamline coordinates are:
   *    +x points towards the center of the circle that is the LHC
   *    +y points vertically up
   *    +z points counterclockwise looking down => from side C = 0 to side A = 1
   * if the RPD rotation angles are zero, its +x and +y axes are aligned
   *    with those of the beamline
   * m_xCenter is the x offset of the center of the RPD relative to beamline +z,
   *    e.g. positive => RPD is too far towards inside of circle that is LHC
   * m_yCenter is the y offset of the center of the RPD relative to beamline +z,
   *    e.g. positive => RPD is too far vertically above beamline
   * m_xyRotAngle is the rotation of the RPD plane about its +z axis from nominal,
   *    its positive direction is given by the right hand rule,
   *    e.g. small and positive => +y axis of RPD is tilted towards the outside
   *    of the circle that is the LHC
   * m_yzRotAngle is the rotation of the RPD plane about the +x axis from nominal,
   *    its positive direction is given by the right hand rule,
   *    e.g. small and positive => +y axis of RPD is tilted towards beamline +z
   *    (for side C = 0, that is towards ATLAS, for side A = 1, that is away from ATLAS)
   */
  std::array<float, 2> m_xCenter;    // The horizontal offset of the RPD relative to the 0 degree line
  std::array<float, 2> m_yCenter;    // The vertical offset of the RPD relative to the 0 degree line
  std::array<float, 2> m_xyRotAngle; // The rotation of the RPD in the x-y plane about its center (in radians)
  std::array<float, 2> m_yzRotAngle; // The rotation of the RPD in the y-z plane about its center (in radians)
    
  
  // read handle keys
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo",
    "Location of the event info"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_xposRelKey {
    this, "xposRelKey", ""
    "X position of RPD tile center relative to center of RPD active area"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_yposRelKey {
    this, "yposRelKey", ""
    "Y position of RPD tile center relative to center of RPD active area"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_rowKey {
    this, "rowKey", ""
    "Row index of RPD channel"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_colKey {
    this, "colKey", ""
    "Column index of RPD channel"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {
    this, "RPDChannelAmplitudeKey", ""
    "RPD channel amplitude"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {
    this, "RPDChannelAmplitudeCalibKey", ""
    "Calibrated RPD channel amplitude"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupFracKey {
    this, "RPDChannelPileupFracKey", ""
    "RPD channel (out of time) pileup as a fraction of non-pileup sum"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {
    this, "RPDChannelStatusKey", ""
    "RPD channel status word"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDStatusKey {
    this, "RPDStatusKey", ""
    "RPD side status word"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCFinalEnergyKey {
    this, "FinalEnergyKey", ""
    "ZDC final energy"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCStatusKey {
    this, "StatusKey", ""
    "ZDC sum status word"
  };
  
  // write handle keys
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdSubAmpKey {
    this, "rpdSubAmpKey", ""
    "Subtracted RPD amplitudes, index row then column"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdSubAmpSumKey {
    this, "rpdSubAmpSumKey", ""
    "Sum of subtracted RPD amplitudes"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xCentroidKey {
    this, "xCentroidKey", ""
    "X position of RPD centroid (in beamline coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yCentroidKey {
    this, "yCentroidKey", ""
    "Y position of RPD centroid (in beamline coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xDetCentroidKey {
    this, "xDetCentroidKey", ""
    "X position of RPD centroid in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yDetCentroidKey {
    this, "yDetCentroidKey", ""
    "Y position of RPD centroid in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xDetCentroidUnsubKey {
    this, "xDetCentroidUnsubKey", ""
    "X position of RPD centroid in RPD coordinates (before geometry corrections), calculated with unsubtracted amplitudes"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yDetCentroidUnsubKey {
    this, "yDetCentroidUnsubKey", ""
    "Y position of RPD centroid in RPD coordinates (before geometry corrections), calculated with unsubtracted amplitudes"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xDetRowCentroidKey {
    this, "xDetRowCentroidKey", ""
    "Row x centroids in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yDetColCentroidKey {
    this, "yDetColCentroidKey", ""
    "Column y centroids in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xDetRowCentroidStdevKey {
    this, "xDetRowCentroidStdevKey", ""
    "Standard deviation of row x centroids in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yDetColCentroidStdevKey {
    this, "yDetColCentroidStdevKey", ""
    "Standard deviation of column y centroids in RPD coordinates (before geometry corrections)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_centroidStatusKey {
    this, "centroidStatusKey", ""
    "Centriod calculation status word"
  };

};
} // namespace ZDC

#endif
