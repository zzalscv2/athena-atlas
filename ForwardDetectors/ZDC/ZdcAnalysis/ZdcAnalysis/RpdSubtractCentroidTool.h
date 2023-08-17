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
#include "ZdcAnalysis/ZDCMsg.h"

#include "xAODEventInfo/EventInfo.h"
#include "CxxUtils/checker_macros.h"

namespace ZDC
{

class ATLAS_NOT_THREAD_SAFE RpdSubtractCentroidTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{
  ASG_TOOL_CLASS(RpdSubtractCentroidTool, ZDC::IZdcAnalysisTool)

  enum {
    ValiBit = 0,             // the output for the given side is valid
	  ZDCValidBit = 1,         // the ZDC analysis on this siode was valid (i.e. not FAILED)
	  MinimumZDCEnergyBit = 2, // there was more than the minimum ZDC energy on this side
	  ExcessiveZDCEnergyBit = 3, // there was more than the maximum ZDC energy on this side
	  PileupBit = 4,             // pileup was detected on at least one channel
	  ExcessivePileupBit = 5,    // pileup exceeding specified limits on at leat one channel
	  SubtrUnderflowBit = 6     // the subtraction yielded excessively negative values in at least one channel
  };
    
public:
  RpdSubtractCentroidTool(const std::string& name);
  virtual ~RpdSubtractCentroidTool() override;

  //interface from AsgTool
  StatusCode initialize() override;
  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

private:

  // Data members
  //
  std::string m_name;
  bool m_init;

  // Configuration data, set to be properties of the tool
  //   as in the ZDC, we use side C = 0, side A = 1 for indexing 
  //
  std::array<float,2> m_minZDCEnergy; // the minimum ZDC energy for which the analysis will be performed
  std::array<float,2> m_maxZDCEnergy; // the maximum ZDC energy for which the analysis will be performed
  std::array<float,2> m_minSubAmp; // the lowest value for a subtracted amplitude to be included in calculation (otherwise taken to be zero?)
  std::array<float,2> m_subAmpUnderflowFrac; // the lowest (most negative) value for a subtracted amplitude as fraction of the sum, if exceeded calculation invalid 
  std::array<float,2> m_pileupMaxFrac; // the largest fractional pileup allowed for any channel
  
  bool m_writeAux;
  std::string m_auxSuffix;

  bool m_validInput;
  unsigned int m_runNumber;
  unsigned int m_lumiBlock;

  // Information from geometry
  //   as in the ZDC, we use side C = 0, side A = 1 for indexing 
  //
  std::array<float, 2> m_xCenter;    // The horizontal offset of the RPD relative to the 0 degree line
  std::array<float, 2> m_yCenter;    // The vertical offset of the RPD relative to the 0 degree line
  std::array<float, 2> m_xyRotAngle; // The rotation of the RPD in the x-y plane about its center
  std::array<float, 2> m_yzRotAngle; // The rotation of the RPD in the y-z plane about its center
    
  // Average x and y centroid positions - obtained from calibration file - to be subtracted from the per-event results
  //
  std::array<float, 2> m_xCentAvg;
  std::array<float, 2> m_yCentAvg;

  // Analysis results
  // ================
  std::array<unsigned int, 2> m_status;
  std::array<float, 2> m_ampSumSub; // the subtracted amplitude sum on the given side
  std::array<std::vector<std::vector<float> >, 2> m_ampSub; // the subtracted amplitude for each channel first index row, second column

  std::array<float, 2> m_xCentUnsub;  // x centroid, average not subtracted
  std::array<float, 2> m_yCentUnsub;  // y centroid, average not subtracted
  std::array<float, 2> m_xCent;  // x centroid, subtracted
  std::array<float, 2> m_yCent;  // y centroid, subtracted
  std::array<float, 2> m_xStdev; // x standard deviation
  std::array<float, 2> m_yStdev; // y standard deviation
  std::array<float, 2> m_xyCov;  // x-y covariance
  
  std::array<std::vector<float>, 2> m_xCentRow; // the x centroid for each row (diagnostic)
  std::array<std::vector<float>, 2> m_xCentCol; // the y centroid for each column (diagnostic)
  
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo", "Location of the event info."
  };

};

} // namespace ZDC

#endif



