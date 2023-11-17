// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Used in ATLFAST3


#ifndef CALOCELLCONTAINERFCSFINALIZERTOOL_H
#define CALOCELLCONTAINERFCSFINALIZERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloInterface/ICaloConstCellMakerTool.h"

class CaloCellContainerFCSFinalizerTool
  : public extends<AthAlgTool, ICaloCellMakerTool, ICaloConstCellMakerTool>
{
public:
  CaloCellContainerFCSFinalizerTool(const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent) ;


  // update theCellContainer
  virtual StatusCode process (CaloCellContainer* theCellContainer,
                              const EventContext& ctx) const override;
  virtual StatusCode process (CaloConstCellContainer* theCellContainer,
                              const EventContext& ctx) const  override;

private:
  template <class CONTAINER>
  StatusCode doProcess (CONTAINER* theCellContainer) const;
};

#endif
