/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADAXISSETTER_H
#define DITAUREC_MUHADAXISSETTER_H

#include "tauRecTools/TauRecToolBase.h"
#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"


/**
 * @brief Set Tau "Detector Axis" and "Intermediate Axis". 
 * 
 *  Note that both axes starts from the barycenter of the cluster associated to the jet seed. 
 *  Then only the 4-vectors of clusters in a cone of dR around these barycenter are summed up, forming the new axis.
 *  For the "Intermediate Axis" the clusters are correct wrt tau vertex in this step (barycenter remains the same).
 *  Using this procedure, the axes are different from the original jet seed axis.
 * 
 * Modified from tauRecTools/TauAxisSetter by Margar Simonyan and Felix Friedrich
 *
 * @author Lianyou SHAN
 *                                                                              
 */

class MuHadAxisSetter : public TauRecToolBase {
public:

    MuHadAxisSetter(const std::string& name);
    ASG_TOOL_CLASS2(MuHadAxisSetter, TauRecToolBase, ITauToolBase);

    ~MuHadAxisSetter() override = default ;

    StatusCode initialize() override ;
    StatusCode eventInitialize() override ;
    StatusCode execute(xAOD::TauJet& pTau) override ;

    StatusCode eventFinalize() override { return StatusCode::SUCCESS; }

private:

    double m_clusterCone;
    /** 
     * enable cell origin correction 
     * eta and phi of the cells are corrected wrt to the origin of the tau vertex
     */
    bool m_doAxisCorrection;

    ToolHandle<CP::IMuonCalibrationAndSmearingTool> m_thMuonCalibrationTool;
    ToolHandle<CP::IMuonSelectionTool> m_thMuonSelectionTool;

};

#endif
