/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _CALIBRATIONNTUPLEANALYSISALG2_H
#define _CALIBRATIONNTUPLEANALYSISALG2_H

// c - c++
#include "string"


//athena
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ToolHandle.h" 

// AtlasCore //
#include "StoreGate/StoreGateSvc.h"

//MuonCalibStandAloneBase
#include "MuonCalibStandAloneBase/CalibSegmentPreparationTool.h"

// segment preparation tools

class MdtCalibOutputDbSvc;
class MdtCalibInputSvc;
/**
@calss CalibNtupleAnalysis
Portage of the calibration_programme as an Athena Algorithm
Read calibration N-tuples and performs the calibration of the MDT chambers
*/

class MdtCalibInputSvc;
class RegionSelectionSvc;

namespace MuonCalib {
class NtupleCalibrationTool;

class CalibNtupleAnalysisAlg2 : public Algorithm
	{
//===========================================
	public:
/** Algorithm Constructor */
	CalibNtupleAnalysisAlg2(const std::string& name, ISvcLocator* pSvcLocator);
/** Algorithm destrucrtor*/
	~CalibNtupleAnalysisAlg2();
/** Is called at the beginning of the analysis */
	StatusCode initialize();
/** execute function NOTE: This will read all events at once*/
	StatusCode execute();	
/** finalize functions */
	StatusCode finalize();
//============================================
	private:	
//! segment preparation tools
	ToolHandleArray<CalibSegmentPreparationTool>  m_seg_prep_tool_handles;
	ToolHandle<NtupleCalibrationTool> m_calib_tool_handle;
//! number of segments to process
	int m_num_segments;
//!output db service
	MdtCalibOutputDbSvc *p_dbservice;
//! calibration input service
	MdtCalibInputSvc *p_calib_input_svc;
//! Calibration tool
	NtupleCalibrationTool *p_calib_tool;
//! Crash job, if no segments are read
	bool m_crash_if_no_segments;
//! stored segments
	std::vector<MuonCalibSegment *> m_stored_segment;
//!internal event counter
	int m_eventnumber;
	};
}

#endif
