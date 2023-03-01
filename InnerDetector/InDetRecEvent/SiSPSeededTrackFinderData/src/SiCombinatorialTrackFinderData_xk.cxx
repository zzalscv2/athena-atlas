/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// Source file for class SiCombinatorialTrackFinderData_xk
/////////////////////////////////////////////////////////////////////////////////

#include "SiSPSeededTrackFinderData/SiCombinatorialTrackFinderData_xk.h"
#include "AthenaKernel/getMessageSvc.h"

namespace InDet {

  SiCombinatorialTrackFinderData_xk::SiCombinatorialTrackFinderData_xk() = default;

  void SiCombinatorialTrackFinderData_xk::setTools(const Trk::IPatternParametersPropagator* propTool,
                                                   const Trk::IPatternParametersUpdator* updatorTool,
                                                   const Trk::IRIO_OnTrackCreator* rioTool,
                                                   const IInDetConditionsTool* pixCondTool,
                                                   const IInDetConditionsTool* sctCondTool,
                                                   const Trk::MagneticFieldProperties* fieldProp,
                                                   const Trk::IBoundaryCheckTool* boundaryCheckTool)
  {
    // Set SiTools and conditions
    //
    m_tools.setTools(propTool,
                     updatorTool,
                     rioTool);
    m_tools.setTools(pixCondTool,
                     sctCondTool);
    m_tools.setTools(fieldProp);    
    m_tools.setTools(boundaryCheckTool);

    // Set tool to trajectory
    //
    m_trajectory.setTools(&m_tools);
    
    m_initialized = true;
  }


  void SiCombinatorialTrackFinderData_xk::setFlagToReturnFailedTrack(const bool flag) {
    if( flag &&  (! m_simpleTrack) ) {
       MsgStream log(Athena::getMessageSvc(), "SiCombinatorialTrackFinderData_xk");
       log << MSG::WARNING << "not simpleTrack, keep flagToReturnFailedTrack as false" << endmsg;
       m_flagToReturnFailedTrack = false;
       return;
    }
    m_flagToReturnFailedTrack = flag;
  }

  void SiCombinatorialTrackFinderData_xk::setHeavyIon(bool flag){
    m_heavyIon = flag;
    m_tools.setHeavyIon(flag);
  }

  void SiCombinatorialTrackFinderData_xk::setITkGeometry(bool flag){
    m_ITkGeometry = flag;
    m_tools.setITkGeometry(flag);
  }

  void SiCombinatorialTrackFinderData_xk::setFastTracking(bool flag){
    m_doFastTracking = flag;
    m_tools.setFastTracking(flag);
  }

  bool SiCombinatorialTrackFinderData_xk::findPatternHoleSearchOutcome (Trk::Track* theTrack, InDet::PatternHoleSearchOutcome & outcome) const {
    auto found = m_holeSearchOutcomes.find(theTrack);
    if (found == m_holeSearchOutcomes.end()){
      return false; 
    }
    outcome = found->second; 
    return true; 
  }
  void SiCombinatorialTrackFinderData_xk::addPatternHoleSearchOutcome (Trk::Track* theTrack, const InDet::PatternHoleSearchOutcome & outcome){
    m_holeSearchOutcomes.emplace(theTrack,outcome); 
  }

} // end of name space
