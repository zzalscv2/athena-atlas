/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EventInfoPixelDecorator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EVENTINFOPIXELDECORATOR_H
#define DERIVATIONFRAMEWORK_EVENTINFOPIXELDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "ExpressionEvaluation/ExpressionParser.h"

namespace DerivationFramework {

  class EventInfoPixelDecorator : public AthAlgTool, public IAugmentationTool {
    public: 
      EventInfoPixelDecorator(const std::string& type, const std::string& name, const IInterface* parent);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      std::unique_ptr<ExpressionParsing::ExpressionParser> m_parser;
      Gaudi::Property<std::string> m_selectionString
         { this, "SelectionString", "" , "" };

      mutable std::atomic<unsigned int> m_ntot {};
      mutable std::atomic<unsigned int> m_npass {};

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
         { this, "EventInfoKey", "EventInfo", ""};
      Gaudi::Property<std::string> m_decorationPrefix
         { this, "DecorationPrefix", "", ""};
     SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_pixelKey
         { this, "TrackMeasurementValidationKey", "PixelClusters", ""};

  }; 
}

#endif // DERIVATIONFRAMEWORK_EVENTINFOPIXELDECORATOR_H
