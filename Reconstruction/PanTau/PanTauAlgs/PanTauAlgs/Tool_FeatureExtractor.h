/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAUALGS_TOOL_FEATUREEXTRACTOR_H
#define PANTAUALGS_TOOL_FEATUREEXTRACTOR_H

#include <string>
#include <map>
#include <vector>

//! ASG
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

// PanTau
#include "PanTauAlgs/HelperFunctions.h"
#include "PanTauAlgs/ITool_PanTauTools.h"
#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/TauConstituent.h"
#include "PanTauAlgs/TauFeature.h"

namespace PanTau {
    
/** @class Tool_FeatureExtractor
    Tool to extract jet features from tau seeds.
    @author Christian Limbach
    @author Sebastian Fleischmann
*/
  class Tool_FeatureExtractor : public asg::AsgTool, virtual public PanTau::ITool_PanTauTools {

    ASG_TOOL_CLASS1(Tool_FeatureExtractor, PanTau::ITool_PanTauTools)
    
    public:

        Tool_FeatureExtractor(const std::string &name);
        virtual StatusCode initialize() override;
        
        //get the features for an input seed
        virtual StatusCode execute(PanTau::PanTauSeed* inSeed) const override;
        
    protected:
        
        //handle to the helper function
        PanTau::HelperFunctions m_HelperFunctions;
        ToolHandle<PanTau::ITool_InformationStore> m_Tool_InformationStore;
	std::string m_Tool_InformationStoreName;
        	
        //Function to calculate basic features
        StatusCode calculateBasicFeatures(PanTau::PanTauSeed* inSeed) const;
        
        //Function to calculate features for one set of constituents
        StatusCode calculateFeatures(PanTau::PanTauSeed* inSeed,
                                     int tauConstituentType,
				     const std::map<std::string, double>& variants_SeedEt) const;
        
        //Function to add the 4 momenta of the tau constituents to the features
        StatusCode addConstituentMomenta(PanTau::PanTauSeed* inSeed) const;
        
        //Function to calculate features based on two sets of constituents
        StatusCode addCombinedFeatures(PanTau::PanTauSeed* inSeed,
				       const std::map<std::string, double>& variants_SeedEt) const;
        
        //Function to add impact parameter features
        StatusCode addImpactParameterFeatures(PanTau::PanTauSeed* inSeed) const;
        
        //Function to fill the variants_SeedEt member
        static void fillVariantsSeedEt(const std::vector<PanTau::TauConstituent*>& tauConstituents,
				std::map<std::string, double>& variants_SeedEt) ;
                
        //helper function to fill the variants_SeedEt map
        static void addFeatureWrtSeedEnergy(PanTau::TauFeature* targetMap,
				     const std::string& featName,
				     double numerator,
				     const std::map<std::string, double>& denominatorMap) ;
        
        int m_Config_UseEmptySeeds = 0;
        
        
        //! Helper members
        std::vector<double> m_Config_CellBased_BinEdges_Eta;
        std::vector<double> m_Config_CellBased_EtaBinned_Pi0MVACut_1prong;
        std::vector<double> m_Config_CellBased_EtaBinned_Pi0MVACut_3prong;
        
        //make these configured via python! (super trick ;))
        static const std::string varTypeName_Sum()          {return "Sum";}
        static const std::string varTypeName_Ratio()        {return "Ratio";}
        static const std::string varTypeName_EtInRing()     {return "EtInRing";}
        static const std::string varTypeName_Isolation()    {return "Isolation";}
        static const std::string varTypeName_Num()          {return "Num";}
        static const std::string varTypeName_Mean()         {return "Mean";}
        static const std::string varTypeName_StdDev()       {return "StdDev";}
        static const std::string varTypeName_HLV()          {return "HLV";}
        static const std::string varTypeName_Angle()        {return "Angle";}
        static const std::string varTypeName_DeltaR()       {return "DeltaR";}
        static const std::string varTypeName_JetMoment()    {return "JetMoment";}
        static const std::string varTypeName_Combined()     {return "Combined";}
        static const std::string varTypeName_JetShape()     {return "JetShape";}
        static const std::string varTypeName_ImpactParams() {return "ImpactParams";}
        static const std::string varTypeName_Basic()        {return "Basic";}
        static const std::string varTypeName_PID()          {return "PID";}
        static const std::string varTypeName_Shots()        {return "Shots";}
        
        std::string m_varTypeName_Sum;
        std::string m_varTypeName_Ratio;
        std::string m_varTypeName_EtInRing;
        std::string m_varTypeName_Isolation;
        std::string m_varTypeName_Num;
        std::string m_varTypeName_Mean;
        std::string m_varTypeName_StdDev;
        std::string m_varTypeName_HLV;
        std::string m_varTypeName_Angle;
        std::string m_varTypeName_DeltaR;
        std::string m_varTypeName_JetMoment;
        std::string m_varTypeName_Combined;
        std::string m_varTypeName_JetShape;
        std::string m_varTypeName_ImpactParams;
        std::string m_varTypeName_Basic;
        std::string m_varTypeName_PID;

	bool m_init=false;

  public:
	inline bool isInitialized() override {return m_init;}
};

} // end of namespace PanTau
#endif // PANTAUALGS_TOOL_FEATUREEXTRACTOR_H
