/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

// $Id: KLFitterRun.h 688037 2015-08-07 01:24:06Z morrisj $
#ifndef ANALYSISTOP_TOPEVENTRECONSTRCUTIONTOOLS_KLFITTERRUN_H
#define ANALYSISTOP_TOPEVENTRECONSTRCUTIONTOOLS_KLFITTERRUN_H

#include "TopEventSelectionTools/EventSelectorBase.h"
#include "TopEventReconstructionTools/KLFitterTool.h"

#include <utility>
#include <string>
#include <memory>


namespace top {
  class Event;
  class TopConfig;

  class KLFitterRun: public EventSelectorBase {
  public:
    KLFitterRun(const std::string& kSelectionName, const std::string& kParameters,
                std::shared_ptr<top::TopConfig> config);
    virtual ~KLFitterRun() {}

    virtual bool apply(const top::Event&) const override;
    std::string name() const override;

  private:

    std::pair<bool,bool> hasAutoSetOption(const std::string &curtom_parameters);

      std::string m_name;

      bool m_useJetAutoSet;
      bool m_useJetAutoSetPCBT;
      int m_Njcut;
      int m_nb;
      int m_delta;
      int m_Nbmin;
      int m_N5max;

      std::unique_ptr<top::KLFitterTool> m_myFitter;
  };
}
#endif
