/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSim_READOUTPUTHEADERTOOL_H
#define FPGATrackSim_READOUTPUTHEADERTOOL_H

/**
 * @file FPGATrackSimOutputHeaderTool.h
 *
 * This class reads/write FPGATrackSim output data from/to a ROOT file 
 * Designed to be not thread-safe
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"
#include <numeric>
#include <atomic>

class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;

class FPGATrackSimOutputHeaderTool : public extends<AthAlgTool, IFPGATrackSimEventOutputHeaderTool>  
{

public:

  FPGATrackSimOutputHeaderTool(std::string const &, std::string const &, IInterface const *);
  virtual ~FPGATrackSimOutputHeaderTool()  = default;
  virtual StatusCode initialize() override; 
  virtual StatusCode finalize()   override;

  virtual StatusCode readData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader, bool &last) override;
  virtual StatusCode writeData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader)            override;
  
  std::string fileName() { return std::accumulate(m_inpath.value().begin(), m_inpath.value().end(), std::string{}); }

private:
  // JO configuration
  StringArrayProperty   m_inpath          {this, "InFileName", {"."}, "input file paths"};
  StringProperty        m_rwoption        {this, "RWstatus", std::string("READ"), "define read or write file option: READ, RECREATE, HEADER"};
  BooleanProperty       m_runSecondStage  {this, "RunSecondStage",false, "flag to enable running the second stage fitting"};

  // internal counters  
  std::atomic<unsigned> m_event = 0;
  std::atomic<unsigned> m_totevent = 0;
  std::atomic<unsigned> m_file = 0;
 
  std::string m_branchNameIn_1st;
  std::string m_branchNameIn_2nd;
  std::string m_branchNameOut;    
  
  StatusCode openFile(std::string const & path);

};

#endif // FPGATrackSim_READOUTPUTHEADERTOOL_H
