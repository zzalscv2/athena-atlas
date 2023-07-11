/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/* ***********************************************************************************\
 *                                                                                   *
 *      Name: GlobalLargeRDNNCalibration                                             *
 *      Purpose: Perform the DNN JES and JMS step of the large-R jets' calibration   *
 *                                                                                   *
 *  #   Date    Comments                   By                                        *
 * -- -------- -------------------------- ------------------------------------------ *
 *  1 31/01/23  First Version              G. Albouy, P.-A. Delsart                  * 
\*************************************************************************************/


#ifndef JetCalibTools_GlobalLargeRDNNCalibration_H
#define JetCalibTools_GlobalLargeRDNNCalibration_H

#include <string>
#include <vector>

//xAOD EDM classes
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

// Other packages includes
#include "AsgServices/ServiceHandle.h"
#include "AthOnnxruntimeService/IONNXRuntimeSvc.h"
// ONNX Runtime include(s).
#include <core/session/onnxruntime_cxx_api.h>

// Local includes
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCalibTools/JetCalibrationStep.h"

class GlobalLargeRDNNCalibration : virtual public ::JetCalibrationStep {
  
public:
  // Constructor/destructor/init
    /**
    @brief      The constructor. Note that this doesn't have all the necessary information, so it will not configure things correctly. 
    */
    GlobalLargeRDNNCalibration();      

    /**
    @brief      The constructor. Note that this doesn't have all the necessary information, so it will not configure things correctly. 
    @param name The name of the tool being created
    */
    GlobalLargeRDNNCalibration(const std::string& name);      

    /**
    @brief               The constructor, which is used by the JetCalibrationTool
    @param name          The name of the tool being created
    @param config        The name of the config file for the calibration
    @param jetAlgo       The name of the jet collection
    @param calibAreaTag  The tag for this calibration
    @param dev           A flag for if the calibration is run in development mode
    */
    GlobalLargeRDNNCalibration(const std::string& name, TEnv * config, TString calibArea, bool dev);

    /**
    @brief  The destructor
    */
    virtual ~GlobalLargeRDNNCalibration() ;

    /**
    @brief          Returns the charged fraction of a jet
    @param name The name of the tool being created
    */
    virtual StatusCode initialize() override;
  

    /// VarRetriever is a generic class to access Jet and/or JetEventInfo variables.
    ///  This class is expected to be placeholder in use while a proper configurable system to genericaly access variables is developed
    struct VarRetriever;

    protected:
    // @brief          Calibrates the jet, and decorates it with the calibration using the name "JetGNNCScaleMomentum"
    // @param jet_reco The jet
    // @param jetEventInfo A set of information about the event and jet
    virtual StatusCode calibrate(xAOD::Jet& jet, JetEventInfo&) const override;

    private:

    /**
    @brief Returns a vector of input features for the NN
    @param jet_reco  The jet
    @param jetEventInfo A set of information about the event and jet
    */
    std::vector<float> getJetFeatures( xAOD::Jet& jet_reco, JetEventInfo& jetEventInfo) const;

    /**
    @brief Create ORT sesssion from input model
    @param modelFile  string containing the model filename
    */
    inline std::unique_ptr< Ort::Session > CreateORTSession(const std::string& modelFile);
    /**
    @brief Get NN input layer info
    @param session  pointer to ORT session
    */
    inline  std::tuple<std::vector<int64_t>, std::vector<const char*> > GetInputNodeInfo(const std::unique_ptr< Ort::Session >& session);
    /**
    @brief Get NN output layer info
    @param session  pointer to ORT session
    */
    inline  std::tuple<std::vector<int64_t>, std::vector<const char*> > GetOutputNodeInfo(const std::unique_ptr< Ort::Session >& session);
  
    std::vector<TString> m_NNInputs;
    std::vector<double> m_eScales;
    std::vector<double> m_NormOffsets;
    std::vector<double> m_NormScales;
    std::string m_modelFileName;

    std::vector<VarRetriever*> m_varretrievers;
  
    std::unique_ptr< Ort::Session > m_session;
    std::vector<int64_t> m_input_node_dims;
    std::vector<const char*> m_input_node_names;
    std::vector<int64_t> m_output_node_dims;
    std::vector<const char*> m_output_node_names;
    
    TEnv * m_config;
    std::string m_calibArea;


}; // Class GlobalLargeRDNNCalibration


#endif

inline std::unique_ptr< Ort::Session > GlobalLargeRDNNCalibration::CreateORTSession(const std::string& modelFile){
   
  // Set up the ONNX Runtime session.
  Ort::SessionOptions sessionOptions;
  sessionOptions.SetIntraOpNumThreads( 1 );
  sessionOptions.SetGraphOptimizationLevel( ORT_ENABLE_BASIC );

  ServiceHandle< AthONNX::IONNXRuntimeSvc > svc("AthONNX::ONNXRuntimeSvc",
                                       "AthONNX::ONNXRuntimeSvc");

  return std::make_unique<Ort::Session>( svc->env(),
                                         modelFile.c_str(),
                                         sessionOptions );
}

inline  std::tuple<std::vector<int64_t>, std::vector<const char*> > GlobalLargeRDNNCalibration::GetInputNodeInfo(const std::unique_ptr< Ort::Session >& session){
    
  std::vector<int64_t> input_node_dims;
  size_t num_input_nodes = session->GetInputCount();
  std::vector<const char*> input_node_names(num_input_nodes);
  Ort::AllocatorWithDefaultOptions allocator;
  for( std::size_t i = 0; i < num_input_nodes; i++ ) {
    
      char* input_name = session->GetInputName(i, allocator);
      input_node_names[i] = input_name;
      Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
  
      input_node_dims = tensor_info.GetShape();  
    }
  return std::make_tuple(input_node_dims, input_node_names); 
}

inline  std::tuple<std::vector<int64_t>, std::vector<const char*> > GlobalLargeRDNNCalibration::GetOutputNodeInfo(const std::unique_ptr< Ort::Session >& session){
     
  //output nodes
  std::vector<int64_t> output_node_dims;
  size_t num_output_nodes = session->GetOutputCount();
  std::vector<const char*> output_node_names(num_output_nodes);
  Ort::AllocatorWithDefaultOptions allocator;

  for( std::size_t i = 0; i < num_output_nodes; i++ ) {
    char* output_name = session->GetOutputName(i, allocator);
    output_node_names[i] = output_name;

    Ort::TypeInfo type_info = session->GetOutputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

    output_node_dims = tensor_info.GetShape();
  }
  return std::make_tuple(output_node_dims, output_node_names);
}


