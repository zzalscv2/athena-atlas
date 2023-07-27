/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCPatchPanel_hh
#define TGCPatchPanel_hh

#include "TrigT1TGC/TGCArguments.h"
#include "TrigT1TGC/TGCConnectionPPToSB.h"
#include "TrigT1TGC/TGCNumbering.h"
#include <fstream>
#include <string>

namespace LVL1TGCTrigger {

class TGCPatchPanelOut;
class TGCASDOut;
class TGCBIDOut;
class TGCConnectionInPP;
class TGCDatabaseManager;

typedef class TGCPatchPanel TGCWireTripletPP;
typedef class TGCPatchPanel TGCWireDoubletPP;
typedef class TGCPatchPanel TGCStripTripletPP;
typedef class TGCPatchPanel TGCStripDoubletPP;
typedef class TGCPatchPanel TGCWireInnerPP;
typedef class TGCPatchPanel TGCStripInnerPP;


const int NumberOfPatchPanelOut = 2;
const int NumberOfASDOut = 4;
const int NumberOfPPOutputConnector = 4;
const int NumberOfASDChannel = 16;
const int NChOfPPOutputConnector = 2*NumberOfASDChannel;
const int MaxNumberOfConnector = 10;
const int NumberOfBunchKeptInPP = 1;

class TGCPatchPanel {
public:
  void clockIn(int bunch, TGCDatabaseManager* db=0);

  TGCPatchPanelOut* getOutput(int SBId);
  void eraseOutput(int SBId);

  int getIdSlaveBoard(int port) const;
  void setIdSlaveBoard(int port,int id);

  // pointer to adjacent Patch Panel board.
  TGCPatchPanel* getAdjacentPP(int side) { return m_PPAdj[side]; };
  const TGCPatchPanel* getAdjacentPP(int side) const { return m_PPAdj[side]; };
  void setAdjacentPP(int side, TGCPatchPanel* PP);

  void setASDOut(int ch, int connector, const TGCASDOut* asdOut);

  TGCBIDOut* getBIDOut(int ch, int connector, int bunch);

  void dumpPPOut();      // print output of Patch Panel
  void dumpPPOut(int i); // bit pattern is separated by underscore

  void showProperty();

  int getId() const;
  void setId(int idIn);
  int getType() const { return m_type; };
  void setType(int typeIn) { m_type=typeIn; };
  TGCRegionType getRegion() const { return m_region; };
  void setRegion(TGCRegionType regionIn) { m_region=regionIn; };

  void connect();
  std::string getTypeName(int typeIn) const;

  TGCPatchPanel(TGCArguments*);
  ~TGCPatchPanel();
  TGCPatchPanel(const TGCPatchPanel& right) = delete;
  TGCPatchPanel& operator = (const TGCPatchPanel& right);

  const TGCArguments* tgcArgs() const { return m_tgcArgs; }

private:
  void showResult() const;
  void doBID();
  int doOrLogic();
  int createOutput();
  int getInputConnectorIndex(const int connectorId) const;
  void clearASDOut();
  void deleteBIDOut();

  int m_id;
  int m_idSlaveBoard[NumberOfPatchPanelOut];
  int m_type;
  TGCRegionType m_region;
  int m_bunchCounter;
  bool m_hasASDOut;
  bool m_hasBIDOut;
  int  m_nHit; // 18-Jan-01 Added by KH

  TGCPatchPanelOut* m_PPOut[NumberOfPatchPanelOut];
  const TGCASDOut* m_ASDOut[NChOfPPOutputConnector][MaxNumberOfConnector];
  TGCBIDOut* m_BIDOut[NChOfPPOutputConnector][MaxNumberOfConnector][NumberOfBunchKeptInPP];
  TGCPatchPanel* m_PPAdj[2]; // pointer to adjacent board.
  TGCConnectionInPP* m_connectionInPP;
  
  TGCArguments* m_tgcArgs;
  
};

} //end of namespace bracket

#endif // TGCPatchPanel_hh
