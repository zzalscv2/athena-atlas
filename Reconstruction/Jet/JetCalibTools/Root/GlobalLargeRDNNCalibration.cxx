/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <TEnv.h>
#include <tuple>
#include <cmath>
#include <map>
#include <core/session/onnxruntime_cxx_api.h>

#include "JetCalibTools/JetCalibUtils.h"
#include "PathResolver/PathResolver.h"
#include "JetCalibTools/CalibrationMethods/GlobalLargeRDNNCalibration.h"


/// VarRetriever is a generic class to access Jet and/or JetEventInfo variables.
///  This class is expected to be placeholder in use while a proper configurable system to genericaly access variables is developed
struct GlobalLargeRDNNCalibration::VarRetriever{

  /// the value of the variable to be retrieved from the jet and/or JetEventInfo
  virtual float value(const  xAOD::Jet& jet, JetEventInfo& jetInfo, double eScale) = 0;
  virtual ~VarRetriever(){};
};

namespace {

  /// VarAccessorRetriever simply retrieves an attribute  from a jet 
  struct VarAccessorRetriever : public GlobalLargeRDNNCalibration::VarRetriever {
    VarAccessorRetriever(const std::string &n): m_acc(n) {}

    virtual float value(const xAOD::Jet& jet, JetEventInfo&, double eScale) {
      return m_acc(jet) * eScale;
    }
    
    SG::AuxElement::ConstAccessor<float> m_acc;
  };


// Define shortcuts macro to declare specialized VarRetriever class in one line
#define DEF_RETRIEVER0(cname, expr )  struct Var_##cname : public GlobalLargeRDNNCalibration::VarRetriever { float value(const xAOD::Jet& jet, JetEventInfo& , double eScale ) { return expr ; } }
#define DEF_RETRIEVER1(cname, expr )  struct Var_##cname : public GlobalLargeRDNNCalibration::VarRetriever { float value(const xAOD::Jet& , JetEventInfo& jetInfo, double eScale ) { return expr ; } }

  DEF_RETRIEVER0( eta, jet.eta()*eScale ) ;
  DEF_RETRIEVER0( log_e, log(jet.e()*eScale) ) ;
  DEF_RETRIEVER0( log_m, log(jet.m()*eScale) ) ;
  DEF_RETRIEVER0( m, jet.m()*eScale ) ;
  
  DEF_RETRIEVER1( mu, jetInfo.mu()*eScale );
  DEF_RETRIEVER1( NPV, jetInfo.NPV()*eScale );

#undef DEF_RETRIEVER

  /// buildVarRetriever creates and returns a specialized VarRetriever for the variable named name
  GlobalLargeRDNNCalibration::VarRetriever* buildVarRetriever(const std::string & name){

    // create a map of known specialized VarRetriever.
    // it's just a map "name" <-> function returning a Var_xyz()
    static const std::map<std::string, std::function<GlobalLargeRDNNCalibration::VarRetriever*()> > knownVar{
      {"eta",      [](){return new Var_eta();} },
      {"log_e",    [](){return new Var_log_e();} },
      {"log_m",    [](){return new Var_log_m();} },
      {"mu",       [](){return new Var_mu();} },
      {"NPV",      [](){return new Var_NPV();} },    
    };

    auto it = knownVar.find(name);
    //  if name is not a known variable, assume it's a jet attribute, so return a generic VarAccessorRetriever
    if( it == knownVar.end() ) return new VarAccessorRetriever(name);
    // else we just return an instance of a known VarRetriever class
    //  (it->second is  the function : we call it to obtain a new pointer)
    return it->second();
  }

}

GlobalLargeRDNNCalibration::GlobalLargeRDNNCalibration()
  : JetCalibrationStep::JetCalibrationStep("GlobalLargeRDNNCalibration/GlobalLargeRDNNCalibration"),
    m_config(nullptr), m_calibArea("")
{
}


// Constructor
GlobalLargeRDNNCalibration::GlobalLargeRDNNCalibration(const std::string& name)
  : JetCalibrationStep::JetCalibrationStep(name.c_str()),
    m_config(nullptr), m_calibArea("")
{
}

GlobalLargeRDNNCalibration::GlobalLargeRDNNCalibration(const std::string& name, TEnv * config, TString calibArea, bool /*dev*/)
  : JetCalibrationStep::JetCalibrationStep( name.c_str() ),
    m_config(config), m_calibArea(calibArea)
{
}

GlobalLargeRDNNCalibration::~GlobalLargeRDNNCalibration(){
  for(VarRetriever* v: m_varretrievers) delete v;
}

