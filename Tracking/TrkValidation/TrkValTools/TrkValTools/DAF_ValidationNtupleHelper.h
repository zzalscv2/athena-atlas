/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// DAF_ValidationNtupleHelper.h
//   Header file for DAF_ValidationNtupleHelper
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_DAF_VALIDATIONNTUPLEHELPER_H
#define TRK_DAF_VALIDATIONNTUPLEHELPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkValInterfaces/IValidationNtupleHelperTool.h"
#include "TrkParameters/TrackParameters.h"

class TTree;

namespace Trk {

class IResidualPullCalculator;

/** @class DAF_ValidationNtupleHelper
    This validation tool writes information about Trk::CompetingRIOsOnTrack
    into an ntuple.
*/

class DAF_ValidationNtupleHelper : virtual public Trk::IValidationNtupleHelperTool,
  public AthAlgTool {
public:

    // standard AlgToolmethods
    DAF_ValidationNtupleHelper(const std::string&,const std::string&,const IInterface*);
    ~DAF_ValidationNtupleHelper();

    // standard Athena methods
    StatusCode initialize();
    StatusCode finalize();


    /** fill Trk::CompetingRIOsOnTrack data */
    virtual StatusCode fillMeasurementData (
        const Trk::MeasurementBase*,
        const Trk::TrackParameters*,
        const int& detectorType,
        const bool& isOutlier );

    /** fill special data about holes on track (here: do nothing) */
    virtual StatusCode fillHoleData (
        const Trk::TrackStateOnSurface&,
        const int& );

    /** add items to the ntuple and configure the helper tool:
        should be called once (per detector type) by the
        steering tool (Trk::ITrackValidationNtupleTool) */
    virtual StatusCode addNtupleItems (
        TTree* tree,
        const int& detectorType );

    /** reset ntuple variables */
    virtual StatusCode resetVariables (
        const int& detectorType );

private:
    
    static const unsigned int s_maxContainedROTs  = 8;   // maximal number of contained ROTs per CompetingRIOsOnTrack
    bool m_ignoreMissTrkCov;
    bool m_writeHitPositions; //!< jobOption: shall the positions of the contained ROTs be written?
    ToolHandle<Trk::IResidualPullCalculator>    m_residualPullCalculator;   //!< The residual and pull calculator tool
    

    int* m_isUnbiased;    

    std::vector<int>*   m_nContainedROTs;
    std::vector<int>*   m_indexOfMaxAssgnProb;
    std::vector<float>* m_maxAssgnProb;
    
};


} // end of namespace

#endif // TRK_DAF_VALIDATIONNTUPLEHELPER_H
