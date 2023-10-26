// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCELLCONTOCOPYTOOL_H
#define CALOREC_CALOCELLCONTOCOPYTOOL_H

/**
 @class CaloCellContCopyTool
 @brief Concrete tool for coping an entire CaloCellContainer

  CaloCellContCopyTool to be used by CaloCellMaker algorithms.
  The tool copies Calo cells from an existing
  CaloCellContainer to the one processed by the CaloCellMaker algorithm.
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloCellMakerTool.h"

class CaloCellContCopyTool
  : public extends<AthAlgTool,ICaloCellMakerTool>
{
  public:
    using base_class::base_class;

    virtual StatusCode initialize() override;
    virtual StatusCode process (CaloCellContainer* theCellContainer,
                                const EventContext& ctx) const override;


private:
    SG::ReadHandleKey<CaloCellContainer> m_srcCellContainerKey{this,"InputName","AllCalo"};
};


#endif /* CALOREC_CALOCELLFASTCOPYTOOL_H_ */
