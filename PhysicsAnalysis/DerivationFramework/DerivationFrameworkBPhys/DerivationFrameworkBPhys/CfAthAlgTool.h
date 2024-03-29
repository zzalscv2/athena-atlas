/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//============================================================================
// CfAthAlgTool.h
//============================================================================
// 
// Author : Wolfgang Walkowiak <Wolfgang.Walkowiak@cern.ch.>
// Changes:
//
// Wrapper around AthAlgTool to provide easy access to CutFlowSvc
// and some utility methods for it.
// Methods for accessing the CutFlowSvc are modelled after
// AthFilterAlgorithm's implementation.
//
// This class inherits from AthAlgTool.  It should be inherited from.
//
//============================================================================
//
#ifndef DERIVATIONFRAMEWORK_CfAthAlgTool_H
#define DERIVATIONFRAMEWORK_CfAthAlgTool_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/ICutFlowSvc.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

#include <string>
#include <map>

namespace DerivationFramework {
  
  class ATLAS_NOT_THREAD_SAFE CfAthAlgTool : public AthAlgTool {
  public:
    // constructor with parameters
    CfAthAlgTool(const std::string& t, const std::string& n,
		 const IInterface* p);
    // destructor
    virtual ~CfAthAlgTool();
    
    // Initialization method invoked by the framework.
    virtual StatusCode sysInitialize() override;

    // add event to a named counter -- returns counts after adding
    virtual bool addEvent(const std::string &name, double weight=1.) const;

    // add to a named counter -- returns counts after adding
    // if counts > 1 : same weight is added multiple times
    virtual bool addToCounter(const std::string &name, uint64_t counts=1,
			      double weight=1.) const;

  protected:
    // add a counter by name -- returns id if it already exists
    CutIdentifier getCounter(const std::string &name) const;

    // returns counter name by id
    std::string getCounterNameById(CutIdentifier id) const;

    // returns counter id by name
    CutIdentifier getCounterIdByName(const std::string &name) const;
    
  private:
    // handle to the service holding tables of cut-flows for filtering algs.
    ServiceHandle<ICutFlowSvc> m_cutFlowSvc;

    // base name for counters
    std::string m_ctbasename;

    // map of counter names to counter ids
    typedef std::map<std::string, CutIdentifier> NameIdMap_t;
    mutable NameIdMap_t m_mctn;

    // base counter
    mutable CutIdentifier m_bid;
    mutable bool          m_bidisset;
    
  }; // class
} // namespace

#endif // DERIVATIONFRAMEWORK_CfAthAlgTool_H
