/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimREADRAWRANDOMHITSTOOL_H
#define FPGATrackSimREADRAWRANDOMHITSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"

#include "TFile.h"
#include "TTree.h"



class FPGATrackSimReadRawRandomHitsTool : public extends<AthAlgTool, IFPGATrackSimEventInputHeaderTool>
{
  public:

  FPGATrackSimReadRawRandomHitsTool(const std::string&, const std::string&, const IInterface*);
  virtual ~FPGATrackSimReadRawRandomHitsTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last) override;
  virtual StatusCode writeData(FPGATrackSimEventInputHeader* header) override; 
  virtual StatusCode finalize() override;
  
  StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last, bool doReset);

  private:
  // JO configuration    
  StringProperty m_inpath {this, "InFileName", "httsim_smartwrapper.root", "input path"};

  // Internal pointers       
  unsigned m_entry = 0;
};

#endif // FPGATrackSimREADRAWRANDOMHINPUTTOOL_H
