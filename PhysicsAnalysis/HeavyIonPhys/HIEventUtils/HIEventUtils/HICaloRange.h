/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_HICALORANGE_H
#define HIEVENTUTILS_HICALORANGE_H

#include "HIEventUtils/HIEventDefs.h"
#include <unordered_map>

class HICaloRange
{
public:
  //methods to access static member
  static const HICaloRange& getRange();

  //public class member functions
  bool LayerInRange(float eta, int layer);
  inline float getRangeMin(int layer) const {return m_range.find(layer)->second.first;};
  inline float getRangeMax(int layer) const {return m_range.find(layer)->second.second;};

private:
  //private constructor for singleton
  HICaloRange();
  void initializeRange();

  //members
  typedef std::pair<float,float> range_t;
  std::unordered_map<int, range_t > m_range;
};


#endif
