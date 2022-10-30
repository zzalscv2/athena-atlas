// emacs: this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGROICONVERSION_ROIWRITER_H
#define TRIGROICONVERSION_ROIWRITER_H

// System include(s):
#include <string>

// Athena include(s):
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// EDM include(s):
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

/// Algorithm creating xAOD::RoiDescriptorStore objects
///
/// This algorithm creates xAOD::RoiDescriptorStore objects during AOD writing.
///
/// @author M. Sutton
/// @author F. Winklmeier
///
class RoiWriter : public AthReentrantAlgorithm {

public:
  /// Algorithm constructor
  RoiWriter( const std::string& name, ISvcLocator* pSvcLocator );
  
  /// Execute the conversion
  virtual StatusCode execute(const EventContext& ctx) const override;
  
}; // class RoiWriter

#endif // TRIGROICONVERSION_ROIWRITER_H
