/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArBadChanBitPackingBase_H
#define LArBadChanBitPackingBase_H

#include "LArRecConditions/LArBadChannel.h"
#include <string>
#include <vector>
#include <map>

template <class T>
class TLArBadChanBitPackingBase {
 public:

  typedef std::vector<LArBadChannel::PosType>        PosContainer;

  TLArBadChanBitPackingBase(): m_bitPos( 8*sizeof(LArBadChannel::BitWord),0),
  m_index( 8*sizeof(LArBadChannel::BitWord),-1),
  m_highGainMask(0),
  m_mediumGainMask(0),
  m_lowGainMask(0) {};	

  const std::string& stringName( typename T::ProblemType pb) const;

  std::pair<bool, typename T::ProblemType> enumName( const std::string& str) const;

  LArBadChannel::PosType bitPosition( typename T::ProblemType pb) const {return m_bitPos[static_cast<LArBadChannel::PosType>(pb)];}

  LArBadChannel::PosType wordSize() const {return sizeof(LArBadChannel::BitWord);}

  void setBit( typename T::ProblemType pb, LArBadChannel::BitWord& word, bool value=true) const;
  void setBit( typename T::ProblemType pb, LArBadChannel& word, bool value=true) const;

  bool setBit( const std::string& name, LArBadChannel::BitWord& word, bool value=true) const;
  bool setBit( const std::string& name, LArBadChannel& word, bool value=true) const;

  std::string stringStatus( const LArBadChannel& bc) const;

  // These LArBadChannel::BitWord masks are used to do gain-dependent masking in LArBadChannelMasker.
  LArBadChannel::BitWord highGainMask() const   {return m_highGainMask;}
  LArBadChannel::BitWord mediumGainMask() const {return m_mediumGainMask;}
  LArBadChannel::BitWord lowGainMask() const    {return m_lowGainMask;}

  int numberOfProblemTypes() const { return m_nameVec.size();}

 protected:

  // This is used to describe the dependence of each T::ProblemType on LAr gain.
  enum GainDependence { independent, low, medium, high};

  PosContainer               m_bitPos;
  std::vector<int>           m_index;
  std::vector< std::pair<typename T::ProblemType, GainDependence> >   m_enumVec;  

  std::vector<std::string>   m_nameVec;
  std::map<std::string, typename T::ProblemType>  m_nameMap;

  // These are non-static so that different TLArBadChanBitPackingBase versions can be used simultaneously.
  LArBadChannel::BitWord m_highGainMask;
  LArBadChannel::BitWord m_mediumGainMask;
  LArBadChannel::BitWord m_lowGainMask;

  void addBit( typename T::ProblemType pb, const std::string& name, GainDependence gaindep);
  int index( typename T::ProblemType pb) const {return m_index[pb];}
  void initMasks();
};

#include "LArBadChanBitPackingBase.icc"


#endif

