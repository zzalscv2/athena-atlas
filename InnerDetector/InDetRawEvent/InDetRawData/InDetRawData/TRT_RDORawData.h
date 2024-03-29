/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_RDORawData.h
//   Header file for class TRT_RDORawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement RawData for TRT
///////////////////////////////////////////////////////////////////
// Version 1.0 14/10/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_TRT_RDORAWDATA_H
#define INDETRAWDATA_TRT_RDORAWDATA_H

// Base classes
#include "InDetRawData/InDetRawData.h"



class TRT_RDORawData :   public InDetRawData{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  // offline compact id of the readout channel, 
  // the word
  TRT_RDORawData(const Identifier rdoId, const unsigned int word);
 // Destructor:
  virtual ~TRT_RDORawData() = default;

  ///////////////////////////////////////////////////////////////////
  // Virtual methods 
  ///////////////////////////////////////////////////////////////////

  // High level threshold:
  virtual bool highLevel() const=0;

    // Time over threshold in ns for valid digits; zero otherwise:
  virtual double timeOverThreshold() const=0;  

    // drift time in bin
  virtual int driftTimeBin() const=0;  

protected:
 TRT_RDORawData() = default;
 TRT_RDORawData(const TRT_RDORawData&) = default;
 TRT_RDORawData(TRT_RDORawData&&) = default;
 TRT_RDORawData& operator=(const TRT_RDORawData&) = default;
 TRT_RDORawData& operator=(TRT_RDORawData&&) = default;
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

#endif // INDETRAWDATA_TRT_RDORAWDATA_H
