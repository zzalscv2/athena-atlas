/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCHighPtBoard_hh
#define TGCHighPtBoard_hh

#include "TrigT1TGC/TGCSlaveBoard.h"
#include "TrigT1TGC/TGCConnectionHPBToSL.h"

namespace LVL1TGCTrigger {
class TGCSlaveBoardOut;
class TGCHighPtBoardOut;
class TGCHighPtChipOut;

const int PtLow = 0;
const int PtHigh =1;

const int NumberOfAdjacentHPB = 2;
const int NumberOfChip = 2;
const int NBlockOfDSBChannel = 6;
const int NDecoderInTSB = 4; 
const int NDecoderInDSB = 3; 
const int NDecoderInSTSB = 3; 

enum TGCHBChip { ChipA, ChipB, TotalNumTGCHBChip, HBChipUnit = 1 };

inline TGCHBChip operator++( TGCHBChip &rs, int )
{
    return rs = (TGCHBChip)(rs + 1);
}

class TGCHighPtBoard
{
 private:
  static constexpr int s_NumberOfTSBOut = 2;
  static constexpr int s_NumberOfDSBOut = 3; //look at the HighPt Board

 public:
  virtual ~TGCHighPtBoard();

  void clockIn(int bidIn);
  TGCHighPtChipOut* getOutput();
  void eraseOutput();

  int getId() const;
  void setId(int idIn);
  int getType() const;
  void setType(int typeIn){ m_type=typeIn;}
  TGCRegionType getRegion() const { return m_region;}
  void setRegion(const TGCRegionType regionIn){ m_region = regionIn;}

  void setDSB(int connector, TGCSlaveBoard* SBIn);
  void setTSB(int connector, TGCSlaveBoard* SBIn);
  TGCSlaveBoardOut* getTSBOut(int chip, int port);
  void setAdjacentHPB(int side, TGCHighPtBoard* hpt){ m_adjacentHPB[side]=hpt;};

  void showResult() const;

 protected:
  TGCHighPtBoard();  // use derived class only
  TGCHighPtBoard(const TGCHighPtBoard& right) = delete;
  TGCHighPtBoard& operator=(const TGCHighPtBoard& right) = delete;

 protected:
  void collectInput();
  void createHighPtChipOut();
  void createHighPtBoardOut();
  void createLowPtBoardOut();		//create Low-pT Outputs from SLB ASICs.
  void doCoincidence();
  void trackSelector(int chip, int ptIn);
  virtual void highLowSelector(int chip);
  void deleteSlaveBoardOut();
  void loadLowPtOutput(int chip, int block, TGCSlaveBoardOut* dataDSB);
  void eraseHighPtOutput();
  void eraseLowPtOutput();

  virtual void decodeSlaveBoardOut(int chip, 
				   TGCHitPattern* tHit, 
				   TGCHitPattern* dHit) = 0;
  virtual void doCoincidenceMatrix( int chip, int block,
				    const TGCHitPattern* tHit, 
				    const TGCHitPattern* dHit) = 0;
  virtual void setDecoderIn()=0;
  virtual void clearDecoderIn()=0;

  TGCHighPtChipOut* m_highPtChipOut;

  TGCHighPtBoardOut* m_highPtBoardOut;
  TGCHighPtBoardOut* m_lowPtBoardOut;	//Low-pT Outputs from SLB ASICs.

  TGCSlaveBoard* m_DSB[NumberOfChip][s_NumberOfDSBOut];
  TGCSlaveBoard* m_TSB[NumberOfChip][s_NumberOfTSBOut];
  TGCSlaveBoardOut* m_DSBOut[NumberOfChip][s_NumberOfDSBOut];
  TGCSlaveBoardOut* m_TSBOut[NumberOfChip][s_NumberOfTSBOut];
  TGCHighPtBoard* m_adjacentHPB[NumberOfAdjacentHPB];
  TGCSlaveBoardOut* m_decoderInTSB[NumberOfChip][NDecoderInTSB];
  TGCSlaveBoardOut* m_decoderInDSB[NumberOfChip][NDecoderInDSB];

  int m_id;
  int m_bid;
  int m_idSectorLogic;
  int m_type;
  TGCRegionType m_region{TGCRegionType::FORWARD};

  int m_priorSign;
  int m_maxNumberOfHPBData;
  int m_maxDev;
  int m_maxDevOred;
  int m_nChOfTSBOut;
  int m_nChOfDSBOut;
  int m_nChOfDSBHit;
  int m_nChOfTSBHit;
  int m_nChInTSBRegion;
};

inline
void TGCHighPtBoard::setId(int idIn)
{
  m_id = idIn;
}

inline
int TGCHighPtBoard::getId() const
{
  return m_id;
}

inline
int TGCHighPtBoard::getType() const
{
  return m_type;
}

inline
void TGCHighPtBoard::setDSB(int connector, TGCSlaveBoard* SBIn)
{
  int port = connector % s_NumberOfDSBOut;
  int chip = connector / s_NumberOfDSBOut;
  m_DSB[chip][port] = SBIn;
}

inline
void TGCHighPtBoard::setTSB(int connector, TGCSlaveBoard* SBIn)
{
  int port = connector % s_NumberOfTSBOut;
  int chip = connector / s_NumberOfTSBOut;
  m_TSB[chip][port] = SBIn;
}


} //end of namespace bracket

#endif // TGCHighPtBoard_hh
