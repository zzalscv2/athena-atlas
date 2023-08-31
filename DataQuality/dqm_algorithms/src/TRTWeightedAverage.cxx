/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*! \file TRTWeightedAverage.cxx : Written for TRT Drift Time histograms. Algorithm calculates weighted mean of the selected binrange. This value compared against the expected value and the absolute difference returned. Also particular for this histogram; normalized value of bin 0 can be used for correction of the result. This leads to a tighter control for the Drift time histogram.  Method to store quantities in db; returns dqm_core::Result
 * \author Aytul Adiguzel & Emre Celebi
 */

#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_algorithms/TRTWeightedAverage.h"
#include "dqm_algorithms/tools/AlgorithmHelper.h"
#include "TH1.h"
#include "ers/ers.h"

#include <string>
#include <sstream>

static dqm_algorithms::TRTWeightedAverage myInstance;

dqm_algorithms::TRTWeightedAverage::TRTWeightedAverage()
{
    dqm_core::AlgorithmManager::instance().registerAlgorithm("TRTWeightedAverage", this);
}

dqm_algorithms::TRTWeightedAverage::~TRTWeightedAverage()
{
}

dqm_algorithms::TRTWeightedAverage *dqm_algorithms::TRTWeightedAverage::clone()
{
    return new TRTWeightedAverage();
}

dqm_core::Result *dqm_algorithms::TRTWeightedAverage::execute(const std::string &name, const TObject &object, const dqm_core::AlgorithmConfig &config)
{
    const TH1 * histogram;
    const TH1 * refhist;

    if (object.IsA()->InheritsFrom("TH1")) {
        histogram = static_cast<const TH1*>(&object);
        if (histogram->GetDimension() > 1) {
            throw dqm_core::BadConfig(ERS_HERE, name, "dimension > 1");
        }
    } else {
        throw dqm_core::BadConfig(ERS_HERE, name, "does not inherit from TH1");
    }

    const double minstat = dqm_algorithms::tools::GetFirstFromMap("MinStat", config.getParameters(), -1);
    const double firstbin = dqm_algorithms::tools::GetFirstFromMap("FirstBin", config.getParameters(), -1);
    const double lastbin = dqm_algorithms::tools::GetFirstFromMap("LastBin", config.getParameters(), -1);
    const double expectedaverage = dqm_algorithms::tools::GetFirstFromMap("ExpectedAverage", config.getParameters(), -1);
    const double p0 =  dqm_algorithms::tools::GetFirstFromMap("P0", config.getParameters(), -1);
    const double p1 =  dqm_algorithms::tools::GetFirstFromMap("P1", config.getParameters(), -1);
    const double correctusingbin1 = dqm_algorithms::tools::GetFirstFromMap("CorrectUsingBin1", config.getParameters(), -1);
    
    if (histogram->GetEntries() < minstat) {
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["InsufficientEntries"] = histogram->GetEntries();
        return result;
    }

    if (firstbin>lastbin){
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["Range_is_not_correct "] = 1;
        return result;
    }

    if (firstbin<0){
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["Range_is_not_correct "] = 2;
        return result;
    }
    
    if (lastbin > histogram->GetNbinsX()){
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["Range_is_not_correct "] = 3;
        return result;
    }
    
    try {
         refhist = dynamic_cast<const TH1*>( config.getReference() );
    }
    catch ( dqm_core::Exception & ex ) {
         throw dqm_core::BadRefHist(ERS_HERE,name," Could not retreive reference");
    }

    if (!refhist) {
         throw dqm_core::BadRefHist(ERS_HERE, name, "Reference is not a histogram");
    }
  
    if (histogram->GetDimension() != refhist->GetDimension() ) {
         throw dqm_core::BadRefHist( ERS_HERE, name, "Dimension" );
    }
  
    if ((histogram->GetNbinsX() != refhist->GetNbinsX()) || (histogram->GetNbinsY() != refhist->GetNbinsY())) {
         throw dqm_core::BadRefHist( ERS_HERE, name, "Non-matching number of bins of reference" );
    }
  
   std::map<std::string, double> results; // you can set flagging thresholds on any of these result tags
       
   double mean = 0;
   double sum = 0;
   double sumcontent = 0;
   double CorrectedMean= 0;

    for (int i = firstbin; i <= lastbin; ++i) {
        
        const double binContent = histogram->GetBinContent(i);
        const double binCenter = histogram->GetXaxis()->GetBinCenter(i);
        
        sum += binContent*binCenter;
        sumcontent += binContent;
    }


    if (sumcontent==0){
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["Content_is_zero "] = 1;
        return result;
    }

    mean = sum/sumcontent;

    if (correctusingbin1>0){
         double bin0=histogram->GetBinContent(1);
         double Nentries=histogram->GetEntries();
         double correctionfactor=bin0/Nentries;
         CorrectedMean= mean-(p0+p1*correctionfactor);
    }

    results["Weighted_mean"] = mean; 
    results["Abs_Diff"] = std::fabs(mean-expectedaverage);
    if (correctusingbin1>0){
         results["Corrected_Weighted_mean"]= CorrectedMean+expectedaverage;
         results["Corrected_Abs_Diff"]=std::fabs(CorrectedMean);
    }else{
         results["Corrected_Weighted_mean"]=0;
         results["Corrected_Abs_Diff"]=0;
    }
    //return result;
    return tools::MakeComparisons(results, config.getGreenThresholds(), config.getRedThresholds());
}
void dqm_algorithms::TRTWeightedAverage::printDescription(std::ostream& out)
{
   //Modify following part:
   out << "TRTWeightedAverage: Calculates weighted average of the bins within the selected bin range. It is also possible to correct these results using the normalized value of the 0th bin. This is helpful for TRT drift time histogram. In the case of the correction algorithm expects the coefficients p0 and p1 such that (weighted_mean-p0-p1*normalized_firtbin_value) is 0 on the average. \n"
     "Optional parameter: MinStat :\n"
     "Optional parameter: FirstBin :\n"
     "Optional parameter: LastBin :\n"
     "Optional parameter: ExpectedAverage :\n"
     "Optional parameter: P0 :\n"
     "Optional parameter: P1 :\n"
     "Optional parameter: CorrectUsing0Bin :\n"
     "Returned values: \n"
     "  Weighted_mean: weighted average result for the selected bins. \n"     
     "  Abs_Diff: Absolute value of the difference between Weighted_mean and ExpectedAverage, fabs(Weighted_mean-ExpectedAverage) \n"
     "  Corrected_Weighted_mean:  Weighted_mean-(p0+p1*normalized_bin_0)+ExpectedAverage , 0 if CorrectUsing0Bin==0 \n"
     "  Corrected_Abs_Diff:  fabs(Weighted_mean-(p0+p1*normalized_bin_0)), 0 if CorrectUsing0Bin==0 \n"     
       << std::endl;
}
