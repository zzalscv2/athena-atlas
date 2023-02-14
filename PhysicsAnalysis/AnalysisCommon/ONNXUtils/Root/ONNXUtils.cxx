#include <MyAnalysis/ONNXUtils.h>

void ONNXWrapper::LoadModel(std::string model_path) {
    
    // Use the path resolver to find the location of the network .onnx file
    m_modelPath = PathResolverFindCalibFile(model_path);
    // if (m_modelPath == "") return StatusCode::FAILURE;

    //  init onnx envi
    m_onnxEnv = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_FATAL, "");

    // initialize session options if needed
    m_session_options.SetIntraOpNumThreads(1);
    m_session_options.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_EXTENDED);


    // Initialise the ONNX environment and session using the above options and the model name
    m_onnxSession = std::make_unique<Ort::Session>(*m_onnxEnv,
                                                   m_modelPath.c_str(),
                                                   m_session_options);

    // get the input nodes
    m_nr_inputs = m_onnxSession->GetInputCount();
    
    // get the output nodes
    m_nr_output = m_onnxSession->GetOutputCount();
    
    // std::cout << "------- input size: " << m_nr_inputs << "\n";

    // iterate over all input nodes
    for (std::size_t i = 0; i < m_nr_inputs; i++) {
      const char* input_name = m_onnxSession->GetInputName(i, m_allocator);

      m_input_names.push_back(input_name);

      m_input_dims.push_back(getShape(m_onnxSession->GetInputTypeInfo(i)));
    }
    // init input tensor
    // std::vector<Ort::Value> input_tensor(m_nr_inputs);

    // iterate over all output nodes
    for(std::size_t i = 0; i < m_nr_output; i++ ) {
      const char* output_name = m_onnxSession->GetOutputName(i, m_allocator);

      m_output_names.push_back(output_name);

      m_output_dims.push_back(getShape(m_onnxSession->GetOutputTypeInfo(i)));

      std::vector<float> output(m_output_dims[i][1], 0.0);
      m_outputs.push_back(output);
    }

    }

// void ONNXWrapper::Run(std::vector<std::vector<float>> inputs) { // ADDD custom input size
void ONNXWrapper::Run(std::map<std::string, std::vector<float>> inputs) { // ADDD custom input size
    // Create a CPU tensor to be used as input
    // std::cout << "------- Creating ONNX tensors: \n";
    Ort::MemoryInfo memory_info("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault);

    // define input tensor
    std::vector<Ort::Value> output_tensor;
    std::vector<Ort::Value> input_tensor;


    // add the inputs to vector
    for(std::size_t i = 0; i < m_nr_inputs; i++ ){          
      input_tensor.push_back(Ort::Value::CreateTensor<float>(memory_info,
                                                            inputs[m_input_names[i]].data(),
                                                            inputs[m_input_names[i]].size(),
                                                            m_input_dims[i].data(),
                                                            m_input_dims[i].size()));
    }

    // init output tensor and fill with zeros
    for(std::size_t i = 0; i < m_nr_output; i++ ){                           
      output_tensor.push_back(Ort::Value::CreateTensor<float>(memory_info,
                                                      m_outputs[i].data(),
                                                      m_outputs[i].size(),
                                                      m_output_dims[i].data(),
                                                      m_output_dims[i].size()));
    }

    Ort::Session& session ATLAS_THREAD_SAFE = *m_onnxSession;

    // run the model
    session.Run(Ort::RunOptions{nullptr},
                m_input_names.data(),
                input_tensor.data(),
                2,
                m_output_names.data(),
                output_tensor.data(),
                2);    
    }

std::vector<int64_t> ONNXWrapper::getShape(Ort::TypeInfo model_info) {
      auto tensor_info = model_info.GetTensorTypeAndShapeInfo();
      std::vector<int64_t> dims = tensor_info.GetShape();
      dims[0]=1;
      return dims;
    }