// Initialize
StatusCode GlobalLargeRDNNCalibration::initialize(){
  ATH_MSG_DEBUG("Initializing tool");
  if ( !m_config ) { ATH_MSG_FATAL("Config file not specified. Aborting."); return StatusCode::FAILURE; }

  // get list of input features
  m_NNInputs = JetCalibUtils::Vectorize( m_config->GetValue("DNNC.Inputs","") );
  // Now build a VarRetriever for each of the input features 
  m_varretrievers.resize(m_NNInputs.size());
  ATH_MSG_DEBUG("DNN inputs");
  for (long unsigned int i=0;i<m_NNInputs.size();i++) {
    m_varretrievers[i] = buildVarRetriever( m_NNInputs[i].Data() );
    ATH_MSG_DEBUG("  " << m_NNInputs[i]);
  }

  // get normalization constants for input features
  m_eScales = JetCalibUtils::VectorizeD( m_config->GetValue("DNNC.EScales","") );
  m_NormOffsets = JetCalibUtils::VectorizeD( m_config->GetValue("DNNC.NormOffsets","") );
  m_NormScales = JetCalibUtils::VectorizeD( m_config->GetValue("DNNC.NormScales","") );
    
  if (m_eScales.size()!=m_NNInputs.size() || m_NormOffsets.size()!=m_NNInputs.size() || m_NormScales.size()!=m_NNInputs.size()) {
    ATH_MSG_FATAL("Misconfiguration of config file : not same number of offset/scale parameters and number of features. Will exit");
    return StatusCode::FAILURE;
  }

  if( msgLvl(MSG::DEBUG) ){
    ATH_MSG_DEBUG("m_NormOffsets size : " << m_NormOffsets.size());
    ATH_MSG_DEBUG("m_NormOffsets");
    for (long unsigned int i=0;i<m_NormOffsets.size();i++) {
      ATH_MSG_DEBUG("  " << m_NormOffsets[i]);
    }
    ATH_MSG_DEBUG("m_NormScales size : " << m_NormScales.size());
    ATH_MSG_DEBUG("m_NormScales");
    for (long unsigned int i=0;i<m_NormScales.size();i++) {
      ATH_MSG_DEBUG("  " << m_NormScales[i]);
    }
  }
    
  // Get DNN config file
  m_modelFileName = m_config->GetValue("DNNC.ONNXInput","");
  std::string modelPath="JetCalibTools/"+m_calibArea+"CalibrationConfigs/"+m_modelFileName;
  const std::string fullModelPath = PathResolverFindCalibFile( modelPath ); // Full path
  ATH_MSG_INFO("Using ONNX model : " << m_modelFileName);
  ATH_MSG_INFO("resolved in: " << fullModelPath);

  // Set up the ONNX Runtime session.
  m_session = GlobalLargeRDNNCalibration::CreateORTSession(fullModelPath);
  ATH_MSG_DEBUG( "Created the ONNX Runtime session" );


  /************************** Input Nodes *****************************/
  /*********************************************************************/
  std::tuple<std::vector<int64_t>, std::vector<const char*> > inputInfo = GlobalLargeRDNNCalibration::GetInputNodeInfo(m_session);
  m_input_node_dims = std::get<0>(inputInfo);
  m_input_node_names = std::get<1>(inputInfo);

  if( msgLvl(MSG::DEBUG) ){
    for( std::size_t i = 0; i < m_input_node_names.size(); i++ ) {
      // print input node names
      ATH_MSG_DEBUG("Input "<<i<<" : "<<" name= "<<m_input_node_names[i]);
    
      // print input shapes/dims
      ATH_MSG_DEBUG("Input "<<i<<" : num_dims= "<<m_input_node_dims.size());
      for (std::size_t j = 0; j < m_input_node_dims.size(); j++){
        ATH_MSG_DEBUG("Input "<<i<<" : dim "<<j<<"= "<<m_input_node_dims[j]);
      }
    }
  }

  /************************** Output Nodes *****************************/
  /*********************************************************************/
  std::tuple<std::vector<int64_t>, std::vector<const char*> > outputInfo = GlobalLargeRDNNCalibration::GetOutputNodeInfo(m_session);
  m_output_node_dims = std::get<0>(outputInfo);
  m_output_node_names = std::get<1>(outputInfo);

  if( msgLvl(MSG::DEBUG) ){
    for( std::size_t i = 0; i < m_output_node_names.size(); i++ ) {
      // print input node names
      ATH_MSG_DEBUG("Output "<<i<<" : "<<" name= "<<m_output_node_names[i]);

      // print input shapes/dims
      ATH_MSG_DEBUG("Output "<<i<<" : num_dims= "<<m_output_node_dims.size());
      for (std::size_t j = 0; j < m_output_node_dims.size(); j++){
        ATH_MSG_DEBUG("Output "<<i<<" : dim "<<j<<"= "<<m_output_node_dims[j]);
      }
    }
  }

  /**************************************************************************************
   * m_input_node_dims[0] = -1; -1 needs to be replaced by the batch size; for no batch --> 1 
   * m_input_node_dims[1] = 21
   ****************************************************************************************/
  m_input_node_dims[0] = 1;
  m_output_node_dims[0] = 1;

  if (m_NNInputs.size()!=(long unsigned int)m_input_node_dims[1]) {
    ATH_MSG_FATAL("DNN input features not the same size as in config, will exit");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}



StatusCode GlobalLargeRDNNCalibration::calibrate(xAOD::Jet& jet, JetEventInfo& jetEventInfo) const {

  xAOD::JetFourMom_t jetStartP4 = jet.jetP4() ;

  // Get input features normalized for jet
  std::vector<float> input_tensor_values = getJetFeatures(jet, jetEventInfo);
  if( msgLvl(MSG::DEBUG) ){
    ATH_MSG_DEBUG("Input tensor values : ");
    for (long unsigned int i=0;i<input_tensor_values.size();i++) ATH_MSG_DEBUG(" " << input_tensor_values[i]);
  }

  // Check for nan or +/- inf
  int nNan = std::count_if(input_tensor_values.begin(), input_tensor_values.end(), [](float f){return std::isnan(f) || std::isinf(f);});
  if (nNan>0) {
    ATH_MSG_WARNING("Encountered Nan or inf value in input features, will not apply calibration");
    jet.setAttribute<xAOD::JetFourMom_t>("JetDNNCScaleMomentum",jetStartP4);
    return StatusCode::SUCCESS;
  }
  

  // Convert input_tensor_values array to onnx-compatiple tensor
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, 
                                                            input_tensor_values.data(), 
                                                            input_tensor_values.size(), 
                                                            m_input_node_dims.data(), 
                                                            m_input_node_dims.size());

  // Make sure we get the same tensors
  std::vector<float> vec(input_tensor.GetTensorMutableData<float>(), input_tensor.GetTensorMutableData<float>() + m_input_node_dims[1]);
  if (vec!=input_tensor_values) {
    ATH_MSG_WARNING("Input tensor after convertion to Ort tensor is not the same as the input vector");
    ATH_MSG_WARNING("Will not apply calibration");
    jet.setAttribute<xAOD::JetFourMom_t>("JetDNNCScaleMomentum",jetStartP4);
    return StatusCode::SUCCESS;
  }

  // Run inference on input_tensor
  Ort::Session& session ATLAS_THREAD_SAFE = *m_session;
  auto output_tensor =  session.Run(Ort::RunOptions{nullptr},
                                       m_input_node_names.data(),
                                       &input_tensor,
                                       m_input_node_names.size(),      
                                       m_output_node_names.data(),
                                       m_output_node_names.size());
  if (!output_tensor.front().IsTensor() || output_tensor.size() != m_output_node_names.size() || output_tensor.front().GetTensorTypeAndShapeInfo().GetShape() != m_output_node_dims) {
    ATH_MSG_WARNING("Output tensor does not have the same size as output layer, will not apply calibration");
    jet.setAttribute<xAOD::JetFourMom_t>("JetDNNCScaleMomentum",jetStartP4);
    return StatusCode::SUCCESS;
  }

  // Get pointer to output tensor float values
  float* outputE = output_tensor.at(0).GetTensorMutableData<float>();
  float* outputM = output_tensor.at(1).GetTensorMutableData<float>();

  // Apply calibration to jet p4
  float predRespE = outputE[0];  // first element is predicted response
  float predRespM = outputM[0];

  // Print the output predictions for E/M
  ATH_MSG_DEBUG("Output E : " << predRespE);
  ATH_MSG_DEBUG("Output M : " << predRespM);
  
  if (predRespE==0 || predRespM==0) {
    ATH_MSG_WARNING("Predictions give 0 values, will not apply calibration");
    jet.setAttribute<xAOD::JetFourMom_t>("JetDNNCScaleMomentum",jetStartP4);
    return StatusCode::SUCCESS;
  }

  float calibE = jetStartP4.e() / predRespE;
  float calibM = jetStartP4.mass() / predRespM;
  float calibpT = std::sqrt( calibE*calibE - calibM*calibM )/std::cosh( jetStartP4.eta() );

  TLorentzVector TLVjet;
  TLVjet.SetPtEtaPhiM( calibpT, jetStartP4.eta(), jetStartP4.phi(), calibM );
  xAOD::JetFourMom_t calibP4;
  calibP4.SetPxPyPzE( TLVjet.Px(), TLVjet.Py(), TLVjet.Pz(), TLVjet.E() );

  // Transfer calibrated jet properties to the Jet object
  jet.setAttribute<xAOD::JetFourMom_t>("JetDNNCScaleMomentum",calibP4);
  jet.setJetP4( calibP4 );

  return StatusCode::SUCCESS;

}



std::vector<float> GlobalLargeRDNNCalibration::getJetFeatures( xAOD::Jet& jet_reco, JetEventInfo& jetEventInfo) const {
  
  // Init input tensor
  std::vector<float> input_tensor_values(m_NNInputs.size());

  // retrieve all input variables from the jet and/or jetEventInfo using our VarRetriever collection :
  for(size_t i=0;i<input_tensor_values.size();i++){
    float v = m_varretrievers[i]->value(jet_reco, jetEventInfo, m_eScales[i]);
    // also perform normalisation :
    input_tensor_values[i] = v*m_NormScales[i] + m_NormOffsets[i];
  }

  return input_tensor_values;
}
