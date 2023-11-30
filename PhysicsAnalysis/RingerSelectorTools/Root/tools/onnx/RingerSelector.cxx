/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local includes:
#include "RingerSelectorTools/tools/onnx/RingerSelector.h"
#include "PathResolver/PathResolver.h"
#include <filesystem>
#include <algorithm>
#include "TEnv.h"



namespace Ringer{

  namespace onnx{


    //==============================================================================
    RingerSelector::RingerSelector(const std::string& name):
      asg::AsgMessaging(name)
    {;}
    

    //==============================================================================
    StatusCode RingerSelector::read_from( const std::string& path , AthONNX::IONNXRuntimeSvc *svc )
    {

      std::string configFile = PathResolverFindCalibFile( path );
      std::string basepath = GetBasePath( path );

      ATH_MSG_INFO( "Loading all tunings configs from " << configFile );
      ATH_MSG_INFO( "Basepath is "<< basepath);

      if (configFile.empty()){
         ATH_MSG_ERROR( "Could not locate the config file: " << configFile);
         return StatusCode::FAILURE; 
      }

      // Read configuration file
      TEnv env(configFile.c_str());
   

      // Retrieve all models
      {
        // Retrieve the size
        unsigned size = env.GetValue( "Model__size" , 0 );
        auto PreprocessingMode = GetValues<int>( "Model__barcode", env); // select the input preprocessing mode
          
        // Retrieve Et bins
        auto etmin = GetValues<float>( "Model__etmin", env );
        auto etmax  = GetValues<float>( "Model__etmax", env );
        
        // Retrieve Eta bins
        auto etamin = GetValues<float>( "Model__etamin", env );
        auto etamax  = GetValues<float>( "Model__etamax", env );
        
        // Retreive all ONNX model file paths
        auto model_paths = GetPaths( "Model__path", env );

        // Loop over all models
        for ( unsigned idx = 0; idx < size; ++idx )
        {
          std::string modelPath = PathResolverFindCalibFile( basepath+"/"+model_paths[idx] );
          ATH_MSG_DEBUG( "Reading Onnx model from: " << modelPath );
          auto model = Ringer::onnx::Model( modelPath, svc, etmin[idx], etmax[idx], etamin[idx], etamax[idx], PreprocessingMode[idx]) ;
          // Compile the model
          model.compile();
          m_models.push_back(model);
        }
      }

      // Retrieve all thresholds
      {
        // Retrieve the size
         unsigned size = env.GetValue( "Threshold__size" , 0);
         auto max_avgmu = GetValues<float>( "Threshold__MaxAverageMu", env );
         
         // Retrieve Et bins
         auto etmin = GetValues<float>( "Threshold__etmin", env );
         auto etmax = GetValues<float>( "Threshold__etmax", env );
         // Retrieve Eta bins
         auto etamin  = GetValues<float>( "Threshold__etamin", env );
         auto etamax  = GetValues<float>( "Threshold__etamax", env );
         // Retrieve slopes
         auto slopes = GetValues<float>( "Threshold__slope", env );
         // Retrieve offsets
         auto offsets = GetValues<float>("Threshold__offset", env );

         for ( unsigned idx = 0; idx <size; ++idx ){
           m_thresholds.push_back( Ringer::onnx::Threshold(
                                     etmin[idx],
                                     etmax[idx],
                                     etamin[idx],
                                     etamax[idx],
                                     slopes[idx],
                                     offsets[idx],
                                     max_avgmu[idx])
                                 );
          }
      }

      return StatusCode::SUCCESS;
    }


   
    //==============================================================================
    bool RingerSelector::accept( const xAOD::TrigRingerRings *ringsCluster, float discr, float avgmu ) const 
    {
      float et = ringsCluster->emCluster()->et();
      float eta = std::abs(ringsCluster->emCluster()->eta());
    
      ATH_MSG_DEBUG( "Event et = "<< et << ", eta = " << eta );
      for( auto& cutDef : m_thresholds ){
        if ( et < cutDef.etMin()*Gaudi::Units::GeV || et >= cutDef.etMax()*Gaudi::Units::GeV ) continue;
        if ( eta < cutDef.etaMin() || eta >= cutDef.etaMax() ) continue;
        return cutDef.accept( discr, avgmu );
      }// loop over all thresholds
    
      return false;
    }


    //==============================================================================
    float RingerSelector::predict(const xAOD::TrigRingerRings *ringsCluster , const xAOD::TrigElectron *el ) const
    {
      float et = ringsCluster->emCluster()->et();
      float eta = std::abs(ringsCluster->emCluster()->eta());

      // Find the correct model and predict
      for( auto& model : m_models ){
        
        if(et<model.etMin()*Gaudi::Units::GeV   || et  >= model.etMax()*Gaudi::Units::GeV ) continue;
        if(eta<model.etaMin() || eta >= model.etaMax()) continue;
        auto inputs = prepare_inputs( model.barcode(), ringsCluster, el );
        auto output = model.predict( inputs ); // propagate the input throut the model
        ATH_MSG_DEBUG( "The current model predict with output: " << output );
        return output;
      }

      ATH_MSG_DEBUG("There is no model available for this cluster.");
      return -999;
    }



