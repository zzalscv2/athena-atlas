/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*! \file TRTCheckPeakSimple.cxx checks on the most probable value (peak) in the histogram and returns dqm_core::Result
 * \author Peter Cwetanski
 */

#include "dqm_algorithms/TRTCheckPeakSimple.h"
#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/Result.h"
#include "dqm_algorithms/tools/AlgorithmHelper.h"
#include "ers/ers.h"

#include "TClass.h"
#include "TH1.h"
#include "TF1.h"

#include <cmath>
#include <iostream>
#include <map>

static dqm_algorithms::TRTCheckPeakSimple staticInstance;

namespace dqm_algorithms
{

TRTCheckPeakSimple::TRTCheckPeakSimple(): m_name("TRTCheckPeakSimple")
{
    dqm_core::AlgorithmManager::instance().registerAlgorithm(m_name, this);
}

TRTCheckPeakSimple::~TRTCheckPeakSimple()
{
}

dqm_core::Algorithm *TRTCheckPeakSimple::clone()
{
    return new TRTCheckPeakSimple(*this);
}


dqm_core::Result *TRTCheckPeakSimple::execute(const std::string &name, const TObject &object, const dqm_core::AlgorithmConfig &config)
{
    const TH1 *histogram;

    if (object.IsA()->InheritsFrom("TH1")) {
        histogram = static_cast<const TH1*>(&object);
        if (histogram->GetDimension() > 2) {
            throw dqm_core::BadConfig(ERS_HERE, name, "dimension > 2 ");
        }
    } else {
        throw dqm_core::BadConfig(ERS_HERE, name, "does not inherit from TH1");
    }
    const double minstat = dqm_algorithms::tools::GetFirstFromMap("MinStat", config.getParameters(), -1);

    if (histogram->GetEntries() < minstat) {
        // ERS_INFO("Histogram does not satisfy MinStat requirement " << histogram->GetName());
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["InsufficientEntries"] = histogram->GetEntries();
        return result;
    }

    dqm_core::Result *result = new dqm_core::Result();

    if (histogram->Integral() == 0 && histogram->GetEntries() > 0) {
        ERS_DEBUG(1, "Histogram " << histogram->GetName() << " is filled with zeroes!");
        result->status_ = dqm_core::Result::Red;
        result->tags_["Integral"] = histogram->Integral();
        return result;
    }


    const double gthreshold = dqm_algorithms::tools::GetFromMap("PeakPosition", config.getGreenThresholds());
    const double rthreshold = dqm_algorithms::tools::GetFromMap("PeakPosition", config.getRedThresholds());
    const std::vector<int> range = dqm_algorithms::tools::GetBinRange(histogram, config.getParameters());
    const int xmin = range[0];
    const int xmax = range[1];

    //compute the weighted mean
    float wmean = 0;
    float sum = 0;
    float integral = 0;
    for (int i = xmin; i <= xmax; i++) {
        integral += histogram->GetBinContent(i);
        sum += i * (histogram->GetBinContent(i));
    }
    wmean = sum / integral; // make sure that integer division does not truncate decimal of mean

    //find the most probable value (peak)
    float maxbinsum = 0;
    float binsum = 0;
    float peakbin = 0;
    float peakpos = 0;

    for (int i = xmin + 1; i <= xmax - 1; i++) {
        binsum = histogram->GetBinContent(i - 1) + histogram->GetBinContent(i) + histogram->GetBinContent(i + 1);
        if (binsum > maxbinsum) {
            maxbinsum = binsum;
            if (maxbinsum != 0) {
                peakbin = ((i - 1) * histogram->GetBinContent(i - 1) + i * histogram->GetBinContent(i) + (i + 1) * histogram->GetBinContent(i + 1)) /  maxbinsum;
                peakpos = (histogram->GetBinCenter(i - 1) * histogram->GetBinContent(i - 1) + histogram->GetBinCenter(i) * histogram->GetBinContent(i) + histogram->GetBinCenter(i + 1) * histogram->GetBinContent(i + 1)) / maxbinsum;
            } else {
                peakbin = 0;
                peakpos = 0;
            }
        }
    }

    result->tags_["Weighted_mean"] = wmean;
    result->tags_["PeakBin"] = peakbin;
    result->tags_["PeakPosition"] = peakpos;

    if      (peakpos >= gthreshold) result->status_ = dqm_core::Result::Green;
    else if (peakpos >  rthreshold) result->status_ = dqm_core::Result::Yellow;
    else                            result->status_ = dqm_core::Result::Red;
    return result;
}

void TRTCheckPeakSimple::printDescription(std::ostream& out)
{
    out << m_name << ": Checks on the most probable value (peak) in the histogram." << std::endl;
}

} // namespace dqm_algorithms
