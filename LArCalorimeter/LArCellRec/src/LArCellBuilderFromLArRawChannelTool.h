//Dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LARCELLBUILDERFROMLARRAWCHANNELTOOL_H
#define LARCELLREC_LARCELLBUILDERFROMLARRAWCHANNELTOOL_H
/** 
@class LArCellBuilderFromLArRawChannelTool 
@brief Building LArCell objects from LArRawChannel 


 AlgTool properties (name defined in cxx file): 
     RawChannelsName :  input RawChannelContainer 
     EThreshold:          energy threshold

   Modified:  Dec 4, 2002   Hong Ma 
     Use MakeLArCellFromRaw to make LArCell. 
   Modified: June 2, 2004, David Rousseau : converted to AlgTool
*/

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CaloInterface/ICaloCellMakerTool.h"
#include "Identifier/HWIdentifier.h"
#include "LArIdentifier/LArOnlineID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "AthAllocators/DataPool.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRawEvent/LArRawChannelContainer.h"

class CaloCellContainer ;
class CaloCell_ID;
class CaloCellContainer ;
class LArCell;

/**
 * @class LArCellBuilderFromLArRawChannelTool
 * @brief An AlgTool class to create a CaloCellContainer out of a LArRawChannel container.
 *
 * Inherits from ICaloCellMakerTool and should be called by an instance of the 
 * CaloCellMaker algorithm. 
 * 
 */


class LArCellBuilderFromLArRawChannelTool
  : public extends<AthAlgTool, ICaloCellMakerTool>
{
public:  
  /**
   * @brief Standard AlgTool constructor
   */
  using base_class::base_class;
  /**
   * @brief Destructor, deletes the MsgService.
   */
  virtual ~LArCellBuilderFromLArRawChannelTool() =default;

  /**
   * @brief Initialize method.
   * @return Gaudi status code.
   *
   * Initialazes pointers to servies and private members variable.
   * Sets the m_subCalo variable based on the m_lArRegion jobOption.
   * Computes the total number of cells based on the subcalo hash-range. 
   *
   */
  virtual StatusCode initialize() override;

  /**
   * @brief process method as defined in ICaloCellMaker interface
   * @param theCellContainer Pointer to the CaloCellContainer we are working on
   * @param ctx The event context.
   * @return Gaudi status code.
   *
   */
  virtual StatusCode process (CaloCellContainer* theCellContainer,
                              const EventContext& ctx) const override;

private: 

  SG::ReadHandleKey<LArRawChannelContainer>   m_rawChannelsKey{this,"RawChannelsName","LArRawChannels","Name of input container"}; //!< rdo container name (jO)
  Gaudi::Property<bool>            m_addDeadOTX{this,"addDeadOTX",true,"Add dummy cells for missing FEBs"};                        //!< activate addition of missing cells from dead OTX
  Gaudi::Property<int>             m_initialDataPoolSize{this,"InitialCellPoolSize",-1,"Initial size of the DataPool<LArCells> (-1: Use nCells)"};  //!< Initial size of DataPool<LArCell>

  //Internally used variables
  unsigned m_nTotalCells=0;                   //!< Number of cells, set in Initialize()


  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"LArCablingKey","LArOnOffIdMap","Key of  conditions data object holding cabling"};
  const LArOnlineID* m_onlineID=nullptr;
  const CaloCell_ID*  m_caloCID=nullptr;
  SG::ReadCondHandleKey<LArBadFebCont> m_missingFebKey{this,"MissingFebKey","LArBadFeb","Key of conditions data object holding bad-feb info"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};      


};


#endif





