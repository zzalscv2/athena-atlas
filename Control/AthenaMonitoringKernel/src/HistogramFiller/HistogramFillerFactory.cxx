/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/starts_with.h"

#include "StaticHistogramProvider.h"
#include "LumiblockHistogramProvider.h"
#include "OfflineHistogramProvider.h"
#include "LiveHistogramProvider.h"

#include "HistogramFiller1D.h"
#include "HistogramFillerEfficiency.h"
#include "CumulativeHistogramFiller1D.h"
#include "HistogramFillerRebinable.h"
#include "VecHistogramFiller1D.h"
#include "HistogramFillerProfile.h"
#include "HistogramFiller2D.h"
#include "HistogramFiller2DProfile.h"
#include "HistogramFillerTree.h"

#include "HistogramFillerFactory.h"

using namespace Monitored;

HistogramFiller* HistogramFillerFactory::create(const HistogramDef& def) {
  std::shared_ptr<IHistogramProvider> histogramProvider = createHistogramProvider(def);
  
  if (CxxUtils::starts_with(def.type, "TH1")) {
    if (def.kCumulative) {
      return new CumulativeHistogramFiller1D(def, histogramProvider);
    } else if (def.kAddBinsDynamically || def.kRebinAxes) {
      return new HistogramFillerRebinable1D(def, histogramProvider);
    } else if (def.kVec || def.kVecUO) {
      return new VecHistogramFiller1D(def, histogramProvider);
    } else {
      return new HistogramFiller1D(def, histogramProvider);
    }
  } else if (CxxUtils::starts_with(def.type, "TH2")) {
    if (def.kAddBinsDynamically || def.kRebinAxes) {
      return new HistogramFillerRebinable2D(def, histogramProvider);
    } else {
      return new HistogramFiller2D(def, histogramProvider);
    }
  } else if (def.type == "TProfile") {
    if (def.kAddBinsDynamically || def.kRebinAxes) {
      return new HistogramFillerProfileRebinable(def, histogramProvider);
    } else {
      return new HistogramFillerProfile(def, histogramProvider);
    }
  } else if (def.type == "TProfile2D") {
    return new HistogramFiller2DProfile(def, histogramProvider);
  } else if (def.type == "TEfficiency") {
    return new HistogramFillerEfficiency(def, histogramProvider);
  } else if (def.type == "TTree") {
    return new HistogramFillerTree(def, histogramProvider);
  }
  
  return nullptr;
}

std::shared_ptr<IHistogramProvider> HistogramFillerFactory::createHistogramProvider(const HistogramDef& def) {
  std::shared_ptr<IHistogramProvider> result;

  if (def.runmode == HistogramDef::RunMode::Offline) {
    result.reset(new OfflineHistogramProvider(m_gmTool, m_factory, def));
  } else if (def.kLBNHistoryDepth) {
    result.reset(new LumiblockHistogramProvider(m_gmTool, m_factory, def));
  } else if (def.kLive) {
    result.reset(new LiveHistogramProvider(m_gmTool, m_factory, def));
  } else {
    result.reset(new StaticHistogramProvider(m_factory, def));
  }

  return result;
}
