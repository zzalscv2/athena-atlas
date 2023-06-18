/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFPGATrackSimEventOutputHeaderTool_H
#define IFPGATrackSimEventOutputHeaderTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TFile.h"
#include "TTree.h"

class FPGATrackSimLogicalEventOutputHeader;
class FPGATrackSimLogicalEventInputHeader;

class IFPGATrackSimEventOutputHeaderTool : virtual public ::IAlgTool 
{      
 public:

  DeclareInterfaceID( IFPGATrackSimEventOutputHeaderTool, 1, 0);
  virtual ~IFPGATrackSimEventOutputHeaderTool() = default;
   
  virtual StatusCode readData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader, bool &last) = 0;
  virtual StatusCode writeData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader) = 0;
  
  virtual TTree* getEventTree() {return m_EventTree;};
  virtual  FPGATrackSimLogicalEventInputHeader*  getLogicalEventInputHeader_1st()  {return m_eventInputHeader_1st; };
  virtual  FPGATrackSimLogicalEventInputHeader*  getLogicalEventInputHeader_2nd()  {return m_eventInputHeader_2nd; };
  virtual  FPGATrackSimLogicalEventOutputHeader* getLogicalEventOutputHeader()   {return m_eventOutputHeader;};
 
 protected:
  
  FPGATrackSimLogicalEventInputHeader  *m_eventInputHeader_1st;
  FPGATrackSimLogicalEventInputHeader  *m_eventInputHeader_2nd;
  FPGATrackSimLogicalEventOutputHeader *m_eventOutputHeader;

  

  TFile *m_infile = nullptr;
  TTree *m_EventTree = nullptr;

};


#endif // IFPGATrackSimEventOutputHeaderTool_H
