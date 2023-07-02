/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITCONTAINER_H
#define LARDIGITCONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/BaseInfo.h"
#include "LArRawEvent/LArDigit.h"

/**
   @class LArDigitContainer
   @brief Container class for LArDigit */

/* date of creation : 30/09/2000 */

/*  modification :    30/06/2001 */
/*  date of last modification :    09/19/2002   HMA 
      Move to DataVector 
 */

class LArDigitContainer : public DataVector<LArDigit> {

 public :
 
  /** @brief Constructor */
   LArDigitContainer() : DataVector<LArDigit>() { }

  /** @brief Alternative Construction with ownership policy*/
  LArDigitContainer(SG::OwnershipPolicy ownPolicy) : DataVector<LArDigit>(ownPolicy) { }
   

/**
  * destructor 
  */
  virtual ~LArDigitContainer() { }
private:    

} ;


CLASS_DEF(LArDigitContainer,2711,0)
SG_BASE(LArDigitContainer, DataVector<LArDigit> );

#endif
