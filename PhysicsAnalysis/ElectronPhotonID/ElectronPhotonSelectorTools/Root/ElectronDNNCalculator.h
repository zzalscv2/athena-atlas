/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef ELECTRONDNNCALCULATOR_H
#define ELECTRONDNNCALCULATOR_H

// This include is needed at the top before any includes regarding lwtnn
// since it includes Eigen in a specific way which causes compilation errors
// if not included before lwtnn
#include "EventPrimitives/EventPrimitives.h"

#include "AsgMessaging/AsgMessagingForward.h"
#include "ElectronPhotonSelectorTools/AsgElectronSelectorTool.h"
#include "TTree.h"
#include <vector>
#include <string>
#include "lwtnn/generic/FastGraph.hh"


namespace MVAEnum{
  struct MVACalcVars{
    double eta;
    double et;
    double f3;
    double Rhad;
    double Rhad1;
    double Reta;
    double weta2;
    double f1;
    double Eratio;
    double deltaEta1;
    double d0;
    double qd0;
    double d0significance;
    double Rphi;
    double dPOverP;
    double deltaPhiRescaled2;
    double trans_TRTPID;
    double wtots1;
    double EoverP;
    double nPixHitsPlusDeadSensors;
    double nSCTHitsPlusDeadSensors;
    double SCTWeightedCharge;
  };

  struct QTVars{
    std::vector<double> eta;
    std::vector<double> et;
    std::vector<double> f3;
    std::vector<double> Rhad;
    std::vector<double> Rhad1;
    std::vector<double> Reta;
    std::vector<double> weta2;
    std::vector<double> f1;
    std::vector<double> Eratio;
    std::vector<double> deltaEta1;
    std::vector<double> d0;
    std::vector<double> qd0;
    std::vector<double> d0significance;
    std::vector<double> Rphi;
    std::vector<double> dPOverP;
    std::vector<double> deltaPhiRescaled2;
    std::vector<double> trans_TRTPID;
    std::vector<double> wtots1;
    std::vector<double> EoverP;
    std::vector<double> nPixHitsPlusDeadSensors;
    std::vector<double> nSCTHitsPlusDeadSensors;
    std::vector<double> SCTWeightedCharge; 
  };

}


class ElectronDNNCalculator : public asg::AsgMessagingForward
{
public:
  /** Constructor of the class*/
  ElectronDNNCalculator( AsgElectronSelectorTool* owner,
                         const std::string& modelFileName,
                         const std::string& quantileFileName,
                         const std::vector<std::string>& variablesName,
                         const bool multiClass,
                         const bool CFReject);

  /** Standard destructor*/
  ~ElectronDNNCalculator() {};

  /** Get the prediction of the DNN model*/
  Eigen::Matrix<float, -1, 1> calculate( const MVAEnum::MVACalcVars& varsStruct ) const;

private:
  /** transform the input variables according to a given QuantileTransformer.*/
  double transformInput( const std::vector<double>& quantiles, double value ) const;
  /** read the bins and values of the QuantileTransformer to transform the input variables.*/
  int readQuantileTransformer( TTree* tree, const std::vector<std::string>& variables );

  /// DNN interface via lwtnn
  std::unique_ptr<lwt::generic::FastGraph<float>> m_graph = 0;

  /// Quantile values for each variable that needs to be transformed with the QuantileTransformer
  MVAEnum::QTVars m_quantiles;
  /// Reference values for the QuantileTransformer. Basically just equidistant bins between 0 and 1.
  std::vector<double> m_references;
  /// Whether the used model is a multiclass model or not.
  bool m_multiClass;
  bool m_CFReject; 

};

#endif
