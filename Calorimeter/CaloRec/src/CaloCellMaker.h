/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCELLMAKER_H
#define CALOREC_CALOCELLMAKER_H

/********************************************************************

 NAME:     CaloCellMaker.h
 PACKAGE:  offline/Calorimeter/CaloRec

 AUTHORS:  David Rousseau
 CREATED:  May 11, 2004

 PURPOSE:  Create a CaloCellContainer by calling a set of tools
 sharing interface CaloInterface/ICaloCellMakerTool.h

 ********************************************************************/

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

// Calo includes
#include "CaloEvent/CaloCellContainer.h"
//#include "CaloInterface/ICaloCellMakerTool.h"

class IChronoStatSvc;
class ICaloCellMakerTool;

class CaloCellMaker: public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~CaloCellMaker() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:

    /// ChronoStatSvc
    ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};
    Gaudi::Property<bool> m_doChronoStat{this,"EnableChronoStat",true};

    //Decide if the container owns the cells or views it (default false=view)
    Gaudi::Property<bool> m_ownPolicyProp{this,"OwnPolicy",false};
    SG::OwnershipPolicy m_ownPolicy{SG::VIEW_ELEMENTS};

    /// Output cell continer to be used 
    SG::WriteHandleKey<CaloCellContainer> m_caloCellsOutputKey{this,"CaloCellsOutputName","AllCalo","SG Key of the output container"};

    /// Array of CellMaker (and corrector) AlgTools
    ToolHandleArray<ICaloCellMakerTool> m_caloCellMakerTools{this,"CaloCellMakerToolNames",{}};

};
#endif
