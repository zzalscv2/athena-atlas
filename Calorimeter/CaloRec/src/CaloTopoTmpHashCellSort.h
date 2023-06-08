/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOTOPOTMPHASHCELLSORT_H
#define CALOTOPOTMPHASHCELLSORT_H
//-----------------------------------------------------------------------
// File and Version Information:
// $Id: CaloTopoTmpHashCellSort.h,v 1.2 2008-08-28 05:15:05 ssnyder Exp $
//
// Description: ration (could be S/N, or E/V) sorting for CaloTopoTmpHashCells
//   
// Environment:
//      Software developed for the ATLAS Detector at the CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

#include "CaloTopoTmpHashCell.h"

namespace CaloTopoTmpHashCellSort{
  
  // comparison,  order seed cells by E/V
  template <class T>
  class compare
  {

  public:
    inline   compare() {} ;
    inline   bool operator () (const CaloTopoTmpHashCell<T>& c1, 
			       const CaloTopoTmpHashCell<T>& c2)
    {
      return c1.getCaloTopoTmpClusterCell()->getSignedRatio() > c2.getCaloTopoTmpClusterCell()->getSignedRatio();
    }  
  
  };

  template <class T>
  class compareAbs
  {

  public:
    inline   compareAbs() {} ;
    inline   bool operator () (const CaloTopoTmpHashCell<T>& c1, 
			       const CaloTopoTmpHashCell<T>& c2)
    {
      return std::abs(c1.getCaloTopoTmpClusterCell()->getSignedRatio()) > std::abs(c2.getCaloTopoTmpClusterCell()->getSignedRatio());
    }  
  };

  //These are used to agree with GPU ordering: use the cell index as a tie-breaker.

  template <class T>
  class compareWithIndex
  {

  public:
    inline   compareWithIndex() {} ;
    inline   bool operator () (const CaloTopoTmpHashCell<T>& c1, 
			       const CaloTopoTmpHashCell<T>& c2)
    {
      const auto s1 = c1.getCaloTopoTmpClusterCell()->getSignedRatio();
      const auto s2 = c2.getCaloTopoTmpClusterCell()->getSignedRatio();
      if (s1 == s2) {
	return c1.getCaloTopoTmpClusterCell()->getID() > c2.getCaloTopoTmpClusterCell()->getID();
      }
      return s1 > s2;
    }  
  
  };

  template <class T>
  class compareAbsWithIndex
  {

  public:
    inline   compareAbsWithIndex() {} ;
    inline   bool operator () (const CaloTopoTmpHashCell<T>& c1, 
			       const CaloTopoTmpHashCell<T>& c2)
    {
      const auto s1 = std::abs(c1.getCaloTopoTmpClusterCell()->getSignedRatio());
      const auto s2 = std::abs(c2.getCaloTopoTmpClusterCell()->getSignedRatio());
      if (s1 == s2) {
	return c1.getCaloTopoTmpClusterCell()->getID() > c2.getCaloTopoTmpClusterCell()->getID();
      }
      return s1 > s2;
    }  
  };
}

#endif // CALOTOPOTMPHASHCELLSORT_H

