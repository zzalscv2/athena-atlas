/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_BEAMSPOTRETRIEVER_H
#define JIVEXML_BEAMSPOTRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h" 

class IBeamCondSvc;  

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
    
    /// Standard constructor
    BeamSpotRetriever(const std::string& type,const std::string& name,const IInterface* parent);

      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return typeName; };

  private:
     
      ///The data type that is generated by this retriever
      const std::string typeName;

      /// A service handle for the beamspot service
      ServiceHandle<IBeamCondSvc> m_beamSpotSvc; 
  };
}

#endif