    //==============================================================================
    // PreprocessingMode = 0: use only rings normalized by total energy as input
    // PreprocessingMode = 1: use only a half of total rings and then normalize to use as input
    std::vector<std::vector<float>> RingerSelector::prepare_inputs(  unsigned PreprocessingMode,
                                                                     const xAOD::TrigRingerRings *ringsCluster, 
                                                                     const xAOD::TrigElectron */*el*/ ) const
    {
      std::vector< std::vector< float > > inputs;

      // Barcode 0 for all rings normalized by the total energy
      if ( PreprocessingMode == 0 ){ 
        const std::vector<float> rings = ringsCluster->rings();
        std::vector<float> refRings(rings.size());
        refRings.assign(rings.begin(), rings.end());
        float energy=0.0;
        for(auto &ring : refRings ) energy+=ring;

        if (energy > 0){
          for(auto &ring : refRings ) ring/=energy;
        }
        
        inputs.push_back( refRings );

      // Barcode 1 for half rings normalized by the total energy
      }else if ( PreprocessingMode == 1){

        const std::vector<float> rings = ringsCluster->rings();
        std::vector<float> refRings(rings.size());
        refRings.assign(rings.begin(), rings.end());

        std::vector<float> halfRings; 
        
        constexpr int PS   = 8;
        constexpr int EM1  = 64;
        constexpr int EM2  = 8;
        constexpr int EM3  = 8;
        constexpr int HAD1 = 4;
        constexpr int HAD2 = 4;
        constexpr int HAD3 = 4;

        halfRings.insert(halfRings.end(), refRings.begin(), refRings.begin() + PS/2);
        halfRings.insert(halfRings.end(), refRings.begin() + PS, refRings.begin() + PS + (EM1/2));
        halfRings.insert(halfRings.end(), refRings.begin() + PS + EM1, refRings.begin() + PS + EM1 + (EM2/2));
        halfRings.insert(halfRings.end(), refRings.begin() + PS + EM1 + EM2, refRings.begin() + PS + EM1 + EM2 + (EM3/2));
        halfRings.insert(halfRings.end(), refRings.begin() + PS + EM1 + EM2 + EM3, refRings.begin() + PS + EM1 + EM2 + EM3 + (HAD1/2));
        halfRings.insert(halfRings.end(), refRings.begin() + PS + EM1 + EM2 + EM3 + HAD1, refRings.begin() + PS + EM1 + EM2 + EM3 + HAD1 + (HAD2/2));
        halfRings.insert(halfRings.end(), refRings.begin() + PS + EM1 + EM2 + EM3 + HAD1 + HAD2, refRings.begin() + PS + EM1 + EM2 + EM3 + HAD1 + HAD2 + (HAD3/2));

        // concatenate
        float energy=0.0;
        for(auto &ring : halfRings ) energy+=ring;

        if (energy > 0){
          for(auto &ring : halfRings ) ring/=energy;
        }

        inputs.push_back( halfRings );
      }

      return inputs;
    }






    //==============================================================================
    template <typename T> bool RingerSelector::strtof(const std::string& input, T& f)
    {  
      int diff = 0 ;
      std::string tmp = input;
      std::string::size_type first(0);
      std::string::size_type last(0);
      first = ( input.find('#') ) ;
 
      //if we do not find a comment character "#" we are fine
      if (first == std::string::npos) {
        std::istringstream buffer (tmp);
        buffer>>f;
        return true;
      } 
      
      //if we have found comment character check if it is inlined between two "#"
      last = (input.find('#',first+1) );
      //if nor error
      if (last == std::string::npos) {
        return false;
      }
      //else if between two "#" remove this part 
      diff = last - first ;
      tmp= tmp.erase(first,diff+1);
      std::istringstream buffer (tmp);
      buffer>>f;
      return true;
    }


    //==============================================================================
    template <typename T>  std::vector<T> RingerSelector::GetValues (const std::string& input,  TEnv& env)
    {    
      std::vector<T> CutVector;    
      std::string env_input(env.GetValue(input.c_str(), ""));
      if (!env_input.empty()) {
        std::string::size_type end;
        do {
          end = env_input.find(';');
          T  myValue(0);
          if(strtof(env_input.substr(0,end),myValue)){
            CutVector.push_back(myValue);
          }
          if (end != std::string::npos) {
            env_input= env_input.substr(end+1);
          }
        } while (end != std::string::npos);     
      }
      return CutVector;
    }


    //==============================================================================
    std::vector<std::string> RingerSelector::GetPaths(const std::string& input, TEnv& env)
    {
      std::vector<std::string> CutVector;    
      std::string env_input(env.GetValue(input.c_str(), ""));
      if (!env_input.empty()) {
        std::string::size_type end;
        do {
          end = env_input.find(';');
          CutVector.push_back( env_input.substr(0,end) );
          if (end != std::string::npos) {
            env_input= env_input.substr(end+2);
          }
        } while (end != std::string::npos);     
      }
      return CutVector;
    }

    std::string RingerSelector::GetBasePath(const std::string &path) const {
      // Get the normalized parent path
      std::string ret = std::filesystem::path(path).parent_path().lexically_normal().string();
      if (!ret.empty() && ret.front()=='/') ret.erase(0, 1);
      return ret;
    }


  } // namespace onnx
} // namespace Ringer


