/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_BEAMSPOTRETRIEVER_H
#define JIVEXML_BEAMSPOTRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "BeamSpotConditionsData/BeamSpotData.h"


namespace JiveXML {
  
  /**
   * @class BeamSpotRetriever
   * @brief Retrieve the @c BeamSpot information (official ID s/w result)
   *
   *  - @b Retrieved @b Data
   *    - <em>id</em>: BeamSpot dump
   *    .
   */
  class BeamSpotRetriever : virtual public IDataRetriever,
                                    public AthAlgTool {
    
  public:
    
    virtual StatusCode initialize() override;
    
    /// Standard constructor
    BeamSpotRetriever(const std::string& type,const std::string& name,const IInterface* parent);

      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override; 

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "BeamSpot"; };

  private:
     
      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
  };
}

#endif
