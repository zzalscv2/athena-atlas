/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADJETRNNEVALUATOR_H
#define DITAUREC_MUHADJETRNNEVALUATOR_H

#include <memory>

#include "tauRecTools/TauRecToolBase.h"

// xAOD includes
#include "xAODTau/TauJet.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

// Forward declarations
class TauJetRNN;

/**
 * @brief Modified from tauRecTools/TauJetRNNEvaluator by C. Deutsch & W. Davey
 *
 * @author Lainyou SHAN
 *
 */

class MuHadJetRNNEvaluator : public TauRecToolBase {
public:
    ASG_TOOL_CLASS2(MuHadJetRNNEvaluator, TauRecToolBase, ITauToolBase)

    MuHadJetRNNEvaluator(const std::string &name );

    virtual ~MuHadJetRNNEvaluator() override ;

    StatusCode initialize() override ;
    StatusCode execute(xAOD::TauJet &tau) override ;

    // Getter for the underlying RNN implementation
    TauJetRNN *get_rnn_1p();
    TauJetRNN *get_rnn_3p();

private:
    // Selects tracks to be used as input to the network
    StatusCode get_tracks(const xAOD::TauJet &tau,
                          std::vector<const xAOD::TauTrack *> &out);

    // Selects clusters to be used as input to the network
    StatusCode get_clusters(const xAOD::TauJet &tau,
                            std::vector<const xAOD::CaloCluster *> &out);

private:
    std::string m_output_varname;
    std::string m_weightfile_1p;
    std::string m_weightfile_3p;
    std::size_t m_min_charged_tracks;
    std::size_t m_max_tracks;
    std::size_t m_max_clusters;
    float m_max_cluster_dr;

    // Configuration of the weight file
    std::string m_input_layer_scalar;
    std::string m_input_layer_tracks;
    std::string m_input_layer_clusters;
    std::string m_output_layer;
    std::string m_output_node;

    // Wrappers for lwtnn
    std::unique_ptr<TauJetRNN> m_net_1p; 
    std::unique_ptr<TauJetRNN> m_net_3p; 

    bool m_classifierDone ;
    int m_isoTrackType;

};

#endif // TAUREC_TAUJETRNNEVALUATOR_H
